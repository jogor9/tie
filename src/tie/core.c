#include <assert.h>
#include <math.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "math.h"

typedef uint32_t HistoryID;

#define HISTORYID_MAX UINT32_MAX

typedef struct {
        uint32_t flagged_index;
} ObjectID;

typedef enum {
        POINT,
        LINE
} ObjectType;

typedef struct {
        mat2d matrix;
        vec2d position;
} Transform;

typedef struct {
        ObjectType type;
        Transform transform;
        ObjectID *parents;
        ObjectID *children;
} Object;

// history changes include:
// * modification of global state:
// * * selection stack
// * * jumps (cursor position)
// * * intersection graph
// * insertion of new objects
// * modification of existing objects
// * deletion of objects
//
// 1 insert point
// 2 goto 0.5,0 # raw values in gotos are absolute
// 3 lineto history.points.-1
// 4 select history.lines.-1
// 5 append bezier quadratic 0.25,0.25 0.25,-0.25
// 6 undo -2
// # 7 delete point # may be an error depending on user options
//                  # if not, deletes the line and the curve that depend on it
//                  # and then itself
//                  # the history will then look like the one below
// 7 delete line
// 8 delete curve
// 9 delete point
//
// 1 insert point # objectID = 1
// * A point is inserted in the current cursor position
//
// 2 goto 0.5,0
// * The cursor is updated to position 0.5,0.
//
// 3 lineto history.points.-1
// * This command splits into two subcommands:
// * *
// * * 3a insert point 0.5,0 # objectID = 2
// * * * The first subcommand inserts a point at position (0.5,0.0)
// * *
// * * 3b insert line 2 1
// * * * The second subcommand inserts a new line and makes the two points
// * * * children of the new line. This splits the command further:
// * * * *
// * * * * 3b.1 insert line 0.5,0 0,0 # objectID = 3
// * * * * * The line is inserted.
// * * * *
// * * * * 3b.2 children 2 add [2,1]
// * * * * * The newly created line's children are updated.
// * * * *
// * * * * 3b.3 parents 0 add [3]
// * * * * * The left point's parents are updated.
// * * * *
// * * * * 3b.4 parents 1 add [3]
// * * * * * The right point's parents are updated.
//
// 4 select history.lines.-1
// * This command splits into two subcommands:
// * *
// * * 4a select push
// * * * A new selection is pushed onto the selection stack
// * *
// * * 4b select add 3
// * * * The line is added to the selection on the top of the stack
//
// 5 append bezier quadratic 0.25,0.25 0.25,-0.25
// * The append command is an insert followed by a join:
// * *
// * * 5a insert bezier quadratic 0.25,0.25 0.25,-0.25
// * * * The bezier is inserted with control points:
// * * * C1 = 0.0,0.0 (cursor position)
// * * * C2 = 0.25,0.25
// * * * C3 = 0.25,-0.25
// * * * The algorithm discovers that this bezier starts on the left point of
// * * * the line and that it intersects the line itself. The intersection
// * * * point splits the curve in two. Several things happen:
// * * * *
// * * * * 5a.1 insert point 0.25,0.25 # objectID = 4
// * * * * 5a.2 insert point 0.25,-0.25 # objectID = 5
// * * * * * The missing control points are inserted
// * * * *
// * * * * 5a.3 insert bezier quadratic 1 4 5 # objectID = 6
// * * * * * The main curve object is inserted before anything else
// * * * *
// * * * * 5a.4 insert point ("intersection point") # objectID = 7
// * * * * * Any intersection points are then inserted.
// * * * *
// * * * * 5a.5 insert subcurve 6 0.0 ("intersection parameter") # objectID = 8
// * * * * 5a.6 insert subcurve 6 ("intersection parameter") 1.0 # objectID = 9
// * * * * 5a.7 insert subcurve 3 0.0 ("intersection paramater") # objectID = 10
// * * * * 5a.8 insert subcurve 3 ("intersection parameter") 1.0 # objectID = 11
// * * * * * The subcurves are inserted next.
// * * * *
// * * * * 5a.9 insert area [11, 8]
// * * * * * The area that was created between objects 11 and 8 is inserted.
//
// on intersection of two curves, both of those curves split into two subcurves
// the intersection always happens on curves that do not have subcurves
// nor do they have any stray points lying on them
// the subcurves are inserted as children into the intersecting curves
// the endpoints of the intersecting curves are moved as children
// of the subcurves, unless the parent curve is held by a strong link to them,
// then they're shared both by the parent curve and by the subcurve
//
// todo: see how many lines does a discretization of a bezier curve take
// * * * *
// * * * * 5a.10
// * * * * *
// // swap the points of the line, since it also gets split
// * * * *
// * * * * 5a.7 insert area
// * * * *
// * * * * 5a.8 children 6 add [8, 9]
// * * * * 5a.9 parents 8 add [6]
// * * * * 5a.10 parents 9 add [6]
// * * * * *
// * * * * 5a.2 parents 0 add [4]
// * * * * * The left point's parents are updated with the curve.
// * * * * *

// Puts the two furthest points in out1 and out2.
// n must be at least 2.
// Currently this algorithm takes n * (n - 1) / 2
// vector subtractions, multiplications, summations and scalar comparisons.

logint_decl(uint32_t, loguint32);

static inline uint32_t dyn_object_stack_size(const uint32_t *stack)
{
        return stack[0];
}

static inline int dyn_object_stack_push(uint32_t item, uint32_t *stack)
{
        uint32_t *new_stack_data;
        size_t sz = dyn_object_stack_size(stack);

        if (sz >= 15 && BIT(loguint32(sz + 1)) == sz + 1) {
                new_stack_data = realloc(stack, 2 * (sz + 1) * sizeof(*stack));
                if (!new_stack_data) {
                        return -1;
                }
        }

        stack[sz + 1] = item;
        stack[0] += 1;

        return 0;
}

static inline uint32_t dyn_object_stack_peek(const uint32_t *stack)
{
        return stack[dyn_object_stack_size(stack)];
}

static inline Object *identify_object(const ObjectID *id,
                                      Object *historical,
                                      const uint32_t *latest)
{
        if (id->flagged_index & RBIT(id->flagged_index, 0)) {
                return historical
                     + dyn_object_stack_peek(
                               &latest[id->flagged_index
                                       & ~RBIT(id->flagged_index, 0)]);
        }
        return historical + id->flagged_index;
}

#define Location(ptr_type)                                                     \
        union {                                                                \
                ptr_type *address;                                             \
                uint64_t offset;                                               \
        }
#define location_make_absolute(base, location)                                 \
        ((location)->address =                                                 \
                 (void *)((unsigned char *)(base) + (location)->offset))
#define location_make_relative(base, location)                                 \
        ((location)->offset = (unsigned char *)(location)->address             \
                            - (unsigned char *)(base))

#define Stack(n) int

typedef struct {
        alignas(16) vec2d cursor_position;
        Location(Object) object_tree;
        Stack(BTreeRoot(ObjectID)) selection_stack;
} HistoryEntry;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define MAX_LINEAR_HISTORIES UINT32_MAX

typedef struct LinearHistory_ LinearHistory;

struct LinearHistory_ {
        uint64_t size;
        Location(HistoryEntry) entries;
        uint32_t parent; // root if refers to itself
        uint32_t children[2]; // not a child if refers to itself
};
#ifdef CACHE_LINE_SIZE
static_assert(sizeof(LinearHistory) <= CACHE_LINE_SIZE);
#endif

//

typedef struct History_ History;

struct History_ {
        uint64_t size;
        uint64_t capacity;
        Location(LinearHistory) tree;
        Location(HistoryEntry) entries;
        uint64_t current_entry;
};

typedef struct TieFileHeader_ TieFileHeader;

struct TieFileHeader_ {
        uint8_t magic_byte; // 0x83
        int8_t tie_string[3];
        uint16_t major;
        uint16_t minor;
        History history;
};
