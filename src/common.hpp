#ifndef COMMON_HPP
#define COMMON_HPP

#include <cstdlib>

#ifdef DEBUG
# include <cassert>
#else
# define assert(x) /* x */
#endif

#endif /* COMMON_HPP */
