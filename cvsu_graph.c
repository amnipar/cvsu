/**
 * @file cvsu_graph.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief A generic attributed graph structure.
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
#include "cvsu_graph.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string attribute_list_alloc_name = "attribute_list_alloc";
string attribute_list_free_name = "attribute_list_free";
string attribute_list_create_name = "attribute_list_create";
string attribute_list_destroy_name = "attribute_list_destroy";
string attribute_list_nullify_name = "attribute_list_nullify";
string attribute_add_name = "attribute_add";
string attribute_find_name = "attribute_find";

/******************************************************************************/

attribute_list *attribute_list_alloc
()
{
  TRY();
  attribute_list *ptr;

  CHECK(memory_allocate((data_pointer *)&ptr, 1, sizeof(attribute_list)));
  CHECK(attribute_list_nullify(ptr));

  FINALLY(attribute_list_alloc);
  return ptr;
}

/******************************************************************************/

void attribute_list_free
(
  attribute_list *ptr
)
{
  TRY();

  r = SUCCESS;
  if (ptr != NULL) {
    CHECK(attribute_list_destroy(ptr));
    CHECK(memory_deallocate((data_pointer *)&ptr));
  }

  FINALLY(attribute_list_free);
}

/******************************************************************************/

void attribute_list_create
(
  attribute_list *target,
  uint32 count
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_TRUE(attribute_list_is_null(target));
  CHECK_PARAM(count > 0);

  CHECK(memory_allocate((data_pointer *)&target->items, count+1,
                        sizeof(attribute)));
  target->count = count;

  FINALLY(attribute_list_create);
  RETURN();
}

/******************************************************************************/

void attribute_list_destroy
(
  attribute_list *target
)
{
  TRY();

  CHECK_POINTER(target);

  CHECK(memory_deallocate((data_pointer*)&target->items));
  CHECK(attribute_list_nullify(target));

  FINALLY(attribute_list_destroy);
  RETURN();
}

/******************************************************************************/

result attribute_list_nullify
(
  attribute_list *target
)
{
  TRY();

  CHECK_POINTER(target);

  target->items = NULL;
  target->count = 0;

  FINALLY(attribute_list_nullify);
  RETURN();
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
  uint32 key,
  type_label type,
  void *value
)
{
  TRY();

  FINALLY(attribute_add);
  RETURN();
}

/******************************************************************************/

result attribute_find
(
  attribute_list *source,
  attribute **target,
  uint32 key
)
{
  TRY();

  FINALLY(attribute_find);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
