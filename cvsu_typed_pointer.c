/**
 * @file cvsu_typed_pointer.c
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

#include "cvsu_macros.h"
#include "cvsu_typed_pointer.h"
#include "cvsu_memory.h"
#include "cvsu_list.h"
#include "cvsu_set.h"
#include "cvsu_attribute.h"
#include "cvsu_graph.h"
#include "cvsu_context.h"
#include "cvsu_annotation.h"
#include "cvsu_pixel_image.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string typed_pointer_alloc_name = "typed_pointer_alloc";
string typed_pointer_create_name = "typed_pointer_create";
string typed_pointer_clone_name = "typed_pointer_clone";
string typed_pointer_copy_name = "typed_pointer_copy";
string typed_pointer_set_value_name = "typed_pointer_set_value";
string tuple_create_name ="tuple_create";
string tuple_promote_name = "tuple_promote";
string tuple_extend_name = "tuple_extend";
string tuple_ensure_has_unique_name = "tuple_ensure_has_unique";

string ensure_is_name = "ensure_is";
string ensure_has_name = "ensure_has";

/******************************************************************************/

uint32 typesize[] = {
  0,
  /* basic types */
  sizeof(type_label),
  sizeof(truth_value),
  sizeof(pointer),
  sizeof(typed_pointer),
  sizeof(string),
  sizeof(sint8),
  sizeof(uint8),
  sizeof(sint16),
  sizeof(uint16),
  sizeof(sint32),
  sizeof(uint32),
/*
  sizeof(sint64),
  sizeof(uint64),
*/
  sizeof(real32),
  sizeof(real64),
  sizeof(pixel_value),
  sizeof(typed_pointer),
  sizeof(list),
  sizeof(disjoint_set),
  sizeof(graph),
  sizeof(node),
  sizeof(attribute),
  sizeof(attribute_list),
  sizeof(attribute_stat),
  sizeof(link),
  sizeof(link_head),
  /* tree annotation types */
  sizeof(statistics),
  sizeof(raw_moments),
  sizeof(accumulated_stat),
  sizeof(neighborhood_stat),
  sizeof(edge_response),
  sizeof(link_measure),
  sizeof(edge_profile),
  sizeof(edge_links),
  sizeof(boundary_message),
  sizeof(boundary),
  sizeof(segment_message),
  sizeof(segment),
  /* parsing context types */
  sizeof(stat_accumulator),
  sizeof(pixel_image)
};

/******************************************************************************/

real cast_from_none
(
  typed_pointer *tptr
)
{
  (void)tptr;
  return 0;
}

real cast_from_unsupported
(
  typed_pointer *tptr
)
{
  (void)tptr;
  printf("Error: casting from unsupported typed_pointer\n");
  return 0;
}

real cast_from_truth_value
(
  typed_pointer *tptr
)
{
  return (real)*((truth_value*)tptr->value);
}

real cast_from_typed_pointer
(
  typed_pointer *tptr
)
{
  return typed_pointer_cast_from((typed_pointer*)tptr->value);
}

real cast_from_s8
(
  typed_pointer *tptr
)
{
  return (real)*((char *)tptr->value);
}

real cast_from_u8
(
  typed_pointer *tptr
)
{
  return (real)*((byte *)tptr->value);
}

real cast_from_s16
(
  typed_pointer *tptr
)
{
  return (real)*((sint16 *)tptr->value);
}

real cast_from_u16
(
  typed_pointer *tptr
)
{
  return (real)*((uint16 *)tptr->value);
}

real cast_from_s32
(
  typed_pointer *tptr
)
{
  return (real)*((sint32 *)tptr->value);
}

real cast_from_u32
(
  typed_pointer *tptr
)
{
  return (real)*((uint32 *)tptr->value);
}

real cast_from_f32
(
  typed_pointer *tptr
)
{
  return (real)*((real32 *)tptr->value);
}

real cast_from_f64
(
  typed_pointer *tptr
)
{
  return (real)*((real64 *)tptr->value);
}

real cast_from_pixel_value
(
  typed_pointer *tptr
)
{
  return ((pixel_value*)tptr->value)->cache;
}

typed_pointer_cast_from_function cast_from_functions[] = {
  &cast_from_none, /* t_UNDEF */
  /* basic types */
  &cast_from_unsupported, /* t_type */
  &cast_from_truth_value, /* t_truth_value */
  &cast_from_unsupported, /* t_pointer */
  &cast_from_typed_pointer, /* t_typed_pointer */
  &cast_from_unsupported, /* t_string */
  &cast_from_s8, /* t_S8 */
  &cast_from_u8, /* t_U8 */
  &cast_from_s16, /*t_S16 */
  &cast_from_u16, /* t_U16 */
  &cast_from_s32, /* t_S32 */
  &cast_from_u32, /* t_U32 */
/*
  t_S64,
  t_U64,
*/
  &cast_from_f32, /* t_F32 */
  &cast_from_f64, /* t_F64 */
  &cast_from_pixel_value, /* t_pixel_value */
  &cast_from_unsupported, /* t_tuple */
  &cast_from_unsupported, /* t_list */
  &cast_from_unsupported, /* t_disjoint_set */
  &cast_from_unsupported, /* t_graph */
  &cast_from_unsupported, /* t_node */
  &cast_from_unsupported, /* t_attribute */
  &cast_from_unsupported, /* t_attribute_list */
  &cast_from_unsupported, /* t_attribute_stat */
  &cast_from_unsupported, /* t_link */
  &cast_from_unsupported, /* t_link_head */
  /* tree annotation types */
  &cast_from_unsupported, /* t_statistics */
  &cast_from_unsupported, /* t_raw_moments */
  &cast_from_unsupported, /* t_accumulated_stat */
  &cast_from_unsupported, /* t_neighborhood_stat */
  &cast_from_unsupported, /* t_edge_response */
  &cast_from_unsupported, /* t_link_measure */
  &cast_from_unsupported, /* t_edge_profile */
  &cast_from_unsupported, /* t_edge_links */
  &cast_from_unsupported, /* t_boundary_message */
  &cast_from_unsupported, /* t_boundary */
  &cast_from_unsupported, /* t_segment_message */
  &cast_from_unsupported, /* t_segment */
  /* parsing context types */
  &cast_from_unsupported, /* t_stat_accumulator */
  &cast_from_unsupported, /* t_pixel_image */
};

/******************************************************************************/

typed_pointer *typed_pointer_alloc
()
{
  TRY();
  typed_pointer *ptr;

  CHECK(memory_allocate((data_pointer*)&ptr, 1, sizeof(typed_pointer)));
  typed_pointer_nullify(ptr);

  FINALLY(typed_pointer_alloc);
  return ptr;
}

/******************************************************************************/

void typed_pointer_free
(
  typed_pointer *ptr
)
{
  if (ptr != NULL) {
    typed_pointer_destroy(ptr);
    memory_deallocate((data_pointer*)&ptr);
  }
}

/******************************************************************************/

result typed_pointer_create
(
  typed_pointer *tptr,
  type_label type,
  uint32 count,
  uint32 token,
  pointer value
)
{
  TRY();
  uint32 size;

  CHECK_POINTER(tptr);
  typed_pointer_destroy(tptr);
  size = typesize[((uint32)type)];
  CHECK(memory_allocate((data_pointer*)&tptr->value, count, size));
  CHECK(memory_clear((data_pointer)tptr->value, count, size));
  tptr->type = type;
  tptr->count = count;
  tptr->token = token;
  if (value != NULL) {
    CHECK(memory_copy((data_pointer)tptr->value, (data_pointer)value, count,
                      size));
  }

  FINALLY(typed_pointer_create);
  RETURN();
}

/******************************************************************************/

void typed_pointer_destroy
(
  typed_pointer *tptr
)
{
  if (IS_FALSE(typed_pointer_is_null(tptr))) {
    if (tptr->type == t_tuple) {
      tuple_destroy(tptr);
    }
    else {
      memory_deallocate((data_pointer*)&tptr->value);
      typed_pointer_nullify(tptr);
    }
  }
}

/******************************************************************************/

void typed_pointer_nullify
(
  typed_pointer *tptr
)
{
  /* TODO: create some code generation cues, like ensure_not_null */
  /* if the same thing has been already done in chain, can be pruned */
  if (tptr != NULL) {
    tptr->type = t_UNDEF;
    tptr->count = 0;
    tptr->token = 0;
    tptr->value = NULL;
  }
}

/******************************************************************************/

truth_value typed_pointer_is_null
(
  const typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->value != NULL) {
    return FALSE;
  }
  return TRUE;
  /* TODO: here would benefit from three-state truth value - undef, true, false */
  /* struct pointer that is null is actually undefined, not null */
  /* structure with (type-specific) null contents would be null */
}

/******************************************************************************/

truth_value is_typed_pointer
(
  const typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_typed_pointer) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

result typed_pointer_clone
(
  typed_pointer *target,
  typed_pointer *source
)
{
  TRY();
  if (target->type != source->type || target->count != source->count) {
    CHECK(typed_pointer_create(target, source->type, source->count,
                               source->token, NULL));
  }
  FINALLY(typed_pointer_clone);
  RETURN();
}

/******************************************************************************/

result typed_pointer_copy
(
  typed_pointer *target,
  typed_pointer *source
)
{
  TRY();

  if (target->type != source->type || target->count != source->count) {
    CHECK(typed_pointer_create(target, source->type, source->count,
                               source->token, source->value));
  }
  else {
    if (source->value != NULL) {
      CHECK(memory_copy((data_pointer)target->value, (data_pointer)source->value,
                        source->count, typesize[((uint32)source->type)]));
    }
    target->token = source->token;
  }

  FINALLY(typed_pointer_copy);
  RETURN();
}

/******************************************************************************/

result typed_pointer_set_value
(
  typed_pointer *tptr,
  uint32 index,
  pointer new_value
)
{
  TRY();
  uint32 size;

  CHECK_POINTER(tptr);
  CHECK_PARAM(index < tptr->count);

  size = typesize[tptr->type];
  if (new_value != NULL) {
    CHECK(memory_copy(((data_pointer)&tptr->value) + index * size,
                      (data_pointer)new_value, 1, size));
  }
  FINALLY(typed_pointer_set_value);
  RETURN();
}

/******************************************************************************/

real typed_pointer_cast_from
(
  typed_pointer *tptr
)
{
  return (cast_from_functions[tptr->type])(tptr);
}

/******************************************************************************/

void typed_pointer_cast_into
(
  typed_pointer *tptr,
  real value
)
{
  (void)tptr;
  (void)value;
}

/******************************************************************************/

result tuple_create
(
  typed_pointer *tuple,
  uint32 count
)
{
  TRY();

  CHECK_POINTER(tuple);

  /* ensure the possible previous data is destroyed */
  typed_pointer_destroy(tuple);

  tuple->type = t_tuple;
  CHECK(memory_allocate((data_pointer*)&tuple->value, count, sizeof(typed_pointer)));
  tuple->count = count;

  FINALLY(tuple_create);
  RETURN();
}

/******************************************************************************/

void tuple_destroy
(
  typed_pointer *tuple
)
{
  if (tuple != NULL && tuple->value != NULL && tuple->type == t_tuple) {
    uint32 i;
    typed_pointer *values;
    values = (typed_pointer*)tuple->value;
    for (i = 0; i < tuple->count; i++) {
      typed_pointer_destroy(&values[i]);
    }
    memory_deallocate((data_pointer*)&tuple->value);
    typed_pointer_nullify(tuple);
  }
}

/******************************************************************************/

result tuple_promote
(
  typed_pointer *tptr
)
{
  TRY();
  typed_pointer *values, element;

  CHECK_POINTER(tptr);

  /* make a copy of the previous content */
  element = *tptr;
  tptr->type = t_tuple;
  tptr->count = 0;
  CHECK(memory_allocate((data_pointer*)&values, 1, sizeof(typed_pointer)));
  values[0] = element;
  tptr->value = (pointer)values;
  tptr->count = 1;
  /* then nullify and create a tuple instead */
  /*
  typed_pointer_nullify(tptr);
  CHECK(tuple_create(tptr, 1));
  */
  /* copy the previous content as the first value of the tuple */
  /*
  values = (typed_pointer*)tptr->value;
  */

  FINALLY(tuple_promote);
  RETURN();
}

/******************************************************************************/

result tuple_extend
(
  typed_pointer *tuple,
  typed_pointer *tptr,
  typed_pointer **res
)
{
  TRY();
  typed_pointer *new_values, *old_values;
  uint32 i, count;

  CHECK_POINTER(tuple);
  CHECK_PARAM(tuple->type == t_tuple);

  count = tuple->count + 1;

  old_values = (typed_pointer*)tuple->value;
  CHECK(memory_allocate((data_pointer*)&new_values, count, sizeof(typed_pointer)));
  for (i = 0; i < tuple->count; i++) {
    new_values[i] = old_values[i];
  }
  new_values[i] = *tptr;
  memory_deallocate((data_pointer*)&tuple->value);
  tuple->value = (pointer)new_values;
  tuple->count = count;
  if (res != NULL) {
    *res = &new_values[i];
  }

  FINALLY(tuple_extend);
  RETURN();
}

/******************************************************************************/

result tuple_ensure_has_unique
(
  typed_pointer *tuple,
  type_label type,
  typed_pointer **res
)
{
  TRY();

  CHECK_POINTER(res);
  *res = NULL;
  CHECK_POINTER(tuple);

  if (tuple->type == type) {
    *res = tuple;
  }
  else
  if (tuple->type == t_UNDEF) {
    CHECK(typed_pointer_create(tuple, type, 1, 0, NULL));
    *res = tuple;
  }
  else {
    uint32 i;
    typed_pointer *elements;
    if (tuple->type != t_tuple) {
      CHECK(tuple_promote(tuple));
    }
    else {
      elements = (typed_pointer*)tuple->value;
      for (i = 0; i < tuple->count; i++) {
        if (elements[i].type == type) {
          *res = &elements[i];
          break;
        }
      }
    }
    if (*res == NULL) {
      typed_pointer new_ptr;
      typed_pointer_nullify(&new_ptr);
      CHECK(typed_pointer_create(&new_ptr, type, 1, 0, NULL));
      CHECK(tuple_extend(tuple, &new_ptr, res));
    }
  }

  FINALLY(tuple_ensure_has_unique);
  RETURN();
}

/******************************************************************************/

typed_pointer *tuple_has_type
(
  const typed_pointer *tuple,
  type_label type
)
{
  if (tuple != NULL && tuple->type == t_tuple) {
    uint32 i;
    typed_pointer *elements;

    elements = (typed_pointer*)tuple->value;
    for (i = 0; i < tuple->count; i++) {
      if (elements[i].type == type) {
        return &elements[i];
      }
    }
    return NULL;
  }
  return NULL;
}

/******************************************************************************/

truth_value is_tuple
(
  const typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_tuple) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

result ensure_has
(
  typed_pointer *tptr,
  type_label type,
  typed_pointer **res
)
{
  TRY();
  typed_pointer *new_pointer;

  CHECK_POINTER(tptr);

  new_pointer = NULL;

  if (tptr->type == type) {
    new_pointer = tptr;
  }
  /* turn also undef into tuple */
  /*
  else {
  if (tptr->type == t_UNDEF) {
    CHECK(typed_pointer_create(tptr, type, 1, 0, NULL));
    new_pointer = tptr;
  }
  */
  else {
    uint32 i;
    typed_pointer *elements;
    if (tptr->type != t_tuple) {
      CHECK(tuple_promote(tptr));
    }
    else {
      elements = (typed_pointer*)tptr->value;
      for (i = 0; i < tptr->count; i++) {
        if (elements[i].type == type) {
          new_pointer = &elements[i];
          break;
        }
      }
    }
  }
  /* if not found, add to tuple */
  if (new_pointer == NULL) {
    typed_pointer new_element;
    typed_pointer_nullify(&new_element);
    CHECK(typed_pointer_create(&new_element, type, 1, 0, NULL));
    CHECK(tuple_extend(tptr, &new_element, &new_pointer));
  }

  if (res != NULL) {
    *res = new_pointer;
  }

  FINALLY(ensure_has);
  RETURN();
}

/******************************************************************************/

result ensure_is
(
  typed_pointer *tptr,
  type_label type,
  typed_pointer **res
)
{
  TRY();

  CHECK_POINTER(res);
  *res = NULL;
  CHECK_POINTER(tptr);

  if (tptr->type != type) {
    CHECK(typed_pointer_create(tptr, type, 1, 0, NULL));
  }
  *res = tptr;

  FINALLY(ensure_is);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
