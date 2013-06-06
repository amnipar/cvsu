/**
 * @file cvsu_context.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief Context structures for image parsing algorithms.
 *
 * Copyright (c) 2013, Matti Johannes Eskelinen
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

#ifndef CVSU_CONTEXT_H
#   define CVSU_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_config.h"
#include "cvsu_types.h"
#include "cvsu_typed_pointer.h"

/**
 * Context value for accumulating neighborhood statistics.
 */
typedef struct stat_accumulator_t {
  uint32 round;
  integral_value mean_pool1;
  integral_value mean_acc1;
  integral_value mean_pool2;
  integral_value mean_acc2;
  integral_value dev_pool1;
  integral_value dev_acc1;
  integral_value dev_pool2;
  integral_value dev_acc2;
} stat_accumulator;

truth_value is_stat_accumulator
(
  typed_pointer *tptr
);

stat_accumulator *has_stat_accumulator
(
  typed_pointer *tptr
);

result expect_stat_accumulator
(
  stat_accumulator **target,
  typed_pointer *tptr
);

result ensure_stat_accumulator
(
  typed_pointer *context,
  stat_accumulator **acc
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_CONTEXT_H */
