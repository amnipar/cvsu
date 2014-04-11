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

/**
 * Defines a disjoint set datatype, which is used to encode set membership by
 * ownership of a set attribute, and union operations are managed with a
 * union-find system.
 */
typedef struct disjoint_set_t {
  struct disjoint_set_t *id;
  uint32 rank;
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

void disjoint_set_create
(
  disjoint_set *target
);

disjoint_set *disjoint_set_union
(
  disjoint_set *a,
  disjoint_set *b
);

disjoint_set *disjoint_set_find
(
  disjoint_set *target
);

uint32 disjoint_set_id
(
  disjoint_set *target
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_SET_H */