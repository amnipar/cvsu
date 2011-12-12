/**
 * @file cvsu_memory.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Memory handling routines for the cvsu module.
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

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"

#if (MEMORY_ALLOCATION_METHOD == MEMORY_ALLOCATION_WITH_MALLOC)
#include <stdlib.h>
#elif (MEMORY_ALLOCATION_METHOD == MEMORY_ALLOCATION_WITH_VC)
#include <vcrt.h>
#include <vclib.h>
#include <macros.h>
#include <sysvar.h>
#else
#error "Memory allocation method not defined"
#endif
#if ((MEMORY_COPY_METHOD == MEMORY_COPY_WITH_MEMCPY) || \
     (MEMORY_CLEAR_METHOD == MEMORY_CLEAR_WITH_MEMSET))
#include <string.h>
#else
#error "Memory copy method or memory clear method not defined"
#endif

/******************************************************************************/
/* constants for storing the function names                                   */
/* used in error reporting macros                                             */

string memory_allocate_name = "memory_allocate";
string memory_deallocate_name = "memory_deallocate";
string memory_clear_name = "memory_clear";
string memory_copy_name = "memory_copy";

/******************************************************************************/

result memory_allocate(
    data_pointer *target,
    size_t target_size,
    size_t element_size
    )
{
    TRY();

    CHECK_POINTER(target);

#if (MEMORY_ALLOCATION_METHOD == MEMORY_ALLOCATION_WITH_MALLOC)
    *target = (data_pointer)malloc(target_size * element_size);
#elif (MEMORY_ALLOCATION_METHOD == MEMORY_ALLOCATION_WITH_VC)
    *target = (data_pointer)vcmalloc(target_size * element_size);
#else
#error "Memory allocation method not defined"
    *target = NULL;
#endif

    if (*target == NULL) {
        ERROR(BAD_POINTER);
    }

    FINALLY(memory_allocate);
    RETURN();
}

/******************************************************************************/

result memory_deallocate(
    data_pointer *target
    )
{
    TRY();

    CHECK_POINTER(target);

#if (MEMORY_ALLOCATION_METHOD == MEMORY_ALLOCATION_WITH_MALLOC)
    if (*target != NULL) {
        free(*target);
        *target = NULL;
    }
#elif (MEMORY_ALLOCATION_METHOD == MEMORY_ALLOCATION_WITH_VC)
    if (*target != NULL) {
        vcfree(*target);
        *target = NULL;
    }
#else
#error "Memory allocation method not defined"
#endif

    FINALLY(memory_deallocate);
    RETURN();
}

/******************************************************************************/

result memory_clear(
    data_pointer target,
    size_t target_size,
    size_t element_size
    )
{
    TRY();

    CHECK_POINTER(target);

#if (MEMORY_CLEAR_METHOD == MEMORY_CLEAR_WITH_MEMSET)
    memset(target, 0, target_size * element_size);
#else
#error "Memory clear method not defined"
#endif

    FINALLY(memory_clear);
    RETURN();
}

/******************************************************************************/

result memory_copy(
    data_pointer target,
    const data_pointer source,
    size_t copy_size,
    size_t element_size
    )
{
    TRY();

    CHECK_POINTER(target);
    CHECK_POINTER(source);

#if (MEMORY_COPY_METHOD == MEMORY_COPY_WITH_MEMCPY)
    memcpy(target, source, copy_size * element_size);
#else
#error "Memory copy method not defined"
#endif

    FINALLY(memory_copy);
    RETURN();
}

/******************************************************************************/
