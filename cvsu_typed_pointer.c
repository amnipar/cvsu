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

#include "cvsu_typed_pointer.h"
#include "cvsu_memory.h"

/******************************************************************************/

void typed_pointer_create
(
  typed_pointer *tptr,
  type_label type,
  uint32 count,
  pointer value
)
{
  if (tptr != NULL) {
    tptr->type = type;
    tptr->count = count;
    tptr->value = value;
  }
}

/******************************************************************************/

void typed_pointer_destroy
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->value != NULL) {
    if (tptr->type == t_TUPLE) {
      tuple_destroy(tptr);
    }
    else {
      memory_deallocate((data_pointer*)&tptr->value);
      tptr->type = t_UNDEF;
      tptr->count = 0;
    }
  }
}

/******************************************************************************/

result tuple_create
(
  typed_pointer *tuple,
  uint32 count
)
{
  TRY();
  typed_pointer *values;

  CHECK_POINTER(tuple);

  /* ensure the possible previous data is destroyed */
  typed_pointer_destroy(tuple);

  tptr->type = t_TUPLE;
  CHECK(memory_allocate((data_pointer*)&values, count, sizeof(typed_pointer)));
  tptr->value = (pointer)values;

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
    for (i = tuple->count; i--; ) {
      typed_pointer_destroy(values[i]);
    }
    memory_deallocate((data_pointer*)&tuple->value);
    tptr->type = t_UNDEF;
    tptr->count = 0;
  }
}

/******************************************************************************/

result tuple_extend
(
  typed_pointer *tuple,
  uint32 count
)
{
  TRY();
  typed_pointer *new_values, *old_values;
  uint32 i;

  CHECK_POINTER(tuple);
  CHECK_PARAM(tuple->type == t_TUPLE);
  CHECK_PARAM(count > tuple->count);

  old_values = (typed_pointer*)tuple->value;
  CHECK(memory_allocate((data_pointer*)&new_values, count, sizeof(typed_pointer)));
  for (i = tuple->count; i--; ) {
    new_values[i] = old_values[i];
  }
  memory_deallocate((data_pointer*)&tuple->value);
  tuple->value = (pointer)new_values;
  tuple->count = count;

  FINALLY();
  RETURN();
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

/* end of file                                                                */
/******************************************************************************/
