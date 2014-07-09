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
string attribute_clone_name = "attribute_clone";
string attribute_list_alloc_name = "attribute_list_alloc";
string attribute_list_create_name = "attribute_list_create";
string attribute_list_nullify_name = "attribute_list_nullify";
string attribute_list_clone_name = "attribute_list_clone";
string attribute_add_name = "attribute_add";
string attribute_find_name = "attribute_find";
string attribute_stat_create_name = "attribute_stat_create";
string attribute_stat_combine_name = "attribute_stat_combine";
string attribute_stat_sum_name = "attribute_stat_sum";

/******************************************************************************/

/* this is for non-trivial functions that are not implemented yet */
result cloning_not_implemented
(
  attribute_list *target_list,
  typed_pointer *target,
  typed_pointer *source
)
{
  (void)target_list;
  (void)target;
  (void)source;
  return NOT_IMPLEMENTED;
}

/* does nothing but returns success */
result cloning_none
(
  attribute_list *target_list,
  typed_pointer *target,
  typed_pointer *source
)
{
  (void)target_list;
  (void)target;
  (void)source;
  return SUCCESS;
}

/* for most types, it is enough to copy the typed pointer */
result cloning_default
(
  attribute_list *target_list,
  typed_pointer *target,
  typed_pointer *source
)
{
  (void)target_list;
  /*printf("clone default\n");*/
  return typed_pointer_copy(target, source);
}

/* with sets, need to clone also the set attribute */
result cloning_disjoint_set
(
  attribute_list *target_list,
  typed_pointer *target,
  typed_pointer *source
)
{
  (void)target_list;
  (void)target;
  (void)source;
  return NOT_IMPLEMENTED;
}

/* with attribute statistics, need to clone also the attribute dependency */
/* NOTE: target_list needs to be the list containing the dependency */
result cloning_attribute_stat
(
  attribute_list *target_list,
  typed_pointer *target,
  typed_pointer *source
)
{
  attribute *dependency;
  dependency = ((attribute_stat*)source->value)->parent;
  if (dependency != NULL) {
    dependency = attribute_find(target_list, dependency->key);
    if (dependency != NULL) {
      attribute_stat new_attr_stat;
      new_attr_stat.parent = dependency;
      new_attr_stat.acc = NULL;
      return typed_pointer_create(target, t_attribute_stat, 1, 0,
                                  (pointer)&new_attr_stat);
    }
    else {
      return NOT_FOUND;
    }
  }
  else {
    return BAD_PARAM;
  }
}

attribute_cloning_function cloning_functions[] = {
  &cloning_none,             /* t_UNDEF */
  /* basic types */
  &cloning_default,          /* t_type */
  &cloning_default,          /* t_truth_value */
  &cloning_default,          /* t_pointer */
  &cloning_default,          /* t_typed_pointer */
  &cloning_not_implemented,  /* t_string */
  &cloning_default,          /* t_S8 */
  &cloning_default,          /* t_U8 */
  &cloning_default,          /* t_S16 */
  &cloning_default,          /* t_U16 */
  &cloning_default,          /* t_S32 */
  &cloning_default,          /* t_U32 */
  /*
  t_S64,
  t_U64,
  */
  &cloning_default,          /* t_F32 */
  &cloning_default,          /* t_F64 */
  &cloning_default,          /* t_pixel_value */
  &cloning_not_implemented,  /* t_tuple */
  &cloning_not_implemented,  /* t_list */
  &cloning_disjoint_set,     /* t_disjoint_set */
  &cloning_not_implemented,  /* t_graph */
  &cloning_not_implemented,  /* t_node */
  &cloning_not_implemented,  /* t_attribute */
  &cloning_not_implemented,  /* t_attribute_list */
  &cloning_attribute_stat,   /* t_attribute_stat */
  &cloning_not_implemented,  /* t_link */
  &cloning_not_implemented,  /* t_link_head */
  /* tree annotation types */
  &cloning_not_implemented,  /* t_statistics */
  &cloning_not_implemented,  /* t_raw_moments */
  &cloning_not_implemented,  /* t_accumulated_stat */
  &cloning_not_implemented,  /* t_neighborhood_stat */
  &cloning_not_implemented,  /* t_edge_response */
  &cloning_not_implemented,  /* t_link_measure */
  &cloning_not_implemented,  /* t_edge_profile */
  &cloning_not_implemented,  /* t_edge_links */
  &cloning_not_implemented,  /* t_boundary_message */
  &cloning_not_implemented,  /* t_boundary */
  &cloning_not_implemented,  /* t_segment_message */
  &cloning_not_implemented,  /* t_segment */
  /* parsing context types */
  &cloning_not_implemented,  /* t_stat_accumulator */
  &cloning_not_implemented,  /* t_pixel_image */
};

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

result attribute_clone
(
  attribute_list *target_list,
  attribute *target,
  attribute *source
)
{
  TRY();

  CHECK_POINTER(target_list);
  CHECK_POINTER(target);
  CHECK_POINTER(source);

  target->key = source->key;
  /*printf("clone i\n");*/
  (cloning_functions[source->value.type])(target_list, &target->value,
                                          &source->value);
  /*printf("clone o\n");*/
  FINALLY(attribute_clone);
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
    uint32 i;

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

result attribute_list_clone
(
  attribute_list *target,
  attribute_list *source
)
{
  TRY();
  uint32 i;

  CHECK_POINTER(target);
  CHECK_POINTER(source);

  attribute_list_destroy(target);
  CHECK(attribute_list_create(target, source->size));

  /* add all the same attributes as in source */
  /* NOTE: within attribute_add, attribute_clone is applied */
  /* this will ensure that the attributes remain valid */
  for (i = 0; i < source->count; i++) {
    CHECK(attribute_add(target, &source->items[i], NULL));
  }

  FINALLY(attribute_list_clone);
  RETURN();
}

/******************************************************************************/
/* need to apply some cloning operation? */
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

  new_attr = attribute_find(target, source->key);
  if (new_attr == NULL) {
    if (target->count < target->size) {
      new_attr = &target->items[target->count];
      attribute_nullify(new_attr);
      /* TODO: should use a clone operation instead of create? */
      /*CHECK(attribute_create(new_attr, source->key, &source->value));*/
      CHECK(attribute_clone(target, new_attr, source));
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
  if (added != NULL) {
    *added = new_attr;
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
  uint32 i;
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

/******************************************************************************/

pixel_value *value_attribute_add
(
  attribute_list *target,
  uint32 key,
  uint32 offset
)
{
  result r;
  attribute new_attr, *attr_ptr;
  r = SUCCESS;
  new_attr.key = key;
  /* TODO: implement? needed? */
  (void)target;
  (void)offset;
  (void)new_attr;
  (void)attr_ptr;
  return NULL;
}

/******************************************************************************/

void attribute_stat_init
(
  attribute_stat *target,
  attribute *parent
)
{
  if (target != NULL) {
    target->parent = parent;
    if (target->acc != NULL) {
      memory_deallocate((data_pointer*)&target->acc);
    }
  }
}

/******************************************************************************/

result attribute_stat_create
(
  attribute_stat *target
)
{
  TRY();
  
  CHECK_POINTER(target);
  
  if (target->acc == NULL) {
    CHECK(memory_allocate((data_pointer*)&target->acc, 1,
                          sizeof(attribute_stat_acc)));
  }
  
  if (target->parent != NULL) {
    real value = typed_pointer_cast_from(&target->parent->value);
    attribute_stat_acc_init(target->acc, value);
  }
  else {
    attribute_stat_acc_nullify(target->acc);
  }
  
  FINALLY(attribute_stat_create);
  RETURN();
}

/******************************************************************************/

void attribute_stat_destroy
(
  attribute_stat *target
)
{
  if (target != NULL) {
    memory_deallocate((data_pointer*)&target->acc);
    target->parent = NULL;
  }
}

/******************************************************************************/

void attribute_stat_acc_nullify
(
  attribute_stat_acc *target
)
{
  if (target != NULL) {
    memory_clear((data_pointer)target, 1, sizeof(attribute_stat_acc));
    /* TODO: should put this choice behind a flag and run some tests to */
    /* determine whether it is something that matters or not */
    /*
    target->n = 0;
    target->sval1 = 0;
    target->sval2 = 0;
    target->mean = 0;
    target->variance = 0;
    target->deviation = 0;
    */
  }
}

/******************************************************************************/

void attribute_stat_acc_init
(
  attribute_stat_acc *target,
  real value
)
{
  if (target != NULL) {
    target->n = 1;
    target->sval1 = value;
    target->sval2 = value*value;
    target->mean = value;
    target->variance = 0;
    target->deviation = 0;
  }
}

/******************************************************************************/

void attribute_stat_get
(
  attribute_stat *source,
  attribute_stat_acc *target
)
{
  if (source != NULL && target != NULL) {
    /* accumulator exists: copy the values contained in the acc */
    if (source->acc != NULL) {
      memory_copy((data_pointer)target, (data_pointer)source->acc, 1,
                  sizeof(attribute_stat_acc));
    }
    else
    /* default case: acc initialized with values based on one node's value */
    if (source->parent != NULL) {
      real value = typed_pointer_cast_from(&source->parent->value);
      attribute_stat_acc_init(target, value);
    }
    else {
      attribute_stat_acc_nullify(target);
    }
  }
}

/******************************************************************************/

void attribute_stat_combine
(
  attribute_stat *target,
  attribute_stat *source
)
{
  TRY();
  attribute_stat_acc *target_acc, source_acc;
  real n, s1, s2, m, v;
  
  CHECK_POINTER(target);
  CHECK_POINTER(source);
  
  if (target->acc == NULL) {
    CHECK(attribute_stat_create(target));
  }
  target_acc = target->acc;
  attribute_stat_get(source, &source_acc);
  
  n  = target_acc->n     += source_acc.n;
  s1 = target_acc->sval1 += source_acc.sval1;
  s2 = target_acc->sval2 += source_acc.sval2;
  m  = target_acc->mean   = s1 / n;
  
  v  = s2 / n - m*m;
  v  = v < 0 ? 0 : v;
  
  target_acc->variance = v;
  target_acc->deviation = sqrt(v);
  
  /* the source node will be reverted to the default state */
  if (source->acc != NULL) {
    memory_deallocate((data_pointer*)&source->acc);
    source->acc = NULL;
  }
  
  FINALLY(attribute_stat_combine);
  return;
}

/******************************************************************************/

void attribute_stat_sum
(
  attribute_stat *a,
  attribute_stat *b,
  attribute_stat *c
)
{
  TRY();
  attribute_stat_acc acc_a, acc_b, *acc_c;
  real n, s1, s2, m, v;
  
  CHECK_POINTER(a);
  CHECK_POINTER(b);
  CHECK_POINTER(c);
  
  attribute_stat_get(a, &acc_a);
  attribute_stat_get(b, &acc_b);
  if (c->acc == NULL) {
    CHECK(attribute_stat_create(c));
  }
  acc_c = c->acc;
  
  n  = acc_c->n     = acc_a.n     + acc_b.n;
  s1 = acc_c->sval1 = acc_a.sval1 + acc_b.sval1;
  s2 = acc_c->sval2 = acc_a.sval2 + acc_b.sval2;
  m  = acc_c->mean = s1 / n;
  
  v = s2 / n - m*m;
  v = v < 0 ? 0 : v;
  
  acc_c->variance  = v;
  acc_c->deviation = sqrt(v);
  
  FINALLY(attribute_stat_sum);
  return;
}

/* end of file                                                                */
/******************************************************************************/
