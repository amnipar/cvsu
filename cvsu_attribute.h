/**
 * @file cvsu_attribute.h
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

#ifndef CVSU_ATTRIBUTE_H
#   define CVSU_ATTRIBUTE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_types.h"
#include "cvsu_typed_pointer.h"

/******************************************************************************/

typedef struct attribute_t {
  uint32 key;
  typed_pointer value;
  /** Attribute dependencies for updating the value. May be NULL. */
  struct attribute_dependency_t *dependencies;
} attribute;

attribute *attribute_alloc();

void attribute_free
(
  attribute *ptr
);

/**
 * Creates an attribute that has the given key and value.
 */
result attribute_create
(
  attribute *target,
  uint32 key,
  typed_pointer *value
);

void attribute_destroy
(
  attribute *target
);

void attribute_nullify
(
  attribute *target
);

truth_value attribute_is_null
(
  attribute *target
);

real attribute_to_real
(
  attribute *target
);

/**
 * Tracks the range of a scalar-valued attribute, identified by its key.
 */
typedef struct attribute_range_t {
  uint32 key;
  real min_value;
  real max_value;
  real range;
} attribute_range;

/**
 * Tracks the difference of the given attribute in two elements; most suitable
 * as a link attribute for tracking the difference between linked nodes. The
 * value is cached, so calculating the difference each time it is accessed is
 * not necessary.
 * TODO: would be possible to achieve with a scalar attribute with dependencies
 */
typedef struct attribute_diff_t {
  uint32 key;
  attribute *attr_a;
  attribute *attr_b;
  real cached;
} attribute_diff;

/**
 * A function for evaluating the new value for an attribute based on its list of
 * dependencies. The function should use CHECK macros for ensuring the types are
 * correct. These checks can be then turned off after verifying that the
 * application has been built correctly.
 * TODO: create a CHECK_TYPE macro (for tptr) and CHECK_ATTR_TYPE?
 */
typedef result (*attribute_evaluator)(attribute *target, attribute **dependencies, uint32 length, uint32 token);

/**
 * Defines the dependencies of an attribute by listing the depended attributes
 * and a function for evaluating the new value.
 */
typedef struct attribute_dependency_t {
  uint32 length;
  attribute **dependencies;
  attribute_evaluator eval;
} attribute_dependency;

/**
 * Updates an attribute by checking the token value; if the token value in the
 * attribute is different, the attribute value is outdated and must be evaluated
 * again. This is done by calling the dependency evaluation function, which in
 * turn checks the tokens on the dependencies and may update those that are not 
 * up to date.
 */
result attribute_update
(
  attribute *target,
  uint32 token
);

/******************************************************************************/

typedef struct attribute_list_t {
  attribute *items;
  uint32 size;
  uint32 count;
} attribute_list;

typedef result (*attribute_list_function)(attribute_list *target, pointer params);

attribute_list *attribute_list_alloc();

void attribute_list_free
(
  attribute_list *ptr
);

result attribute_list_create
(
  attribute_list *target,
  uint32 size
);

void attribute_list_destroy
(
  attribute_list *target
);

void attribute_list_nullify
(
  attribute_list *target
);

truth_value attribute_list_is_null
(
  attribute_list *target
);

result attribute_list_clone
(
  attribute_list *target,
  attribute_list *source
);

/**
 * Creates an attribute that has the same value as the given source attribute,
 * taking into account the attribute structure and dependencies. The attribute
 * list parameter is required for resolving the possible attribute dependencies.
 */
result attribute_clone
(
  attribute_list *target_list,
  attribute *target,
  attribute *source
);

result attribute_add
(
  attribute_list *target,
  attribute *source,
  attribute **added
);

result attribute_list_add_new
(
  attribute_list *target,
  uint32 key,
  type_label type,
  attribute **added
);

attribute *attribute_find
(
  attribute_list *source,
  uint32 key
);

attribute *attribute_find_by_type
(
  attribute_list *source,
  type_label type
);

result pixel_value_attribute_add
(
  attribute_list *target,
  uint32 key,
  uint32 offset,
  uint32 token,
  pixel_value **added
);

pixel_value *pixel_value_attribute_get
(
  attribute_list *target,
  uint32 key
);

result position_2d_attribute_add
(
  attribute_list *target,
  uint32 key,
  real x,
  real y,
  position_2d **added
);

position_2d *position_2d_attribute_get
(
  attribute_list *target,
  uint32 key
);

result scalar_attribute_add
(
  attribute_list *target,
  uint32 key,
  real value,
  real **added
);

real *scalar_attribute_get
(
  attribute_list *target,
  uint32 key
);

result pointer_attribute_add
(
  attribute_list *target,
  uint32 key,
  pointer ptr,
  pointer **added
);

pointer *pointer_attribute_get
(
  attribute_list *target,
  uint32 key
);

/******************************************************************************/

typedef result (*attribute_cloning_function)(attribute_list *target_list, typed_pointer *target, typed_pointer *source);

/******************************************************************************/

typedef struct attribute_stat_acc_t {
  real n;
  real sval1;
  real sval2;
  real mean;
  real variance;
  real deviation;
} attribute_stat_acc;

typedef struct attribute_stat_t {
  attribute *dependency;
  real *value;
  attribute_stat_acc *acc;
} attribute_stat;

/**
 * Initializes the attribute_stat structure into the default state, which means
 * the accumulator structure is NULL and the stats are considered as single
 * value read from the parent attribute.
 */
result attribute_stat_init
(
  attribute_stat *target,
  attribute *dependency
);

/**
 * Creates the accumulator structure, replacing the default behavior of getting
 * parent attribute as a single value. The accumulator is initialized with the
 * default values based on the parent attribute's value.
 */
result attribute_stat_create
(
  attribute_stat *target
);

/**
 * Destroys and nullifies the attribute_stat structure, frees the accumulator
 * structure if it is allocated.
 */ 
void attribute_stat_destroy
(
  attribute_stat *target
);

/**
 * Sets the accumulator to null value.
 */
void attribute_stat_acc_nullify
(
  attribute_stat_acc *target
);

/**
 * Initializes the accumulator based on single value.
 */
void attribute_stat_acc_init
(
  attribute_stat_acc *target,
  real value
);

/**
 * Copies the stat values into the given accumulator, taking into account
 * whether the structure is in the default state or not.
 */
void attribute_stat_get
(
  attribute_stat *source,
  attribute_stat_acc *target
);

/**
 * Combines two attribute_stat structures such, that the first one will end up
 * containing the combined statistics and the second one will be reverted to the
 * default state (accumulator is destroyed). This is typically used for
 * calculating the statistics of the union of two disjoint_sets of elements.
 */
void attribute_stat_combine
(
  attribute_stat *target,
  attribute_stat *source
);

/**
 * Calculates the sum of two attribute_stat structures a and b such, that their
 * sum is stored in the third structure, c. All the values of c will be updated
 * such that they reflect the sums. The implementation is done in such a way,
 * that it is safe to use either a or b also as c. If c is different from a and
 * b, then the a and b structures are not changed.
 */
void attribute_stat_sum
(
  attribute_stat *a,
  attribute_stat *b,
  attribute_stat *c
);

result attribute_stat_attribute_add
(
  attribute_list *target,
  uint32 key,
  attribute *dependency,
  attribute_stat **added
);

attribute_stat *attribute_stat_attribute_get
(
  attribute_list *target,
  uint32 key
);

/******************************************************************************/

typedef struct attribute_2d_pos_acc_t {
  real n;
  real sx;
  real sy;
  real cx;
  real cy;
} attribute_2d_pos_acc;

typedef struct attribute_2d_pos_t {
  attribute *dependency;
  position_2d *pos;
  attribute_2d_pos_acc *acc;
} attribute_2d_pos;

result attribute_2d_pos_init
(
  attribute_2d_pos *target,
  attribute *dependency
);

result attribute_2d_pos_create
(
  attribute_2d_pos *target
);

void attribute_2d_pos_destroy
(
  attribute_2d_pos *target
);

void attribute_2d_pos_acc_nullify
(
  attribute_2d_pos_acc *target
);

void attribute_2d_pos_acc_init
(
  attribute_2d_pos_acc *target,
  position_2d *pos
);

void attribute_2d_pos_get
(
  attribute_2d_pos *source,
  attribute_2d_pos_acc *target
);

void attribute_2d_pos_combine
(
  attribute_2d_pos *target,
  attribute_2d_pos *source
);

void attribute_2d_pos_sum
(
  attribute_2d_pos *a,
  attribute_2d_pos *b,
  attribute_2d_pos *c
);

result attribute_2d_pos_attribute_add
(
  attribute_list *target,
  uint32 key,
  attribute *dependency,
  attribute_2d_pos **added
);

attribute_2d_pos *attribute_2d_pos_attribute_get
(
  attribute_list *target,
  uint32 key
);

/******************************************************************************/

typedef struct attribute_moments_acc_2d_t {
  real m00;
  real m10;
  real m01;
  real m20;
  real m11;
  real m02;
  real cx;
  real cy;
  real r1;
  real r2;
  real a;
} attribute_moments_acc_2d;

/**
 * Maintains 2-dimensional shape moments based on positions of a set of nodes.
 */
typedef struct attribute_moments_2d_t {
  attribute *parent;
  attribute_moments_acc_2d *acc;
} attribute_moments_2d;

/**
 * Maintains n-dimensional shape moments based on positions of a set of nodes.
 */
/*
typedef struct attribute_moments_nd_t {
  attribute *parent;
  attribute_moments_acc_nd *acc;
} attribute_moments_nd;
*/
#ifdef __cplusplus
}
#endif

#endif /* CVSU_ATTRIBUTE_H */
