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
  struct attribute_t *dependency;
} attribute;

attribute *attribute_alloc();

void attribute_free
(
  attribute *ptr
);

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

/* maybe adding and finding attributes should be a function of node and link? */

#ifdef __cplusplus
}
#endif

#endif /* CVSU_ATTRIBUTE_H */
