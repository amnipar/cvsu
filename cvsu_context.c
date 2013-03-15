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

#include "cvsu_context.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string expect_stat_accumulator_name = "expect_stat_accumulator";
string expect_path_sniffer_name = "expect_path_sniffer";
string expect_edge_parser_name = "expect_edge_parser";

/******************************************************************************/
/* TODO: these should be moved to some other file, along with the structs...  */

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

result add_stat_accumulator
(
  parse_context *context,
  stat_accumulator **acc
)
{
  TRY();
  CHECK_POINTER(acc);
  *acc = NULL;
  CHECK_POINTER(context);

  if (context.data.type == t_UNDEF) {
    CHECK(typed_pointer_create(&context.data, t_STAT_ACCUMULATOR, 1));
    *acc = (stat_accumulator*)context.data.value;
  }
  else {
    typed_pointer tptr;
    CHECK(typed_pointer_create(&tptr, t_STAT_ACCUMULATOR, 1));
    if (context.data.type != t_TUPLE) {
      CHECK(tuple_promote(&context.data));
    }
    CHECK(tuple_extend(&context.data, &tptr, acc));
  }

  FINALLY();
  RETURN();
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

  if (tptr->type == t_STAT_ACCUMULATOR) {
    *target = (stat_accumulator*)tptr->value;
  }
  else {
    ERROR(BAD_TYPE);
  }

  FINALLY(expect_stat_accumulator);
  RETURN();
}

/******************************************************************************/

void make_path_sniffer
(
  typed_pointer *tptr,
  path_sniffer *source
)
{
  tptr->type = t_PATH_SNIFFER;
  tptr->value = (pointer)source;
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

typedef struct segment_parser_t {

} segment_parser;

/******************************************************************************/

typedef struct edge_parser_t {
  integral_value pool_cost;
  integral_value acc_cost;
  uint32 pool_length;
  uint32 acc_length;
} edge_parser;

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

/******************************************************************************/

typedef struct segment_parser_t {

} segment_parser;

/******************************************************************************/

typedef struct edge_parser_t {
  integral_value pool_cost;
  integral_value acc_cost;
  uint32 pool_length;
  uint32 acc_length;
} edge_parser;

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
