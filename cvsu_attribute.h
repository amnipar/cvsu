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

/******************************************************************************/

typedef void (*attribute_cloning_function)
(
  attribute_list *target_list,
  typed_pointer *target,
  typed_pointer *source
);

/******************************************************************************/

typedef struct attribute_list_t {
  attribute *items;
  uint32 size;
  uint32 count;
} attribute_list;

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

result attribute_add
(
  attribute_list *target,
  attribute *source,
  attribute **added
);

attribute *attribute_find
(
  attribute_list *source,
  uint32 key
);

/******************************************************************************/

typedef struct attribute_stat_acc_t {
  real n;
  real sx;
  real sy;
  real sval1;
  real sval2;
  real cx;
  real cy;
  real mean;
  real variance;
  real deviation;
} attribute_stat_acc;

typedef struct attribute_stat_t
  attribute *parent;
  attribute_stat_acc *acc;
} attribute_stat;

/**
 * Initializes the attribute_stat structure into the default state, which means
 * the accumulator structure is NULL and the stats are considered as single
 * value read from the parent attribute.
 */
void attribute_stat_init
(
  attribute_stat *target,
  attribute *parent
);

/**
 * Creates the accumulator structure, replacing the default behavior of getting
 * parent attribute as a single value.
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

void attribute_stat_acc_nullify
(
  attribute_stat_acc *target
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
 * such that they reflect the sums.
 */
void attribute_stat_sum
(
  attribute_stat *a,
  attribute_stat *b,
  attribute_stat *c
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_ATTRIBUTE_H */
