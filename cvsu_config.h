/**
 * @file cvsu_config.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Configuration flags for cvsu.
 *
 * Copyright (c) 2011-2013, Matti Johannes Eskelinen
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CVSU_CONFIG_H
#   define CVSU_CONFIG_H

/**
 * Check that we have stddef.h
 * If not, we need our own size_t definition.
 */
#undef HAVE_STDDEF_H

/**
 * Check that we have stdbool.h
 * If not, we need our own bool definition.
 */
#undef HAVE_STDBOOL_H

/**
 * Check that we have limits.h
 * If not, we need our own MAX definitions.
 */
#undef HAVE_LIMITS_H

/* later, these will be handled by autoconf... */

/* define this if stddef.h is available */
#define HAVE_STDDEF_H 1

/* define this if stdbool.h is available */
#define HAVE_STDBOOL_H 1

/**
 * Define memory allocation method.
 * @note If some other allocation method than malloc is used, a flag should be
 * defined here. Then the method should be added to cvsu_alloc.c. The
 * deallocation method should be defined as well, since free is used only when
 * MEMORY_ALLOCATION_WITH_MALLOC is defined.
 */
#define MEMORY_ALLOCATION_WITH_MALLOC 0
#define MEMORY_ALLOCATION_METHOD MEMORY_ALLOCATION_WITH_MALLOC

/**
 * Define memory copy method.
 */
#define MEMORY_COPY_WITH_MEMCPY 0
/* #define MEMORY_COPY_WITH_XXX 1 */
#define MEMORY_COPY_METHOD MEMORY_COPY_WITH_MEMCPY
/* #define MEMORY_COPY_METHOD MEMORY_COPY_WITH_XXX */

/**
 * Define memory clear method.
 */
#define MEMORY_CLEAR_WITH_MEMSET 0
/* #define MEMORY_CLEAR_WITH_XXX 1 */
#define MEMORY_CLEAR_METHOD MEMORY_CLEAR_WITH_MEMSET
/* #define MEMORY_CLEAR_METHOD MEMORY_CLEAR_WITH_XXX */

/**
 * Define output method.
 * @note If some other output method than printf is used, a flag should be
 * defined here. Then the output macros should be defined as well in
 * cvsu_output.h.
 */
#define OUTPUT_DISABLED 0
#define OUTPUT_WITH_STDIO 1
/* #define OUTPUT_WITH_XXX 2*/
#define OUTPUT_METHOD OUTPUT_WITH_STDIO

/**
 * Define output level.
 * Decides what kinds of things are printed out.
 */
#define OUTPUT_LEVEL_NONE 0
#define OUTPUT_LEVEL_ERRORS 1
#define OUTPUT_LEVEL_WARNINGS 2
#define OUTPUT_LEVEL_INFO 3
#define OUTPUT_LEVEL_DEBUG 4
#define OUTPUT_LEVEL OUTPUT_LEVEL_ERRORS

/**
 * If fmin and fmax functions are not needed by your compiler, undef this
 */
#define NEED_MINMAX 1

/**
 * Define integral image data type.
 * Can use either single or double precision floating point numbers.
 */
#define INTEGRAL_IMAGE_USING_FLOAT  0
#define INTEGRAL_IMAGE_USING_DOUBLE 1
/* #define INTEGRAL_IMAGE_DATA_TYPE INTEGRAL_IMAGE_USING_FLOAT */
#define INTEGRAL_IMAGE_DATA_TYPE INTEGRAL_IMAGE_USING_DOUBLE

#undef INTEGRAL_IMAGE_HIGHER_ORDER_STATISTICS

#endif /* CVSU_CONFIG_H */
