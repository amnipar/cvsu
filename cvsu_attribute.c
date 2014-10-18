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
string attribute_to_real_name = "attribute_to_real";
string attribute_add_dependencies_name = "attribute_add_dependencies";
string attribute_update_name = "attribute_update";
string attribute_list_alloc_name = "attribute_list_alloc";
string attribute_list_create_name = "attribute_list_create";
string attribute_list_nullify_name = "attribute_list_nullify";
string attribute_list_clone_name = "attribute_list_clone";
string attribute_add_name = "attribute_add";
string attribute_list_add_new_name = "attribute_list_add_new";

string pixel_value_attribute_add_name = "pixel_value_attribute_add";
string position_2d_attribute_add_name = "position_2d_attribute_add";
string scalar_attribute_add_name = "scalar_attribute_add";
string pointer_attribute_add_name = "pointer_attribute_add";

string attribute_stat_init_name = "attribute_stat_init";
string attribute_stat_create_name = "attribute_stat_create";
string attribute_stat_combine_name = "attribute_stat_combine";
string attribute_stat_sum_name = "attribute_stat_sum";
string attribute_stat_attribute_add_name = "attribute_stat_attribute_add";

string attribute_2d_pos_init_name = "attribute_2d_pos_init";
string attribute_2d_pos_create_name = "attribute_2d_pos_create";
string attribute_2d_pos_combine_name = "attribute_2d_pos_combine";
string attribute_2d_pos_sum_name = "attribute_2d_pos_sum";
string attribute_2d_pos_attribute_add_name = "attribute_2d_pos_attribute_add";

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
  dependency = ((attribute_stat*)source->value)->dependency;
  if (dependency != NULL) {
    dependency = attribute_find(target_list, dependency->key);
    if (dependency != NULL) {
      attribute_stat new_attr_stat;
      new_attr_stat.dependency = dependency;
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
  &cloning_not_implemented   /* t_node_ref */
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
  target->dependencies = NULL;

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
    if (target->dependencies != NULL) {
      memory_deallocate((data_pointer*)&target->dependencies->attributes);
      memory_deallocate((data_pointer*)&target->dependencies);
    }
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
    target->dependencies = NULL;
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

real attribute_to_real
(
  attribute *target
)
{
  TRY();
  
  CHECK_POINTER(target);
  
  FINALLY(attribute_to_real);
  return typed_pointer_cast_from(&target->value);
}

/******************************************************************************/

result attribute_add_dependencies
(
  attribute *target,
  uint32 length,
  attribute_evaluator eval
)
{
  TRY();
  attribute_dependency *dep;
  
  CHECK_POINTER(target);
  
  CHECK(memory_allocate((data_pointer*)&dep, 1, sizeof(attribute_dependency)));
  CHECK(memory_allocate((data_pointer*)&dep->attributes, length, 
                        sizeof(attribute*)));
  
  dep->length = length;
  dep->eval = eval;
  target->dependencies = dep;
  
  FINALLY(attribute_add_dependencies);
  RETURN();
}

/******************************************************************************/

result attribute_update
(
  attribute *target,
  uint32 token
)
{
  TRY();
  
  CHECK_POINTER(target);
  
  if (target->dependencies != NULL) {
    CHECK(target->dependencies->eval(target,
                                     target->dependencies->attributes,
                                     target->dependencies->length,
                                     token));
  }
  
  FINALLY(attribute_update);
  RETURN();
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

result attribute_list_add_new
(
  attribute_list *target,
  uint32 key,
  type_label type,
  attribute **added
)
{
  TRY();
  attribute *new_attr;
  
  CHECK_POINTER(target);
  CHECK_POINTER(added);
  
  new_attr = attribute_find(target, key);
  if (new_attr != NULL) {
    if (new_attr->value.type != type) {
      ERROR(BAD_TYPE);
    }
  }
  else {
    if (target->count < target->size) {
      new_attr = &target->items[target->count];
      attribute_nullify(new_attr);
      CHECK(typed_pointer_create(&new_attr->value, type, 1, 1, NULL));
      new_attr->key = key;
      target->count++;
    }
    else {
      ERROR(NOT_IMPLEMENTED);
    }
  }
  *added = new_attr;
  
  FINALLY(attribute_list_add_new);
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

attribute *attribute_find_by_type
(
  attribute_list *source,
  type_label type
)
{
  uint32 i;
  attribute *ptr;
  ptr = NULL;
  if (source != NULL) {
    for (i = 0; i < source->count; i++) {
      if (source->items[i].value.type == type) {
        ptr = &source->items[i];
      }
    }
  }
  return ptr;
}

/******************************************************************************/

result pixel_value_attribute_add
(
  attribute_list *target,
  uint32 key,
  uint32 offset,
  uint32 token,
  pixel_value **added
)
{
  TRY();
  attribute *new_attr;
  pixel_value *new_value;
  
  CHECK_POINTER(target);
  
  CHECK(attribute_list_add_new(target, key, t_pixel_value, &new_attr));
  new_value = (pixel_value*)new_attr->value.value;
  new_value->offset = offset;
  new_value->token = token;
  new_value->cache = 0;
  
  if (added != NULL) {
    *added = new_value;
  }
  
  FINALLY(pixel_value_attribute_add);
  RETURN();
}

/******************************************************************************/

pixel_value *pixel_value_attribute_get
(
  attribute_list *target,
  uint32 key
)
{
  attribute *attr = attribute_find(target, key);
  if (attr != NULL && attr->value.type == t_pixel_value) {
    return (pixel_value*)attr->value.value;
  }
  return NULL;
}

/******************************************************************************/

result position_2d_attribute_add
(
  attribute_list *target,
  uint32 key,
  real x,
  real y,
  position_2d **added
)
{
  TRY();
  attribute *new_attr;
  position_2d *new_pos;
  
  CHECK_POINTER(target);
  
  CHECK(attribute_list_add_new(target, key, t_position_2d, &new_attr));
  new_pos = (position_2d*)new_attr->value.value;
  new_pos->x = x;
  new_pos->y = y;
  
  if (added != NULL) {
    *added = new_pos;
  }
  
  FINALLY(position_2d_attribute_add);
  RETURN();
}

/******************************************************************************/

position_2d *position_2d_attribute_get
(
  attribute_list *target,
  uint32 key
)
{
  attribute *attr = attribute_find(target, key);
  if (attr != NULL && attr->value.type == t_position_2d) {
    return (position_2d*)attr->value.value;
  }
  return NULL;
}

/******************************************************************************/

result scalar_attribute_add
(
  attribute_list *target,
  uint32 key,
  real value,
  real **added
)
{
  TRY();
  attribute *new_attr;
  real *new_scalar;
  
  CHECK_POINTER(target);
  
  CHECK(attribute_list_add_new(target, key, t_real, &new_attr));
  new_scalar = (real*)new_attr->value.value;
  *new_scalar = value;
  
  if (added != NULL) {
    *added = new_scalar;
  }
    
  FINALLY(scalar_attribute_add);
  RETURN();
}

/******************************************************************************/

real *scalar_attribute_get
(
  attribute_list *target,
  uint32 key
)
{
  attribute *attr = attribute_find(target, key);
  if (attr != NULL && attr->value.type == t_real) {
    return (real*)attr->value.value;
  }
  return NULL;
}

/******************************************************************************/

result pointer_attribute_add
(
  attribute_list *target,
  uint32 key,
  pointer ptr,
  pointer **added
)
{
  TRY();
  attribute *new_attr;
  pointer *new_ptr;
  
  CHECK_POINTER(target);
  
  CHECK(attribute_list_add_new(target, key, t_pointer, &new_attr));
  new_ptr = (pointer*)new_attr->value.value;
  *new_ptr = ptr;
  
  if (added != NULL) {
    *added = new_ptr;
  }
  
  FINALLY(pointer_attribute_add);
  RETURN();
}

/******************************************************************************/

pointer *pointer_attribute_get
(
  attribute_list *target,
  uint32 key
)
{
  attribute *attr = attribute_find(target, key);
  if (attr != NULL && attr->value.type == t_pointer) {
    return (pointer*)attr->value.value;
  }
  return NULL;
}

/******************************************************************************/

result attribute_stat_init
(
  attribute_stat *target,
  attribute *dependency
)
{
  TRY();
  
  CHECK_POINTER(target);
  CHECK_POINTER(dependency);
  
  if (dependency->value.type == t_real) {
    real *value = (real*)dependency->value.value;
    target->value = value;
  }
  else
  if (dependency->value.type == t_pixel_value) {
    pixel_value *value = (real*)dependency->value.value;
    target->value = &value->cache;
  }
  else {
    ERROR(BAD_TYPE);
  }
  
  target->dependency = dependency;
  if (target->acc != NULL) {
    memory_deallocate((data_pointer*)&target->acc);
  }
  
  FINALLY(attribute_stat_init);
  RETURN();
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
  
  if (target->dependency != NULL) {
    attribute_stat_acc_init(target->acc, *target->value);
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
    target->dependency = NULL;
    target->value = NULL;
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
    if (source->value != NULL) {
      attribute_stat_acc_init(target, *source->value);
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

/******************************************************************************/

result attribute_stat_attribute_add
(
  attribute_list *target,
  uint32 key,
  attribute *dependency,
  attribute_stat **added
)
{
  TRY();
  attribute *new_attr;
  attribute_stat *new_stat;
  
  CHECK_POINTER(target);
  
  CHECK(attribute_list_add_new(target, key, t_attribute_stat, &new_attr));
  new_stat = (attribute_stat*)new_attr->value.value;
  CHECK(attribute_stat_init(new_stat, dependency));
  
  if (added != NULL) {
    *added = new_stat;
  }
  
  FINALLY(attribute_stat_attribute_add);
  RETURN();
}

/******************************************************************************/

attribute_stat *attribute_stat_attribute_get
(
  attribute_list *target,
  uint32 key
)
{
  attribute *attr = attribute_find(target, key);
  if (attr != NULL && attr->value.type == t_attribute_stat) {
    return (attribute_stat*)attr->value.value;
  }
  return NULL;
}

/******************************************************************************/

result attribute_2d_pos_init
(
  attribute_2d_pos *target,
  attribute *dependency
)
{
  TRY();
  
  CHECK_POINTER(target);
  CHECK_POINTER(dependency);
  CHECK_PARAM(dependency->value.type == t_position_2d);
  
  target->dependency = dependency;
  target->pos = (position_2d*)dependency->value.value;
  if (target->acc != NULL) {
    memory_deallocate((data_pointer*)&target->dependency);
  }
  
  FINALLY(attribute_2d_pos_init);
  RETURN();
}

/******************************************************************************/

result attribute_2d_pos_create
(
  attribute_2d_pos *target
)
{
  TRY();
  
  CHECK_POINTER(target);
  
  if (target->acc == NULL) {
    CHECK(memory_allocate((data_pointer*)&target->acc, 1,
                          sizeof(attribute_2d_pos_acc)));
  }
  
  if (target->pos != NULL) {
    attribute_2d_pos_acc_init(target->acc, target->pos);
  }
  else {
    attribute_2d_pos_acc_nullify(target->acc);
  }
  
  FINALLY(attribute_2d_pos_create);
  RETURN();
}

/******************************************************************************/

void attribute_2d_pos_destroy
(
  attribute_2d_pos *target
)
{
  if (target != NULL) {
    memory_deallocate((data_pointer*)&target->acc);
    target->dependency = NULL;
    target->pos = NULL;
  }
}

/******************************************************************************/

void attribute_2d_pos_acc_nullify
(
  attribute_2d_pos_acc *target
)
{
  if (target != NULL) {
    memory_clear((data_pointer)target, 1, sizeof(attribute_2d_pos_acc));
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

void attribute_2d_pos_acc_init
(
  attribute_2d_pos_acc *target,
  position_2d *pos
)
{
  if (target != NULL && pos != NULL) {
    target->n = 1;
    target->sx = pos->x;
    target->cx = pos->x;
    target->sy = pos->y;
    target->cy = pos->y;
  }
}

/******************************************************************************/

void attribute_2d_pos_get
(
  attribute_2d_pos *source,
  attribute_2d_pos_acc *target
)
{
  if (source != NULL && target != NULL) {
    /* accumulator exists: copy the values contained in the acc */
    if (source->acc != NULL) {
      memory_copy((data_pointer)target, (data_pointer)source->acc, 1,
                  sizeof(attribute_2d_pos_acc));
    }
    else
    /* default case: acc initialized with values based on one node's value */
    if (source->pos != NULL) {
      attribute_2d_pos_acc_init(target, source->pos);
    }
    else {
      attribute_2d_pos_acc_nullify(target);
    }
  }
}

/******************************************************************************/

void attribute_2d_pos_combine
(
  attribute_2d_pos *target,
  attribute_2d_pos *source
)
{
  TRY();
  attribute_2d_pos_acc *target_acc, source_acc;
  real n, sx, sy;
  
  CHECK_POINTER(target);
  CHECK_POINTER(source);
  
  if (target->acc == NULL) {
    CHECK(attribute_2d_pos_create(target));
  }
  target_acc = target->acc;
  attribute_2d_pos_get(source, &source_acc);
  
  n  = target_acc->n  += source_acc.n;
  sx = target_acc->sx += source_acc.sx;
  sy = target_acc->sy += source_acc.sy;
  
  target_acc->cx = sx / n;
  target_acc->cy = sy / n;
  
  /* the source node will be reverted to the default state */
  if (source->acc != NULL) {
    memory_deallocate((data_pointer*)&source->acc);
    source->acc = NULL;
  }
  
  FINALLY(attribute_2d_pos_combine);
  return;
}

/******************************************************************************/

void attribute_2d_pos_sum
(
  attribute_2d_pos *a,
  attribute_2d_pos *b,
  attribute_2d_pos *c
)
{
  TRY();
  attribute_2d_pos_acc acc_a, acc_b, *acc_c;
  real n, sx, sy;
  
  CHECK_POINTER(a);
  CHECK_POINTER(b);
  CHECK_POINTER(c);
  
  attribute_2d_pos_get(a, &acc_a);
  attribute_2d_pos_get(b, &acc_b);
  if (c->acc == NULL) {
    CHECK(attribute_2d_pos_create(c));
  }
  acc_c = c->acc;
  
  n  = acc_c->n  = acc_a.n  + acc_b.n;
  sx = acc_c->sx = acc_a.sx + acc_b.sx;
  sy = acc_c->sy = acc_a.sy + acc_b.sy;
  
  acc_c->cx = sx / n;
  acc_c->cy = sy / n;
  
  FINALLY(attribute_2d_pos_sum);
  return;
}

/******************************************************************************/

result attribute_2d_pos_attribute_add
(
  attribute_list *target,
  uint32 key,
  attribute *dependency,
  attribute_2d_pos **added
)
{
  TRY();
  attribute *new_attr;
  attribute_2d_pos *new_pos;
  
  CHECK_POINTER(target);
  
  CHECK(attribute_list_add_new(target, key, t_attribute_2d_pos, &new_attr));
  new_pos = (attribute_2d_pos*)new_attr->value.value;
  CHECK(attribute_2d_pos_init(new_pos, dependency));
  
  if (added != NULL) {
    *added = new_pos;
  }
  
  FINALLY(attribute_2d_pos_attribute_add);
  RETURN();
}

/******************************************************************************/

attribute_2d_pos *attribute_2d_pos_attribute_get
(
  attribute_list *target,
  uint32 key
)
{
  attribute *attr = attribute_find(target, key);
  if (attr != NULL && attr->value.type == t_attribute_2d_pos) {
    return (attribute_2d_pos*)attr->value.value;
  }
  return NULL;
}

/* end of file                                                                */
/******************************************************************************/
