#ifndef TIE_ATTRIB_H
#define TIE_ATTRIB_H

// __unsequenced__ is more laid back than both __pure__ and __const__,
// so we specify it for both of them if those attributes are not available
#if __has_attribute(__pure__)
#define PURE_FUNC __attribute__((__pure__))
#elif __has_c_attribute(__unsequenced__)
#define PURE_FUNC [[__unsequenced__]]
#else
#define PURE_FUNC
#endif

#if __has_attribute(__const__)
#define CONST_FUNC __attribute__((__const__))
#elif __has_attribute(__pure__)
#define CONST_FUNC __attribute__((__pure__))
#elif __has_c_attribute(__unsequenced__)
#define CONST_FUNC [[__unsequenced__]]
#else
#define CONST_FUNC
#endif

#if __has_attribute(__hot__)
#define HOT_FUNC __attribute__((__hot__))
#else
#define HOT_FUNC
#endif

#if __has_attribute(__malloc__)
#define MALLOC_FUNC(...) __attribute__((__malloc__))
#else
#define MALLOC_FUNC(...)
#endif

#if __has_attribute(__warn_unused_result__)
#define NODISCARD __attribute__((__warn_unused_result__))
#elif __has_attribute(__nodiscard__)
#define NODISCARD __attribute__((__nodiscard__))
#elif __has_c_attribute(__nodiscard__)
#define NODISCARD [[__nodiscard__]]
#else
#define NODISCARD
#endif

#if __has_attribute(__unused__)
#define UNUSED __attribute__((__unused__))
#elif __has_attribute(__maybe_unused__)
#define UNUSED __attribute__((__maybe_unused__))
#elif __has_c_attribute(__maybe_unused__)
#define UNUSED [[__maybe_unused__]]
#else
#define UNUSED
#endif

#endif
