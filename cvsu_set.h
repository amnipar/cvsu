/**
 * @file cvsu_set.h
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

#ifndef CVSU_SET_H
#   define CVSU_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_types.h"
#include "cvsu_attribute.h"

/**
 * Defines a disjoint set datatype, which is used to encode set membership by
 * ownership of a set attribute, and union operations are managed with a
 * union-find system.
 */
typedef struct disjoint_set_t {
  struct disjoint_set_t *id;
  uint32 rank;
  uint32 size;
  attribute_list attributes;
} disjoint_set;

disjoint_set *disjoint_set_alloc();

void disjoint_set_free
(
  disjoint_set *ptr
);

void disjoint_set_nullify
(
  disjoint_set *target
);

truth_value disjoint_set_is_null
(
  disjoint_set *target
);

/**
 * Create a set with size 1, itself its own representative, with the given
 * amount of slots for attributes.
 */
result disjoint_set_create
(
  disjoint_set *target,
  uint32 attribute_count
);

void disjoint_set_destroy
(
  disjoint_set *target
);

/**
 * Creates a new set as a copy of another set. Mainly intended for copying the
 * structure - i.e. the attributes. Everything is initialized to default. This
 * is used in adding set attributes to nodes using a prototype.
 */
result disjoint_set_copy
(
  disjoint_set *target,
  disjoint_set *source
);

result disjoint_set_attributes_create
(
  disjoint_set *target,
  uint32 attribute_count
);

/**
 * Creates a union of two sets by selecting one of the sets as the
 * representative of the union. The size of the union will be the sum of the
 * sizes of the two sets.
 * NOTE: If the sets have attributes, handling the attributes of the union must
 * be taken care of separately. It is advisable to write a specialized function
 * to handle the unions of attributed sets.
 */
disjoint_set *disjoint_set_union
(
  disjoint_set *a,
  disjoint_set *b
);

/**
 * Finds the representative of the sets by following the id links and caching
 * the direct link to it to every node along the way. This compresses the search
 * path and speeds up using the data structure.
 */
disjoint_set *disjoint_set_find
(
  disjoint_set *target
);

/**
 * A utility function for turning the pointer of the set representative into an
 * id number. Calls find before returning the id.
 */
uint32 disjoint_set_id
(
  disjoint_set *target
);

uint32 disjoint_set_attrib_size
(
  disjoint_set *target
);

result disjoint_set_attribute_add
(
  attribute_list *target,
  uint32 key,
  uint32 attribute_count,
  disjoint_set **added
);

disjoint_set *disjoint_set_attribute_get
(
  attribute_list *target,
  uint32 key
);

typedef struct disjoint_set_attribute_params_t {
  uint32 key;
  uint32 attribute_count;
  disjoint_set *added;
} disjoint_set_attribute_params;

typedef struct disjoint_set_label_attribute_params_t {
  uint32 set_key;
  uint32 attribute_count;
  uint32 label_key;
  uint32 offset;
  disjoint_set *added;
} disjoint_set_label_attribute_params;

result disjoint_set_label_attribute_add
(
  attribute_list *target,
  uint32 set_key,
  uint32 attribute_count,
  uint32 label_key,
  uint32 offset,
  disjoint_set **added
);

result disjoint_set_stat_attribute_add
(
  attribute_list *target,
  uint32 set_key,
  uint32 attribute_count,
  uint32 stat_key,
  uint32 dep_key,
  disjoint_set **added
);

typedef struct disjoint_set_stat_attribute_params_t {
  uint32 set_key;
  uint32 attribute_count;
  uint32 stat_key;
  uint32 dep_key;
  disjoint_set *added;
} disjoint_set_stat_attribute_params;

result disjoint_set_stat_pos_attribute_add
(
  attribute_list *target,
  uint32 set_key,
  uint32 attribute_count,
  uint32 stat_key,
  uint32 stat_dep_key,
  uint32 pos_key,
  uint32 pos_dep_key,
  disjoint_set **added
);

typedef struct disjoint_set_stat_pos_attribute_params_t {
  uint32 set_key;
  uint32 attribute_count;
  uint32 stat_key;
  uint32 stat_dep_key;
  uint32 pos_key;
  uint32 pos_dep_key;
  disjoint_set *added;
} disjoint_set_stat_pos_attribute_params;

result disjoint_set_add_attr
(
  attribute_list *target,
  pointer params
);

result disjoint_set_add_label_attr
(
  attribute_list *target,
  pointer params
);

result disjoint_set_add_stat_attr
(
  attribute_list *target,
  pointer params
);

result disjoint_set_add_stat_pos_attr
(
  attribute_list *target,
  pointer params
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_SET_H */
