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
  t_type,
  t_truth_value,
  t_pointer,
  t_typed_pointer,
  t_string,
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
  t_pixel_value,
  t_scalar_value,
  t_position_2d,
  t_tuple,
  t_list,
  t_disjoint_set,
  t_graph,
  t_node,
  t_attribute,
  t_attribute_list,
  t_attribute_stat,
  t_attribute_2d_pos,
  t_link,
  t_link_head,
  /* tree annotation types */
  t_statistics,
  t_raw_moments,
  t_accumulated_stat,
  t_neighborhood_stat,
  t_edge_response,
  t_link_measure,
  t_edge_profile,
  t_edge_links,
  t_boundary_message,
  t_boundary,
  t_segment_message,
  t_segment,
  /* parsing context types */
  t_stat_accumulator,
  t_pixel_image,
  t_node_ref
} type_label;

#define t_real t_F32
/*
#if INTEGRAL_IMAGE_DATA_TYPE == INTEGRAL_IMAGE_USING_FLOAT

#define t_real t_F32

#elif INTEGRAL_IMAGE_DATA_TYPE == INTEGRAL_IMAGE_USING_DOUBLE

#define t_real t_F64

#else
#error "integral image data type not defined"
#endif
*/
/**
 * Stores a generic pointer value, with attached type label and possibility of
 * storing multiple values (arrays or tuples) in one object. Intended to be
 * used with 'expect_x' functions that cast the pointer to the expected value
 * and generate an error if the type doesn't match.
 */
typedef struct typed_pointer_t {
  type_label type;
  uint32 count;
  uint32 token;
  pointer value; /* TODO: change name to data? */
} typed_pointer;

typed_pointer *typed_pointer_alloc();

void typed_pointer_free
(
  typed_pointer *ptr
);

/**
 * Creates a typed pointer containing the given amount of values of given type.
 */
result typed_pointer_create
(
  typed_pointer *tptr,
  type_label type,
  uint32 count,
  uint32 token,
  pointer value
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
  const typed_pointer *tptr
);

/**
 * Checks whether the type of the value is typed_pointer.
 */
truth_value is_typed_pointer
(
  const typed_pointer *tptr
);

/**
 * Clones the structure of a typed_pointer, without copying the data. The old
 * content of the target will be destroyed.
 */
result typed_pointer_clone
(
  typed_pointer *target,
  typed_pointer *source
);

/**
 * Copies the content of a typed_pointer into another typed_pointer. The old
 * content of target will be destroyed.
 */
result typed_pointer_copy
(
  typed_pointer *target,
  typed_pointer *source
);

/**
 * Sets the value of typed_pointer. Care must be taken to ensure that the value
 * pointed to actually matches the type_label of the typed_pointer.
 */
result typed_pointer_set_value
(
  typed_pointer *tptr,
  uint32 index,
  pointer new_value
);

/******************************************************************************/

typedef real (*typed_pointer_cast_from_function)(typed_pointer *tptr);

real typed_pointer_cast_from
(
  typed_pointer *tptr
);

void typed_pointer_cast_into
(
  typed_pointer *tptr,
  real value
);

/******************************************************************************/

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
  const typed_pointer *tuple,
  type_label type
);

/**
 * Checks whether the type of the value is tuple.
 */
truth_value is_tuple
(
  const typed_pointer *tptr
);

/**
 * Ensures that a typed pointer contains one element with a given type. If the
 * element is not found, it is added.
 */
result ensure_has
(
  typed_pointer *tptr,
  type_label type,
  typed_pointer **res
);

/**
 * Ensures that a typed pointer is of given type. If it is not, the old value is
 * destroyed and replaced with the new one.
 */
result ensure_is
(
  typed_pointer *tptr,
  type_label type,
  typed_pointer **res
);

/******************************************************************************/
/* macros for managing typed pointers and tuples                              */

#define IS_TYPE(tptr,exp)\
  ((tptr != NULL && tptr->type == t_##exp) ? (exp*)tptr->value : NULL)

#define HAS_TYPE(tptr,ttype,ttoken)\
  (((tptr) != NULL && (tptr)->type == t_##ttype) ? \
  ((tptr)->token == (ttoken) ? (ttype*)((tptr)->value) : NULL) : \
  ({ typed_pointer *elem = tuple_has_type((tptr), t_##ttype, 1, 1);\
    (elem != NULL && elem->token == (ttoken)) ? (ttype*)(elem->value) : NULL }))

#define EXPECT_TYPE(tptr,exp)\
  ((tptr != NULL && tptr->type == exp) ? ((exp*)tptr->value) : \
  ERROR(BAD_TYPE),NULL)

#define ENSURE_HAS(tptr,type)\
  (tptr != NULL ? { typed_pointer *elem; CHECK(ensure_has(tptr, t_##type, &elem)); (type*)elem->value; } : NULL)



#ifdef __cplusplus
}
#endif

#endif /* CVSU_TYPED_POINTER_H */
