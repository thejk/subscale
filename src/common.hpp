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

#endif /* COMMON_HPP */
