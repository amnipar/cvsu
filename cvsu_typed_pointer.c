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
#include "cvsu_context.h"
#include "cvsu_annotation.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string typed_pointer_create_name = "typed_pointer_create";
string tuple_create_name ="tuple_create";
string tuple_promote_name = "tuple_promote";
string tuple_extend_name = "tuple_extend";
string tuple_ensure_has_unique_name = "tuple_ensure_has_unique";

string ensure_is_name = "ensure_is";
string ensure_has_name = "ensure_has";

/******************************************************************************/

uint32 typesize[] = {
  0,
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
  sizeof(typed_pointer),
  sizeof(list),
  sizeof(statistics),
  sizeof(accumulated_stat),
  sizeof(neighborhood_stat),
  sizeof(accumulated_reg),
  sizeof(edge_response),
  sizeof(boundary_strength),
  sizeof(quad_forest_edge), /* TODO: change to boundary? */
  sizeof(segment_strength),
  sizeof(quad_forest_segment), /* TODO: change to segment */
  sizeof(quad_forest_intersection), /* TODO: change to intersection? */
  sizeof(stat_accumulator),
  sizeof(reg_accumulator),
  sizeof(ridge_finder),
  sizeof(path_sniffer),
  sizeof(edge_parser),
  sizeof(segment_parser)
};

/******************************************************************************/

result typed_pointer_create
(
  typed_pointer *tptr,
  type_label type,
  uint32 count
)
{
  TRY();

  CHECK_POINTER(tptr);
  typed_pointer_destroy(tptr);
  CHECK(memory_allocate((data_pointer*)&tptr->value, count, typesize[((uint32)type)]));
  CHECK(memory_clear((data_pointer)tptr->value, count, typesize[((uint32)type)]));
  tptr->type = type;
  tptr->count = count;

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
    if (tptr->type == t_TUPLE) {
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
  typed_pointer *tptr
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
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_TPOINTER) {
    return TRUE;
  }
  return FALSE;
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

  tuple->type = t_TUPLE;
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
  if (tuple != NULL && tuple->value != NULL && tuple->type == t_TUPLE) {
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
  tptr->type = t_TUPLE;
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
  CHECK_PARAM(tuple->type == t_TUPLE);

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
    CHECK(typed_pointer_create(tuple, type, 1));
    *res = tuple;
  }
  else {
    uint32 i;
    typed_pointer *elements;
    if (tuple->type != t_TUPLE) {
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
      CHECK(typed_pointer_create(&new_ptr, type, 1));
      CHECK(tuple_extend(tuple, &new_ptr, res));
    }
  }

  FINALLY(tuple_ensure_has_unique);
  RETURN();
}

/******************************************************************************/

typed_pointer *tuple_has_type
(
  typed_pointer *tuple,
  type_label type,
  uint32 count,
  uint32 index
)
{
  if (tuple != NULL && tuple->type == t_TUPLE) {
    uint32 i;
    typed_pointer *elements;

    elements = (typed_pointer*)tuple->value;
    for (i = 0; i < tuple->count; i++) {
      if (elements[i].type == type) {
        return &elements[i];
      }
    }
    return NULL;
    /* if count is specified, first checks that the tuple has the correct */
    /* number of items of specified type, then returns the item with given index */
    /*
    if (count > 0) {
      num = 0;
      for (i = 0; i < tuple->count; i++) {
        if (elements[i].type == type) {
          num++;
        }
      }
      if (num == count && index <= count) {
        return &elements[index-1];
      }
      else {
        return NULL;
      }
    }
    */
    /* otherwise just returns the first occurrence of the specified type */
    /*
    else {
      for (i = 0; i < tuple->count; i++) {
        if (elements[i].type == type) {
          return &elements[i];
        }
      }
      return NULL;
    }
    */
  }
  return NULL;
}

/******************************************************************************/

truth_value is_tuple
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_TUPLE) {
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

  CHECK_POINTER(res);
  *res = NULL;
  CHECK_POINTER(tptr);

  if (tptr->type == type) {
    *res = tptr;
  }
  else
  if (tptr->type == t_UNDEF) {
    CHECK(typed_pointer_create(tptr, type, 1));
    *res = tptr;
  }
  else {
    uint32 i;
    typed_pointer *elements;
    if (tptr->type != t_TUPLE) {
      CHECK(tuple_promote(tptr));
    }
    else {
      elements = (typed_pointer*)tptr->value;
      for (i = 0; i < tptr->count; i++) {
        if (elements[i].type == type) {
          *res = &elements[i];
          break;
        }
      }
    }
    if (*res == NULL) {
      typed_pointer new_ptr;
      typed_pointer_nullify(&new_ptr);
      CHECK(typed_pointer_create(&new_ptr, type, 1));
      CHECK(tuple_extend(tptr, &new_ptr, res));
    }
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
    CHECK(typed_pointer_create(tptr, type, 1));
  }
  *res = tptr;

  FINALLY(ensure_is);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
