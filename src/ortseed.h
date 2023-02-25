#ifndef ORTSEED_H
#define ORTSEED_H

#include <stdint.h>

#include "shared_global.h"

COM_EXTERN_C_EXPORT int64_t ortseed_get_random_seed();

COM_EXTERN_C_EXPORT void ortseed_set_random_seed(int64_t seed);

#endif // ORTSEED_H
