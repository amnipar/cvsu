/**
 * @file cvsu_graph.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief A generic attributed graph structure.
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
#include "cvsu_graph.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string attribute_alloc_name = "attribute_alloc";
string attribute_create_name = "attribute_create";
string attribute_list_alloc_name = "attribute_list_alloc";
string attribute_list_create_name = "attribute_list_create";
string attribute_list_nullify_name = "attribute_list_nullify";
string attribute_add_name = "attribute_add";
string attribute_find_name = "attribute_find";
string graph_alloc_name = "graph_alloc";
string graph_create_name = "graph_create";
string graph_create_from_image_name = "graph_create_from_image";

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

  attribute_destroy(target);
  if (value->type == t_tuple) {
    ERROR(NOT_IMPLEMENTED);
  }
  else {
    CHECK(typed_pointer_create(&target->value, value->type, value->count));
    /* TODO: typed_pointer_clone ? */
  }
  CHECK(typed_pointer_copy(&target->value, value));
  target->key = key;

  FINALLY(attribute_create);
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
  uint32 count
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_TRUE(attribute_list_is_null(target));
  CHECK_PARAM(count > 0);

  CHECK(memory_allocate((data_pointer*)&target->items, count+1,
                        sizeof(attribute)));
  target->count = count;

  FINALLY(attribute_list_create);
  RETURN();
}

/******************************************************************************/

void attribute_list_destroy
(
  attribute_list *target
)
{
  if (target != NULL) {
    int i;

    for (i = 0; i < target->count; i++) {
      if (target->items[i].key > 0 ) {
        attribute_destroy(&target->items[i]);
      }
    }
    /* TODO: need a destroy functionality for structured attributes */
    if (target->items[target->count].key > 0) {
      ERROR(NOT_IMPLEMENTED);
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

result attribute_add
(
  attribute_list *target,
  attribute *source
)
{
  TRY();
  ERROR(NOT_IMPLEMENTED);
  FINALLY(attribute_add);
  RETURN();
}

/******************************************************************************/

attribute *attribute_find
(
  attribute_list *source,
  uint32 key
)
{
  int i;
  attribute *ptr;
  ptr = NULL;
  if (source != NULL) {
    for (i = 0; i < source->count; i++) {
      if (source->items[i].key == 0) {
        break;
      }
      if (source->items[i].key == key) {
        ptr = &source->items[i];
      }
    }
  }
  return ptr;
}

/******************************************************************************/

graph *graph_alloc
()
{
  TRY();
  graph *ptr;

  CHECK(memory_allocate((data_pointer*)&ptr, 1, sizeof(graph)))
  graph_nullify(ptr);

  FINALLY(graph_alloc);
  return ptr;
}

/******************************************************************************/

void graph_free
(
  graph *ptr
)
{
  TRY();

  r = SUCCESS;
  if (ptr != NULL) {
    graph_destroy(ptr);
    CHECK(memory_deallocate((data_pointer*)&ptr));
  }

  FINALLY(graph_free);
}

/******************************************************************************/

result graph_create
(
  graph *target,
  uint32 node_size,
  uint32 link_size,
  attribute *attr_label
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(attr_label);

  /* create structures */
  CHECK(list_create(&target->nodes, size, sizeof(node)));
  CHECK(list_create(&target->links, neighborhood*size, sizeof(link)));

  FINALLY(graph_create);
  RETURN();
}

/******************************************************************************/

void graph_destroy
(
  graph *target
)
{
  if (target != NULL) {
    /*CHECK(attribute_list_destroy(&target->images));*/
    CHECK(list_destroy(&target->links));
    /* destroying the nodes might be a bit tricky - typed pointers in attrs */
    CHECK(list_destroy(&target->nodes));
  }
}

/******************************************************************************/

void graph_nullify
(
  graph *target
)
{
  if (target != NULL) {
    list_nullify(&target->nodes);
    list_nullify(&target->links);
    attribute_list_nullify(&target->images);
  }
}

/******************************************************************************/

truth_value graph_is_null
(
  graph *target
)
{
  if (target != NULL) {
    if (list_is_null(&target->nodes) || list_is_null(&target->links)) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

result graph_create_from_image
(
  graph *target,
  pixel_image *source,
  uint32 node_offset_x,
  uint32 node_offset_y,
  uint32 node_step_x,
  uint32 node_step_y,
  graph_neighborhood neighborhood,
  attribute *attr_label
)
{
  TRY();
  uint32 w, h, step, stride, offset, size, i, j;
  float *image_data, *image_pos;
  node *node_data, *node_pos;
  int value;
  type_label type;

  printf("entered graph_create_from_image ");
  printf("ox=%ul oy=%ul sx=%ul sy=%ul n=%ul\n",
         node_offset_x,
         node_offset_y,
         node_step_x,
         node_step_y,
         (unsigned long)neighborhood);

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  /* TODO: allow null label and create a default attribute with key 1 */
  CHECK_POINTER(attr_label);
  type = attr_label->value.type;
  value = *((int*)attr_label->value.value);
  printf("attr type %d id %d value %d\n", (int)type, attr_label->key, value);
  /* for now, support just float images */
  CHECK_PARAM(source->type == p_F32);


  w = source->width;
  h = source->height;
  step = source->step;
  stride = source->stride;
  offset = source->offset;
  size = w*h;
  image_data = (float*)source->data;

  /* create structures */
  CHECK(list_create(&target->nodes, size, sizeof(node)));
  CHECK(list_create(&target->links, neighborhood*size, sizeof(link)));

  /*CHECK(attribute_list_create(&target->images, 4));*/

  node_data = (node*)target->nodes

  for (j = 0; j < h; j++) {
    pos = image_data + j * stride + offset;
    for (i = 0; i < w; i++) {

      pos += step;
    }
  }

  /* initialize nodes and links */

  FINALLY(graph_create_from_image);
  printf("exiting graph_create_from_image\n")
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
