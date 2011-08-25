/**
 * @file alloc.c
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

#include "alloc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

result allocate(byte **dst, size_t dst_size, size_t element_size)
{
    //printf("allocate dst=0x%x dst_size=%d element_size=%d", (unsigned long)dst, (int)dst_size, (int)element_size);
    if (dst == NULL) {
        return BAD_POINTER;
    }
#ifdef USE_MALLOC
    *dst = (byte *)malloc(dst_size * element_size);
#else
    *dst = NULL;
#endif
    if (*dst == NULL) {
        return BAD_POINTER;
    }
    return SUCCESS;
}

result deallocate(byte **dst)
{
    if (dst == NULL) {
        return BAD_POINTER;
    }
#ifdef USE_MALLOC
    if (*dst != NULL) {
        free(*dst);
        *dst = NULL;
    }
#endif
    return SUCCESS;
}

result reset(byte *dst, size_t dst_size, size_t element_size)
{
    if (dst == NULL) {
        return BAD_POINTER;
    }
    memset(dst, 0, dst_size * element_size);
    return SUCCESS;
}
