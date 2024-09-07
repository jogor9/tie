#ifndef TIE_BTREE_H
#define TIE_BTREE_H

#include <stdbool.h>

#include "array.h"
#include "base_array.h"

#define btree_decl(                                                            \
        n, keytype, name, attribs, alloc, dealloc, p, q, expr, usertype, user) \
        typedef struct BTree##n##name##_ BTree##n##name;                       \
        struct BTree##n##name##_ {                                             \
                BTree##n##name *parent;                                        \
                BTree##n##name *children[n];                                   \
                size_t keycount;                                               \
                keytype keys[(n) - 1];                                         \
        };                                                                     \
        PURE_FUNC attribs BTree##n##name btree##n##name##_get_empty(void)      \
        {                                                                      \
                BTree##n##name tree = {                                        \
                        .parent = NULL,                                        \
                        .keycount = 0,                                         \
                };                                                             \
                return tree;                                                   \
        }                                                                      \
        attribs const keytype *btree##n##name##_search(                        \
                const keytype *restrict q,                                     \
                const BTree##n##name *restrict tree,                           \
                const BTree##n##name **restrict node,                          \
                usertype user)                                                 \
        {                                                                      \
                const keytype *p;                                              \
                int s = 0; /* set due to the empty tree case */                \
                                                                               \
                do {                                                           \
                        take_array(tree->keycount, p, tree->keys) {            \
                                s = sgn(expr);                                 \
                                if (s != 1)                                    \
                                        break;                                 \
                        }                                                      \
                        if (s == 0 || !tree->children[p - tree->keys]) {       \
                                if (node)                                      \
                                        *node = tree;                          \
                                return p;                                      \
                        }                                                      \
                        tree = tree->children[p - tree->keys];                 \
                } while (true);                                                \
        }                                                                      \
        BTree *btree##n##name##_insert(const keytype *restrict q,              \
                                       BTree##n##name *restrict tree,          \
                                       usertype user)                          \
        {                                                                      \
                BTree *ins;                                                    \
                ketype *p, temp, *r;                                           \
                                                                               \
                p = (keytype *)btree##n##name##_search(                        \
                        q, tree, (const BTree **)&ins, user);                  \
                if (p < ins->keys + ins->keycount && (expr) == 0)              \
                        return ins;                                            \
                                                                               \
                if (ins->keycount < (n) - 1) {                                 \
                        ins->keys[ins->keycount] = *q;                         \
                        shift_left(r, ins->keys + ins->keycount, p, temp);     \
                        ins->keycount += 1;                                    \
                }                                                              \
        }

typedef struct BTree5_ BTree5;

struct BTree5_ {
        BTree5 *parent;
        BTree5 *children[5];
        int c;
        int keys[4];
};

typedef struct {
        const BTree5 *node;
        int key;
} BTreeSearch;

BTreeSearch btree_search(int key, const BTree5 *tree)
{
        BTreeSearch result = {
                .key = 0,
                .node = tree,
        };
        const int *p;
        int i;

        do {
                find_ordered(&key, p, tree->keys, tree->keys + tree->c - 1);
                i = p - tree->keys;
                if (tree->children[i])
                        tree = tree->children[i];
        } while (tree->keys[i] != key && tree->children[i]);

        result.node = tree;
        result.key = i;
        return result;
}

BTree5 *btree_insert(int key, BTree5 *tree)
{
        BTreeSearch search = btree_search(key, tree);

        if (search.key < search.node->c - 1
            && search.node->keys[search.key] == key)
                return tree;
}

#endif
