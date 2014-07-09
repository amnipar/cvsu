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
string disjoint_set_create_name = "disjoint_set_create";
string disjoint_set_attributes_create_name = "disjoint_set_attributes_create";

string disjoint_set_attr_create_name = "disjoint_set_attr_create";
string disjoint_set_create_with_stat_name = "disjoint_set_create_with_stat";
string disjoint_set_add_attr_name = "disjoint_set_add_attr";
string disjoint_set_add_stat_attr_name = "disjoint_set_add_stat_attr";

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
  if (ptr != NULL) {
    disjoint_set_destroy(ptr);
    memory_deallocate((data_pointer*)&ptr);
  }
}

/******************************************************************************/

void disjoint_set_nullify
(
  disjoint_set *target
)
{
  if (target != NULL) {
    target->id = NULL;
    target->rank = 0;
    target->size = 0;
    attribute_list_nullify(&target->attributes);
  }
}

/******************************************************************************/

truth_value disjoint_set_is_null
(
  disjoint_set *target
)
{
  if (target != NULL) {
    if (target->id == NULL) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

result disjoint_set_create
(
  disjoint_set *target,
  uint32 attribute_count
)
{
  TRY();

  CHECK_POINTER(target);

  disjoint_set_destroy(target);
  if (target != NULL) {
    target->id = target;
    target->rank = 0;
    target->size = 1;
    CHECK(disjoint_set_attributes_create(target, attribute_count));
  }

  FINALLY(disjoint_set_create);
  RETURN();
}

/******************************************************************************/

void disjoint_set_destroy
(
  disjoint_set *target
)
{
  if (target != NULL) {
    attribute_list_destroy(&target->attributes);
    disjoint_set_nullify(target);
  }
}

/******************************************************************************/

result disjoint_set_attributes_create
(
  disjoint_set *target,
  uint32 attribute_count
)
{
  TRY();
  
  CHECK_POINTER(target);
  
  if (attribute_count > 0) {
    CHECK(attribute_list_create(&target->attributes, attribute_count));
  }
  
  FINALLY(disjoint_set_attributes_create);
  RETURN();
}

/******************************************************************************/

void disjoint_set_attributes_clear
(
  disjoint_set *target
)
{
  if (target != NULL) {
    memory_clear((data_pointer)target->attributes.items, 
                 target->attributes.size+1, sizeof(attribute));
    target->attributes.count = 0;
  }
}

/******************************************************************************/

void union_S32(typed_pointer *a, typed_pointer *b)
{
  *((sint32*)a->value) = *((sint32*)a->value) + *((sint32*)b->value);
}

void union_U32(typed_pointer *a, typed_pointer *b)
{
  *((uint32*)a->value) = *((uint32*)a->value) + *((uint32*)b->value);
}

void union_F32(typed_pointer *a, typed_pointer *b)
{
  *((float*)a->value) = *((float*)a->value) + *((float*)b->value);
}

void union_F64(typed_pointer *a, typed_pointer *b)
{
  *((double*)a->value) = *((double*)a->value) + *((double*)b->value);
}

void union_statistics(typed_pointer *a, typed_pointer *b)
{
  statistics *astat, *bstat;
  astat = (statistics*)a->value;
  bstat = (statistics*)b->value;
  astat->N += bstat->N;
  astat->sum += bstat->sum;
  astat->sum2 += bstat->sum2;
}

void union_raw_moments(typed_pointer *a, typed_pointer *b)
{
  raw_moments *araw, *braw;
  araw = (raw_moments*)a->value;
  braw = (raw_moments*)b->value;
  araw->m00 += braw->m00;
  araw->m10 += braw->m10;
  araw->m01 += braw->m01;
  araw->m11 += braw->m11;
  araw->m20 += braw->m20;
  araw->m02 += braw->m02;
}

void attribute_union
(
  disjoint_set *a,
  disjoint_set *b
)
{
  uint32 acount, bcount, count, i;
  acount = a->attributes.count;
  bcount = b->attributes.count;
  if (acount > 0 && bcount > 0) {
    typed_pointer avalue, bvalue;
    count = acount < bcount ? acount : bcount;
    for (i = 0; i < count; i++) {
      if (a->attributes.items[i].key == b->attributes.items[i].key) {
        avalue = a->attributes.items[i].value;
        bvalue = b->attributes.items[i].value;
        if (avalue.type == bvalue.type) {
          switch (avalue.type) {
            case t_S32:
              union_S32(&avalue, &bvalue);
              break;
            case t_U32:
              union_U32(&avalue, &bvalue);
              break;
            case t_F32:
              union_F32(&avalue, &bvalue);
              break;
            case t_F64:
              union_F64(&avalue, &bvalue);
              break;
            case t_statistics:
              union_statistics(&avalue, &bvalue);
              break;
            case t_raw_moments:
              union_raw_moments(&avalue, &bvalue);
              break;
            default:
              /* should generate BAD_TYPE error or something? */
              break;
          }
        }
      }
    }
  }
}

disjoint_set *disjoint_set_union
(
  disjoint_set *a,
  disjoint_set *b
)
{
  if (a != NULL && b != NULL) {
    a = disjoint_set_find(a);
    b = disjoint_set_find(b);
    if (a != b) {
      if (a->rank < b->rank) {
        a->id = b;
        b->size += a->size;
        attribute_union(b, a);
        return b;
      }
      else
      if (a->rank > b->rank) {
        b->id = a;
        a->size += b->size;
        attribute_union(a, b);
        return a;
      }
      else {
        b->id = a;
        a->rank += 1;
        a->size += b->size;
        attribute_union(a, b);
        return a;
      }
    }
    else {
      return a;
    }
  }
  return NULL;
}

/******************************************************************************/

disjoint_set *disjoint_set_find
(
  disjoint_set *target
)
{
  if (target != NULL) {
    if (target->id != NULL && target->id != target) {
      target->id = disjoint_set_find(target->id);
    }
    return target->id;
  }
  return NULL;
}

/******************************************************************************/

uint32 disjoint_set_id
(
  disjoint_set *target
)
{
  return (uint32)disjoint_set_find(target);
}

/******************************************************************************/

uint32 disjoint_set_attrib_size
(
  disjoint_set *target
)
{
  if (target != NULL) {
    return target->attributes.size;
  }
  return 0;
}

/******************************************************************************/

result disjoint_set_attr_create
(
  attribute *target,
  uint32 attribute_count
)
{
  TRY();
  
  CHECK_POINTER(target);
  
  FINALLY(disjoint_set_attr_create);
  RETURN();
}

/******************************************************************************/

result disjoint_set_create_with_stat
(
  disjoint_set *target,
  uint32 attribute_count,
  uint32 stat_attr
)
{
  TRY();
  
  CHECK_POINTER(target);
  
  FINALLY(disjoint_set_create_with_stat);
  RETURN();
}

/******************************************************************************/

result disjoint_set_add_attr
(
  attribute_list *attrs,
  pointer params
)
{
  TRY();
  
  CHECK_POINTER(attrs);
  
  FINALLY(disjoint_set_add_attr);
  RETURN();
}

/******************************************************************************/

result disjoint_set_add_stat_attr
(
  attribute_list *attrs,
  pointer params
)
{
  TRY();
  
  CHECK_POINTER(attrs);
  
  FINALLY(disjoint_set_add_stat_attr);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
