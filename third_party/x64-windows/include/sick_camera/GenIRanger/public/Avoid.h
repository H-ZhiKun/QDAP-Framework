// Copyright 2019-2020 SICK AG. All rights reserved.

#pragma once

/**
 * \def AVOID
 *
 * This macro is used to indicate that a construct should be avoided to be used
 * unless absolutely necessary. In Visual Studio it will cause a deprecation
 * warning.
 */
#ifdef _MSC_VER
#  define AVOID __declspec(deprecated)
#else
#  define AVOID
#endif
