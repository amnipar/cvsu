/**
 * @file cvsu_set.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief A generic disjoint set structure.
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
#include "cvsu_set.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string disjoint_set_alloc_name = "disjoint_set_alloc";
string disjoint_set_free_name = "disjoint_set_free";

/******************************************************************************/

disjoint_set *disjoint_set_alloc
()
{
  TRY();
  disjoint_set *ptr;

  CHECK(memory_allocate((data_pointer*)&ptr, 1, sizeof(disjoint_set)));
  disjoint_set_nullify(ptr);

  FINALLY(disjoint_set_alloc);
  return ptr;
}

/******************************************************************************/

void disjoint_set_free
(
  disjoint_set *ptr
)
{
  TRY();

  r = SUCCESS;
  if (ptr != NULL) {
    CHECK(memory_deallocate((data_pointer*)&ptr));
  }

  FINALLY(disjoint_set_free);
}

/******************************************************************************/

void disjoint_set_nullify
(
  disjoint_set *target
)
{
  if (target != NULL) {
    target->parent = NULL;
    target->id = 0;
    target->rank = 0;
  }
}

/******************************************************************************/

truth_value disjoint_set_is_null
(
  disjoint_set *target
)
{
  if (target != NULL) {
    if (target->parent == NULL) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

void disjoint_set_create
(
  disjoint_set *target,
  uint32 id
)
{
  if (target != NULL) {
    target->parent = target;
    target->id = id;
    target->rank = 0;
  }
}

/******************************************************************************/

disjoint_set *disjoint_set_union
(
  disjoint_set *a,
  disjoint_set *b
)
{
  if (a != NULL && b != NULL) {
    a = disjoint_set_find(a);
    b = disjoint_set_find(b);
    if (a == b) {
      return;
    }
    else {
      if (a->rank < b->rank) {
        a->parent = b;
      }
      else
      if (a->rank > b->rank) {
        b->parent = a;
      }
      else {
        b->parent = a;
        a->rank += 1;
      }
    }
  }
}

/******************************************************************************/

disjoint_set *disjoint_set_find
(
  disjoint_set *target
)
{
  if (target != NULL) {
    if (target->parent != NULL && target->parent != target) {
      target->parent = disjoint_set_find(target->parent);
    }
    return target->parent;
  }
  return NULL;
}

/* end of file                                                                */
/******************************************************************************/
