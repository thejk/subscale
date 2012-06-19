#ifndef COMMON_HPP
#define COMMON_HPP

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstdlib>

#ifdef DEBUG
# include <cassert>
#else
# define assert(x) /* x */
#endif

#if HAVE_CSTDINT
# include <cstdint>
#else
extern "C" {
# include <stdint.h>
};
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


#endif /* COMMON_HPP */
