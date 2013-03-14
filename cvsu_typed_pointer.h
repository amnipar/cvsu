/**
 * @file cvsu_typed_pointer.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Generic pointers with type annotations.
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

#ifndef CVSUTYPES_H
#   define CVSUTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"

typedef enum type_label_t {
  t_UNDEF = 0,
  /* basic types */
  t_TYPE,
  t_TRUTH,
  t_POINTER,
  t_TPOINTER,
  t_STRING,
  t_S8,
  t_U8,
  t_S16,
  t_U16,
  t_S32,
  t_U32,
  t_S64,
  t_U64,
  t_F32,
  t_F64,
  t_TUPLE,
  t_LIST,
  /* tree annotation types */
  t_STAT,
  t_ASTAT,
  t_SEGMENT,
  t_BOUNDARY,
  t_INTERSECTION,
  /* parsing context types */
  t_STAT_ACCUMULATOR,
  t_PATH_SNIFFER,
  t_EDGE_PARSER,
  t_SEGMENT_PARSER
} type_label;

/**
 * Stores a generic pointer value, with attached type label and possibility of
 * storing multiple values (arrays or tuples) in one object. Intended to be
 * used with 'expect_x' functions that cast the pointer to the expected value
 * and generate an error if the type doesn't match.
 */
typedef struct typed_pointer_t {
  type_label type;
  uint32 count;
  pointer value;
} typed_pointer;

/**
 * A convenience function for initializing the fields of a typed_pointer.
 */
void typed_pointer_create
(
  typed_pointer *tptr,
  type_label type,
  uint32 count,
  pointer value
);

/**
 * Deallocates the memory and sets the pointer to NULL.
 */
void typed_pointer_destroy
(
  typed_pointer *tptr
);

/**
 * Tuple is basically an array of typed pointers.
 */
typedef typed_pointer tuple;

/**
 * Creates a tuple, which is basically a typed pointer with multiple typed
 * pointer values inside. Each value may have different type. For tuples with
 * specific types inside, need to create special is-, has- and expect-functions.
 */
make_tuple
(
  typed_pointer *tptr,
  typed_pointer *tuple
);

truth_value is_tuple
(
  typed_pointer *tptr
);

/**
 * A convenience macro for creating pointers, ensures that the possible previous
 * value is deallocated first. Requires that a make_x function exists for the
 * given type x, taking typed_pointer and x* as parameters.
 * Note: it is important that typed_pointers are always allocated to NULL.
 */
#define CREATE_POINTER(ptr,t,c) {\
  t *temp_ptr;\
  typed_pointer_destroy(ptr);\
  CHECK(memory_allocate((data_pointer*)&temp_ptr, (c), sizeof(t)));\
  make_##t((ptr), temp_ptr);\
  (ptr)->count = (c);\
}

#define CREATE_TUPLE(ptr,c) {\
  typed_pointer *temp_ptr;\
  if ((ptr)->value != NULL) {\
    CHECK(memory_deallocate((data_pointer*)&((ptr)->value)));\
  }\
  CHECK(memory_allocate((data_pointer*)&temp_ptr, (c), sizeof(typed_pointer)));\
  (ptr)->type = t_TUPLE;\
  (ptr)->count = (c);\
  (ptr)->value = (pointer)temp_ptr;\
}

#ifdef __cplusplus
}
#endif

#endif /* CVSU_TYPED_POINTER_H */
