/**
 * @file cvsu_context.c
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

#include "cvsu_macros.h"
#include "cvsu_context.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string expect_stat_accumulator_name = "expect_stat_accumulator";
string ensure_stat_accumulator_name = "ensure_stat_accumulator";
string ensure_reg_accumulator_name = "context_ensure_reg_accumulator";
string ensure_range_overlap_name = "context_ensure_range_overlap";
string ensure_ridge_finder_name = "context_ensure_ridge_finder";
string expect_path_sniffer_name = "expect_path_sniffer";
string expect_edge_parser_name = "expect_edge_parser";

/******************************************************************************/

truth_value is_stat_accumulator
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_stat_accumulator) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

stat_accumulator *has_stat_accumulator
(
  typed_pointer *tptr
)
{
  if (IS_TRUE(is_stat_accumulator(tptr))) {
    return (stat_accumulator*)tptr->value;
  }
  if (IS_TRUE(is_tuple(tptr))) {
    typed_pointer *element;
    /* must have exactly one stat accumulator */
    element = tuple_has_type(tptr, t_stat_accumulator);
    if (element != NULL) {
      return (stat_accumulator*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result expect_stat_accumulator
(
  stat_accumulator **target,
  typed_pointer *tptr
)
{
  TRY();

  *target = has_stat_accumulator(tptr);
  if (*target == NULL) {
    ERROR(BAD_TYPE);
  }

  FINALLY(expect_stat_accumulator);
  RETURN();
}

/******************************************************************************/

result ensure_stat_accumulator
(
  typed_pointer *context,
  stat_accumulator **acc
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(acc);
  *acc = NULL;

  CHECK(ensure_is(context, t_stat_accumulator, &tptr));

  *acc = (stat_accumulator*)tptr->value;

  FINALLY(ensure_stat_accumulator);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
