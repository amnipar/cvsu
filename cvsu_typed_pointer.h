/**
 * @file cvsu_typed_pointer.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Generic pointers with type annotations.
 *
 * Copyright (c) 2013, Matti Johannes Eskelinen
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

#ifndef CVSU_TYPED_POINTER_H
#   define CVSU_TYPED_POINTER_H

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
/*
  t_S64,
  t_U64,
*/
  t_F32,
  t_F64,
  t_TUPLE,
  t_LIST,
  /* tree annotation types */
  t_STAT,
  t_ASTAT,
  t_AREG,
  t_SEGMENT,
  t_BOUNDARY,
  t_INTERSECTION,
  /* parsing context types */
  t_STAT_ACCUMULATOR,
  t_REG_ACCUMULATOR,
  t_RANGE_OVERLAP,
  t_RIDGE_FINDER,
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
 * Creates a typed pointer containing the given amount of values of given type.
 */
result typed_pointer_create
(
  typed_pointer *tptr,
  type_label type,
  uint32 count
);

/**
 * Deallocates the memory and sets the structure to NULL value.
 * TODO: should remove 'result' return type from all destroy functions
 */
void typed_pointer_destroy
(
  typed_pointer *tptr
);

/**
 * Sets the structure to NULL value
 * -type is undefined
 * -count is 0
 * -pointer is NULL
 * TODO: should make all nullify functions have void return type
 */
void typed_pointer_nullify
(
  typed_pointer *tptr
);

/**
 * Checks whether the structure contains null value
 */
truth_value typed_pointer_is_null
(
  typed_pointer *tptr
);

/**
 * Checks whether the type of the value is typed_pointer.
 */
truth_value is_typed_pointer
(
  typed_pointer *tptr
);

/**
 * Creates a tuple, which is basically a typed pointer with multiple typed
 * pointer values inside. Each value may have a different type. For tuples with
 * specific types inside, need to create special is-, has- and expect-functions.
 */
result tuple_create
(
  typed_pointer *tptr,
  uint32 count
);

/**
 * Destroys a tuple, by destroying all contained typed pointers
 */
void tuple_destroy
(
  typed_pointer *tuple
);

/**
 * Promotes a typed pointer value into a tuple with one element, containing the
 * previous value contained by the pointer
 */
result tuple_promote
(
  typed_pointer *tptr
);

/**
 * Extends a tuple by one element, adding the new pointer as the last element;
 * returns a pointer to the new element through the res pointer
 */
result tuple_extend
(
  typed_pointer *tuple,
  typed_pointer *tptr,
  typed_pointer **res
);

/**
 * Ensures that a typed pointer has one element with a given type. If the
 * element is found, returns a pointer to that; if it is not found, a new
 * undefined element is added, and a pointer is returned to that. In order to
 * avoid having multiple elements of the given type, all additions should be
 * done with this function.
 */
result tuple_ensure_has_unique
(
  typed_pointer *tuple,
  type_label type,
  typed_pointer **res
);

/**
 * Checks whether the tuple contains the given type, and returns the (first)
 * matching item. If a number larger than 0 is given, checks also that there are
 * exactly that many items with that type, and returns the item with the given
 * (1-based) index. If count is 0, just returns the first matching item.
 */
typed_pointer *tuple_has_type
(
  typed_pointer *tuple,
  type_label type,
  uint32 count,
  uint32 index
);

/**
 * Checks whether the type of the value is tuple.
 */
truth_value is_tuple
(
  typed_pointer *tptr
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_TYPED_POINTER_H */