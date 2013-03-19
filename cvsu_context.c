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

string context_ensure_stat_accumulator_name = "context_ensure_stat_accumulator";
string expect_stat_accumulator_name = "expect_stat_accumulator";
string context_ensure_reg_accumulator_name = "context_ensure_reg_accumulator";
string context_ensure_range_overlap_name = "context_ensure_range_overlap";
string context_ensure_ridge_finder_name = "context_ensure_ridge_finder";
string expect_path_sniffer_name = "expect_path_sniffer";
string expect_edge_parser_name = "expect_edge_parser";

/******************************************************************************/

result context_ensure_stat_accumulator
(
  parse_context *context,
  stat_accumulator **acc
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(acc);
  *acc = NULL;
  CHECK_POINTER(context);

  CHECK(tuple_ensure_has_unique(&context->data, t_STAT_ACCUMULATOR, &tptr));

  *acc = (stat_accumulator*)tptr->value;

  FINALLY(context_ensure_stat_accumulator);
  RETURN();
}

/******************************************************************************/

truth_value is_stat_accumulator
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_STAT_ACCUMULATOR) {
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
    element = tuple_has_type(tptr, t_STAT_ACCUMULATOR, 1, 1);
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

  CHECK_POINTER(target);
  CHECK_POINTER(tptr);
  CHECK_POINTER(tptr->value);

  *target = has_stat_accumulator(tptr);
  if (*target == NULL) {
    ERROR(BAD_TYPE);
  }

  FINALLY(expect_stat_accumulator);
  RETURN();
}

/******************************************************************************/

result context_ensure_reg_accumulator
(
  parse_context *context,
  reg_accumulator **reg
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(reg);
  *reg = NULL;
  CHECK_POINTER(context);

  CHECK(tuple_ensure_has_unique(&context->data, t_REG_ACCUMULATOR, &tptr));

  *reg = (reg_accumulator*)tptr->value;

  FINALLY(context_ensure_reg_accumulator);
  RETURN();
}

/******************************************************************************/

truth_value is_reg_accumulator
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_REG_ACCUMULATOR) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

reg_accumulator *has_reg_accumulator
(
  typed_pointer *tptr
)
{
  if (IS_TRUE(is_reg_accumulator(tptr))) {
    return (reg_accumulator*)tptr->value;
  }
  if (IS_TRUE(is_tuple(tptr))) {
    typed_pointer *element;
    /* must have exactly one stat accumulator */
    element = tuple_has_type(tptr, t_REG_ACCUMULATOR, 1, 1);
    if (element != NULL) {
      return (reg_accumulator*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result context_ensure_range_overlap
(
  parse_context *context,
  range_overlap **overlap
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(overlap);
  *overlap = NULL;
  CHECK_POINTER(context);

  CHECK(tuple_ensure_has_unique(&context->data, t_RANGE_OVERLAP, &tptr));

  *overlap = (range_overlap*)tptr->value;

  FINALLY(context_ensure_range_overlap);
  RETURN();
}

/******************************************************************************/

truth_value is_range_overlap
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_RANGE_OVERLAP) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

range_overlap *has_range_overlap
(
  typed_pointer *tptr
)
{
  if (IS_TRUE(is_range_overlap(tptr))) {
    return (range_overlap*)tptr->value;
  }
  if (IS_TRUE(is_tuple(tptr))) {
    typed_pointer *element;
    /* must have exactly one stat accumulator */
    element = tuple_has_type(tptr, t_RANGE_OVERLAP, 1, 1);
    if (element != NULL) {
      return (range_overlap*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

result context_ensure_ridge_finder
(
  parse_context *context,
  ridge_finder **rfind
)
{
  TRY();
  typed_pointer *tptr;

  CHECK_POINTER(rfind);
  *rfind = NULL;
  CHECK_POINTER(context);

  CHECK(tuple_ensure_has_unique(&context->data, t_RIDGE_FINDER, &tptr));

  *rfind = (ridge_finder*)tptr->value;

  FINALLY(context_ensure_ridge_finder);
  RETURN();
}

/******************************************************************************/

truth_value is_ridge_finder
(
  typed_pointer *tptr
)
{
  if (tptr != NULL && tptr->type == t_RIDGE_FINDER) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

ridge_finder *has_ridge_finder
(
  typed_pointer *tptr
)
{
  if (IS_TRUE(is_ridge_finder(tptr))) {
    return (ridge_finder*)tptr->value;
  }
  if (IS_TRUE(is_tuple(tptr))) {
    typed_pointer *element;
    /* must have exactly one stat accumulator */
    element = tuple_has_type(tptr, t_RIDGE_FINDER, 1, 1);
    if (element != NULL) {
      return (ridge_finder*)element->value;
    }
  }
  return NULL;
}

/******************************************************************************/

truth_value is_path_sniffer
(
  typed_pointer *tptr
)
{
  if (tptr->type == t_PATH_SNIFFER) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

result expect_path_sniffer
(
  path_sniffer **target,
  typed_pointer *tptr
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(tptr);
  CHECK_POINTER(tptr->value);

  if (tptr->type == t_PATH_SNIFFER) {
    *target = (path_sniffer*)tptr->value;
  }
  else {
    ERROR(BAD_TYPE);
  }

  FINALLY(expect_path_sniffer);
  RETURN();
}

/******************************************************************************/

void make_edge_parser
(
  typed_pointer *tptr,
  edge_parser *parser
)
{
  tptr->type = t_EDGE_PARSER;
  tptr->value = (pointer)parser;
}

/******************************************************************************/

truth_value is_edge_parser
(
  typed_pointer *tptr
)
{
  if (tptr->type == t_EDGE_PARSER) {
    return TRUE;
  }
  return FALSE;
}

/******************************************************************************/

result expect_edge_parser
(
  edge_parser **target,
  const typed_pointer *tptr
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(tptr);
  CHECK_POINTER(tptr->value);

  if (tptr->type == t_EDGE_PARSER) {
    *target = (edge_parser*)tptr->value;
  }
  else {
    ERROR(BAD_TYPE);
  }

  FINALLY(expect_edge_parser);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
