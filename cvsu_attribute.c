/**
 * @file cvsu_attribute.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief A generic attribute structure.
 *
 * Copyright (c) 2014, Matti Johannes Eskelinen
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

#include "cvsu_config.h"
#include "cvsu_macros.h"
#include "cvsu_memory.h"
#include "cvsu_attribute.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string attribute_alloc_name = "attribute_alloc";
string attribute_create_name = "attribute_create";
string attribute_list_alloc_name = "attribute_list_alloc";
string attribute_list_create_name = "attribute_list_create";
string attribute_list_nullify_name = "attribute_list_nullify";
string attribute_add_name = "attribute_add";
string attribute_find_name = "attribute_find";

/******************************************************************************/

attribute *attribute_alloc
()
{
  TRY();
  attribute *ptr;

  CHECK(memory_allocate((data_pointer*)&ptr, 1, sizeof(attribute)));
  attribute_nullify(ptr);

  FINALLY(attribute_alloc);
  return ptr;
}

/******************************************************************************/

void attribute_free
(
  attribute *ptr
)
{
  if (ptr != NULL) {
    attribute_destroy(ptr);
    memory_deallocate((data_pointer*)&ptr);
  }
}

/******************************************************************************/

result attribute_create
(
  attribute *target,
  uint32 key,
  typed_pointer *value
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(value);
  CHECK_PARAM(key > 0);

  /*attribute_destroy(target);*/

  if (value->type == t_tuple) {
    ERROR(NOT_IMPLEMENTED);
  }
  else {
    CHECK(typed_pointer_copy(&target->value, value));
  }

  target->key = key;

  FINALLY(attribute_create);
  RETURN();
}

/******************************************************************************/

void attribute_destroy
(
  attribute *target
)
{
  if (target != NULL) {
    typed_pointer_destroy(&target->value);
    attribute_nullify(target);
  }
}

/******************************************************************************/

void attribute_nullify
(
  attribute *target
)
{
  if (target != NULL) {
    target->key = 0;
    typed_pointer_nullify(&target->value);
  }
}

/******************************************************************************/

truth_value attribute_is_null
(
  attribute *target
)
{
  if (target != NULL) {
    if (target->key == 0) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

attribute_list *attribute_list_alloc
()
{
  TRY();
  attribute_list *ptr;

  CHECK(memory_allocate((data_pointer*)&ptr, 1, sizeof(attribute_list)));
  attribute_list_nullify(ptr);

  FINALLY(attribute_list_alloc);
  return ptr;
}

/******************************************************************************/

void attribute_list_free
(
  attribute_list *ptr
)
{
  if (ptr != NULL) {
    attribute_list_destroy(ptr);
    memory_deallocate((data_pointer*)&ptr);
  }
}

/******************************************************************************/

result attribute_list_create
(
  attribute_list *target,
  uint32 size
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_TRUE(attribute_list_is_null(target));
  CHECK_PARAM(size > 0);

  /* reserve one extra slot at the end for extending the list */
  CHECK(memory_allocate((data_pointer*)&target->items, size+1,
                        sizeof(attribute)));
  CHECK(memory_clear((data_pointer)target->items, size+1, sizeof(attribute)));
  target->size = size;
  target->count = 0;

  FINALLY(attribute_list_create);
  RETURN();
}

/******************************************************************************/

void attribute_list_destroy
(
  attribute_list *target
)
{
  if (target != NULL && target->size > 0) {
    int i;

    for (i = 0; i < target->count; i++) {
      attribute_destroy(&target->items[i]);
    }
    /* TODO: need a destroy functionality for structured attributes */
    if (target->items[target->size].key > 0) {
      printf("not implemented functionality used in attribute_list_destroy");
    }
    memory_deallocate((data_pointer*)&target->items);
    attribute_list_nullify(target);
  }
}

/******************************************************************************/

void attribute_list_nullify
(
  attribute_list *target
)
{
  if (target != NULL) {
    target->items = NULL;
    target->size = 0;
    target->count = 0;
  }
}

/******************************************************************************/

truth_value attribute_list_is_null
(
  attribute_list *target
)
{
  if (target != NULL) {
    if (target->items == NULL) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

result attribute_add
(
  attribute_list *target,
  attribute *source,
  attribute **added
)
{
  TRY();
  attribute *new_attr;

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_POINTER(added);

  *added = NULL;
  new_attr = attribute_find(target, source->key);
  if (new_attr != NULL) {
    *added = new_attr;
  }
  else {
    if (target->count < target->size) {
      new_attr = &target->items[target->count];
      attribute_nullify(new_attr);
      CHECK(attribute_create(new_attr, source->key, &source->value));
      target->count++;
      /*
      new_attr->key = source->key;
      CHECK(typed_pointer_copy(&new_attr->value, &source->value));
      */
      *added = new_attr;
    }
    else {
      ERROR(NOT_IMPLEMENTED);
    }
  }

  FINALLY(attribute_add);
  RETURN();
}

/******************************************************************************/

attribute *attribute_find
(
  attribute_list *source,
  uint32 key
)
{
  int i;
  attribute *ptr;
  ptr = NULL;
  if (source != NULL) {
    for (i = 0; i < source->count; i++) {
      if (source->items[i].key == key) {
        ptr = &source->items[i];
      }
    }
  }
  return ptr;
}

/* end of file                                                                */
/******************************************************************************/
