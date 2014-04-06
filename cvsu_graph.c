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
string link_list_alloc_name = "link_list_alloc";
string link_list_create_name = "link_list_create";
string node_create_name = "node_create";
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
  CHECK_PARAM(key > 0);
  
  /*attribute_destroy(target);*/
  
  if (value->type == t_tuple) {
    ERROR(NOT_IMPLEMENTED);
  }
  else {
    CHECK(typed_pointer_copy(&target->value, value));
  }
  
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
  uint32 size
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_TRUE(attribute_list_is_null(target));
  CHECK_PARAM(size > 0);

  /* reserve one extra slot at the end for extending the list */
  CHECK(memory_allocate((data_pointer*)&target->items, size+1,
                        sizeof(attribute)));
  CHECK(memory_clear((data_pointer)target->items, size+1, sizeof(attribute)));
  target->size = size;
  target->count = 0;

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
      attribute_destroy(&target->items[i]);
    }
    /* TODO: need a destroy functionality for structured attributes */
    if (target->items[target->size].key > 0) {
      printf("not implemented functionality used in attribute_list_destroy");
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
    target->size = 0;
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
  attribute *source,
  attribute **added
)
{
  TRY();
  attribute *new_attr;
  
  *added = NULL;
  if (target->count < target->size) {
    new_attr = &target->items[target->count];
    attribute_nullify(new_attr);
    CHECK(attribute_create(new_attr, source->key, &source->value));
    target->count++;
    /*
    new_attr->key = source->key;
    CHECK(typed_pointer_copy(&new_attr->value, &source->value));
    */
    *added = new_attr;
  }
  else {
    ERROR(NOT_IMPLEMENTED);
  }
  
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
      if (source->items[i].key == key) {
        ptr = &source->items[i];
      }
    }
  }
  return ptr;
}

/******************************************************************************/

void link_nullify
(
  link *target
)
{
  if (target != NULL) {
    target->a.body = NULL;
    target->a.other = NULL;
    target->a.other = NULL;
    target->a.dir = d_UNDEF;
    attribute_list_nullify(&target->a.attributes);
    target->b.body = NULL;
    target->b.other = NULL;
    target->b.origin = NULL;
    target->b.dir = d_UNDEF;
    attribute_list_nullify(&target->b.attributes);
    target->weight = 0;
    attribute_list_nullify(&target->attributes);
  }
}

/******************************************************************************/

link_list *link_list_alloc
()
{
  TRY();
  link_list *ptr;
  
  CHECK(memory_allocate((data_pointer*)&ptr, 1, sizeof(link_list)));
  link_list_nullify(ptr);
  
  FINALLY(link_list_alloc);
  return ptr;
}

/******************************************************************************/

void link_list_free
(
  link_list *ptr
)
{
  if (ptr != NULL) {
    link_list_destroy(ptr);
    memory_deallocate((data_pointer*)&ptr);
  }
}

/******************************************************************************/

result link_list_create
(
  link_list *target,
  uint32 size
)
{
  TRY();
  
  CHECK_POINTER(target);
  CHECK_TRUE(link_list_is_null(target));
  CHECK_PARAM(size > 0);
  
  /* reserve one extra slot at the end for extending the list */
  CHECK(memory_allocate((data_pointer*)&target->items, size+1,
                        sizeof(link_head*)));
  CHECK(memory_clear((data_pointer)target->items, size+1, sizeof(link_head*)));
  target->size = size;
  target->count = 0;
  
  FINALLY(link_list_create);
  RETURN();
}

/******************************************************************************/

void link_list_destroy
(
  link_list *target
)
{
  if (target != NULL) {
    if (target->items[target->size] != NULL) {
      printf("not implemented functionality used in node_list_destroy");
    }
    memory_deallocate((data_pointer*)&target->items);
    link_list_nullify(target);
  }
}

/******************************************************************************/

void link_list_nullify
(
  link_list *target
)
{
  if (target != NULL) {
    target->items = NULL;
    target->size = 0;
    target->count = 0;
  }
}

/******************************************************************************/

truth_value link_list_is_null
(
  link_list *target
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

result node_create
(
  node *target,
  uint32 attr_size,
  uint32 link_size
)
{
  TRY();
  
  CHECK_POINTER(target);
  node_nullify(target);
  
  if (attr_size > 0) {
    CHECK(attribute_list_create(&target->attributes, attr_size));
  }
  if (link_size > 0) {
    CHECK(link_list_create(&target->links, link_size));
  }
  
  FINALLY(node_create);
  RETURN();
}

/******************************************************************************/

void node_destroy
(
  node *target
)
{
  if (target != NULL) {
    attribute_list_destroy(&target->attributes);
    link_list_destroy(&target->links);
    node_nullify(target);
  }
}

/******************************************************************************/

void node_nullify
(
  node *target
)
{
  if (target != NULL) {
    target->x = 0;
    target->y = 0;
    target->orientation = 0;
    target->scale = 0;
    attribute_list_nullify(&target->attributes);
    link_list_nullify(&target->links);
  }
}

/******************************************************************************/

truth_value node_is_null
(
  node *target
)
{
  if (target != NULL) {
    if (IS_TRUE(attribute_list_is_null(&target->attributes)) &&
        IS_TRUE(link_list_is_null(&target->links))) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

graph *graph_alloc
()
{
  TRY();
  graph *ptr;

  CHECK(memory_allocate((data_pointer*)&ptr, 1, sizeof(graph)));
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
  if (ptr != NULL) {
    graph_destroy(ptr);
    memory_deallocate((data_pointer*)&ptr);
  }
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
  CHECK(list_create(&target->nodes, node_size, sizeof(node), 1));
  CHECK(list_create(&target->links, link_size, sizeof(link), 1));

  FINALLY(graph_create);
  RETURN();
}

/******************************************************************************/

void graph_destroy
(
  graph *target
)
{
  list_item *item, *end;
  node *current_node;
  link *current_link;
  if (target != NULL) {
    /*CHECK(attribute_list_destroy(&target->sources));*/
    item = target->links.first.next;
    end = &target->links.last;
    while (item != end) {
      current_link = (link*)item->data;
      /*link_destroy(current_link);*/
      item = item->next;
    }
    list_destroy(&target->links);
    
    item = target->nodes.first.next;
    end = &target->nodes.last;
    while (item != end) {
      current_node = (node*)item->data;
      node_destroy(current_node);
      item = item->next;
    }
    list_destroy(&target->nodes);
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
    attribute_list_nullify(&target->sources);
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
  uint32 w, h, step, stride, offset, size, x, y;
  uint32 node_w, node_h, node_size, node_i, node_j;
  byte *image_data, *image_pos;
  sint32 value;
  type_label type;
  attribute *new_attr;
  node new_node, *node_ptr;
  link new_link, *link_ptr, *prev_e, **prev_s, **curr_s, **temp_s;
  link_head *head_ptr;

  printf("entered graph_create_from_image ");
  printf("ox=%lu oy=%lu sx=%lu sy=%lu n=%lu\n",
         node_offset_x,
         node_offset_y,
         node_step_x,
         node_step_y,
         (unsigned long)neighborhood);

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  
  w = source->width;
  h = source->height;
  step = source->step;
  stride = source->stride;
  offset = source->offset;
  size = w*h;
  image_data = (byte*)source->data;
  
  CHECK_PARAM(node_step_x > 0 && node_step_x < w);
  CHECK_PARAM(node_step_y > 0 && node_step_y < h);
  CHECK_PARAM(node_offset_x < w);
  CHECK_PARAM(node_offset_y < h);
  
  /* TODO: allow null label and create a default attribute with key 1 */
  CHECK_POINTER(attr_label);
  type = attr_label->value.type;
  value = *((sint32*)attr_label->value.value);
  printf("attr type %lu id %lu value %li\n", (uint32)type, attr_label->key, 
         value);
  /* for now, support just byte images */
  CHECK_PARAM(source->type == p_U8);

  /* create structures */
  node_w = (uint32)((w - node_offset_x) / node_step_x);
  node_h = (uint32)((h - node_offset_y) / node_step_y);
  node_size = node_w * node_h;
  CHECK(list_create(&target->nodes, node_size, sizeof(node), 1));
  CHECK(list_create(&target->links, 4*node_size, sizeof(link), 1));

  /*CHECK(attribute_list_create(&target->sources, 4));*/
  
  /* create temporary arrays for caching links */
  CHECK(memory_allocate((data_pointer*)&prev_s, node_w, sizeof(link*)));
  CHECK(memory_clear((data_pointer)prev_s, node_w, sizeof(link*)));
  CHECK(memory_allocate((data_pointer*)&curr_s, node_w, sizeof(link*)));
  CHECK(memory_clear((data_pointer)curr_s, node_w, sizeof(link*)));
  temp_s = NULL;
  prev_e = NULL;
  
  node_nullify(&new_node);
  new_node.scale = 1;
  link_nullify(&new_link);
  new_link.weight = 1;
  printf("creating nodes\n");
  /* initialize nodes */
  node_j = 0;
  for (y = node_offset_y; y < h; y += node_step_y) {
    image_pos = image_data + y * stride + (offset + node_offset_x) * step;
    node_i = 0;
    for (x = node_offset_x; x < w; x += node_step_x) {
      value = (sint32)*image_pos;
      
      image_pos += node_step_x*step;
      CHECK(list_append_return_pointer(&target->nodes, (pointer)&new_node,
                                       (pointer*)&node_ptr));
      CHECK(node_create(node_ptr, 4, 4));
      node_ptr->x = (integral_value)x;
      node_ptr->y = (integral_value)y;
      CHECK(attribute_add(&node_ptr->attributes, attr_label, &new_attr));
      (*((sint32*)new_attr->value.value)) = value;
      
      /* initialize links */
      /* add s link if not at edge, cache */
      if (node_j < node_h-1) {
        CHECK(list_append_return_pointer(&target->links, (pointer)&new_link,
                                         (pointer*)&link_ptr));
        head_ptr = &link_ptr->a;
        head_ptr->body = link_ptr;
        head_ptr->other = &link_ptr->b;
        head_ptr->origin = node_ptr;
        head_ptr->dir = d_S;
        node_ptr->links.items[2] = head_ptr;
        curr_s[node_i] = link_ptr;
      }
      /* add w link by using cached node's link */
      if (node_i > 0) {
        link_ptr = prev_e;
        head_ptr = &link_ptr->b;
        head_ptr->body = link_ptr;
        head_ptr->other = &link_ptr->a;
        head_ptr->origin = node_ptr;
        head_ptr->dir = d_W;
        node_ptr->links.items[3] = head_ptr;
      }
      /* add e link if not at edge, cache */
      if (node_i < node_w-1) {
        CHECK(list_append_return_pointer(&target->links, (pointer)&new_link,
                                         (pointer*)&link_ptr));
        head_ptr = &link_ptr->a;
        head_ptr->body = link_ptr;
        head_ptr->other = &link_ptr->b;
        head_ptr->origin = node_ptr;
        head_ptr->dir = d_E;
        node_ptr->links.items[1] = head_ptr;
        prev_e = link_ptr;
      }
      /* add n link by using cached node's link */
      if (node_j > 0) {
        link_ptr = prev_s[node_i];
        head_ptr = &link_ptr->b;
        head_ptr->body = link_ptr;
        head_ptr->other = &link_ptr->b;
        head_ptr->origin = node_ptr;
        head_ptr->dir = d_N;
        node_ptr->links.items[0] = head_ptr;
      }
      node_ptr->links.count = (uint32)neighborhood;
      node_i++;
    }
    temp_s = prev_s;
    prev_s = curr_s;
    curr_s = temp_s;
    node_j++;
  }

  FINALLY(graph_create_from_image);
  memory_deallocate((data_pointer*)&prev_s);
  memory_deallocate((data_pointer*)&curr_s);
  printf("exiting graph_create_from_image\n");
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
