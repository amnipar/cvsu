/**
 * @file alloc.h
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Memory allocation routines.
 *
 * Copyright (c) 2011, Matti Johannes Eskelinen
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ALLOC_H
#   define ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

/**
 * @note If some other allocation method than malloc is used, USE_MALLOC should
 *       be #undef'ed. In this case, the deallocation method should be defined
 *       as well, since free is used only when USE_MALLOC is defined.
 */
#define USE_MALLOC 1

/**
 * A generic function for allocating an array of bytes.
 */
result allocate(byte **dst, size_t dst_size, size_t element_size);

/**
 * A generic function for deallocating an array of bytes.
 */
result deallocate(byte **dst);

/**
 * A generic function for resetting an array of bytes.
 */
result reset(byte *dst, size_t dst_size, size_t element_size);

#ifdef __cplusplus
}
#endif

#endif  /* ALLOC_H */
