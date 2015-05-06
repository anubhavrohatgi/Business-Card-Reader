#ifndef BC_COMMON_H
#define BC_COMMON_H

#include <cmath>
#include <algorithm> 
#include <functional> 
#include <cctype>

#include <opencv2/core/core.hpp>

#include "bcr_debug.h"


#define BCR_NAMESPACE bcr

#define BCR_BEGIN_NAMESPACE namespace BCR_NAMESPACE {
#define BCR_END_NAMESPACE	}

#define BCR_FLT_MAX			(FLT_MAX/128)

#include "string_helpers.h"
#include "opencv_helpers.h"
#include "math_helpers.h"
#include "container_helpers.h"

#define SQUARED(X) ((X) *(X))


BCR_BEGIN_NAMESPACE



BCR_END_NAMESPACE

#endif // BC_COMMON_H