#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>

/* Returns random integer in [min, max]. */
# define rand_range(min, max) ((std::rand() % (((max) + 1) - (min))) + (min))

#endif
