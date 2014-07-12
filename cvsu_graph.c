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

string link_create_name = "link_create";
string link_weight_range_update_name = "link_weight_range_update";
string link_attribute_range_update_name = "link_attribute_range_update";

string link_list_alloc_name = "link_list_alloc";
string link_list_create_name = "link_list_create";

string node_create_name = "node_create";
string node_weight_range_update_name = "node_weight_range_update";
string node_attribute_range_update_name = "node_attribute_range_update";

string graph_alloc_name = "graph_alloc";
string graph_create_name = "graph_create";
string graph_add_node_name = "graph_add_node";
string graph_add_link_name = "graph_add_link";

string graph_for_each_node_name = "graph_for_each_node";
string graph_for_attrs_in_each_node_name = "graph_for_attrs_in_each_node";
string graph_for_each_link_name = "graph_for_each_link";
string graph_for_attrs_in_each_link_name = "graph_for_attrs_in_each_link";

string graph_create_from_image_name = "graph_create_from_image";

/******************************************************************************/

result link_create
(
  link *target,
  uint32 attr_size
)
{
  TRY();

  CHECK_POINTER(target);
  link_nullify(target);

  if (attr_size > 0) {
    CHECK(attribute_list_create(&target->attributes, attr_size));
  }

  FINALLY(link_create);
  RETURN();
}

/******************************************************************************/

void link_destroy
(
  link *target
)
{
  if (target != NULL) {
    /* TODO: need to destroy attribute lists of heads when applicable */
    attribute_list_destroy(&target->attributes);
    link_nullify(target);
  }
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
    target->a.origin = NULL;
    target->a.dir = d_UNDEF;
    attribute_list_nullify(&target->a.attributes);
    target->b.body = NULL;
    target->b.other = NULL;
    target->b.origin = NULL;
    target->b.dir = d_UNDEF;
    attribute_list_nullify(&target->b.attributes);
    target->weight_attribute = NULL;
    target->weight = 0;
    attribute_list_nullify(&target->attributes);
  }
}

/******************************************************************************/

truth_value link_is_null
(
  link *target
)
{
  if (target != NULL) {
    if (IS_TRUE(attribute_list_is_null(&target->attributes))) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

result link_weight_range_update
(
  link *target,
  pointer params
)
{
  TRY();
  attribute *attr;
  value_range *current;
  real value;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  current = (value_range*)params;
  attr = target->weight_attribute;
  value = 0;
  if (attr != NULL) {
    value = typed_pointer_cast_from(&attr->value);
  }
  if (value < current->min_value) {
    current->min_value = value;
  }
  if (value > current->max_value) {
    current->max_value = value;
  }
  
  FINALLY(link_weight_range_update);
  RETURN();
}

/******************************************************************************/

result link_attribute_range_update
(
  link *target,
  pointer params
)
{
  TRY();
  attribute *attr;
  attribute_range *current;
  real value;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  current = (attribute_range*)params;
  
  attr = attribute_find(&target->attributes, current->key);
  value = 0;
  if (attr != NULL) {
    value = typed_pointer_cast_from(&attr->value);
  }
  if (value < current->min_value) {
    current->min_value = value;
  }
  if (value > current->max_value) {
    current->max_value = value;
  }
  
  FINALLY(link_attribute_range_update);
  RETURN();
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
    target->id = 0;
    target->pos.x = 0;
    target->pos.y = 0;
    target->weight_attribute = NULL;
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

result node_weight_range_update
(
  node *target,
  pointer params
)
{
  TRY();
  attribute *attr;
  value_range *current;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  current = (value_range*)params;
  attr = target->weight_attribute;
  if (attr != NULL) {
    real value = typed_pointer_cast_from(&attr->value);
    if (value < current->min_value) {
      current->min_value = value;
    }
    if (value > current->max_value) {
      current->max_value = value;
    }
  }
  
  FINALLY(node_weight_range_update);
  RETURN();
}

/******************************************************************************/

result node_attribute_range_update
(
  node *target,
  pointer params
)
{
  TRY();
  attribute *attr;
  attribute_range *current;
  real value;
  
  CHECK_POINTER(target);
  CHECK_POINTER(params);
  
  current = (attribute_range*)params;
  attr = attribute_find(&target->attributes, current->key);
  if (attr != NULL) {
    value = typed_pointer_cast_from(&attr->value);
    if (value < current->min_value) {
      current->min_value = value;
    }
    if (value > current->max_value) {
      current->max_value = value;
    }
  }
  
  FINALLY(node_attribute_range_update);
  RETURN();
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
  printf("graph_free\n");
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
  uint32 link_size
)
{
  TRY();

  CHECK_POINTER(target);

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
      link_destroy(current_link);
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
    if (list_is_null(&target->nodes) && list_is_null(&target->links)) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

result graph_add_node
(
  graph *target,
  uint32 attr_size,
  uint32 link_size,
  node **added
)
{
  TRY();
  node new_node, *node_ptr;
  
  CHECK_POINTER(target);
  CHECK_POINTER(added);
  
  CHECK(list_append_return_pointer(&target->nodes, (pointer)&new_node,
                                       (pointer*)&node_ptr));
  CHECK(node_create(node_ptr, attr_size, link_size));
  
  *added = node_ptr;
  
  FINALLY(graph_add_node);
  RETURN();
}

/******************************************************************************/

result graph_add_link
(
  graph *target,
  uint32 attr_size,
  link **added
)
{
  TRY();
  link new_link, *link_ptr;
 
  CHECK_POINTER(target);
  CHECK_POINTER(added);
  
  CHECK(list_append_return_pointer(&target->links, (pointer)&new_link,
                                         (pointer*)&link_ptr));
  CHECK(link_create(link_ptr, attr_size));
  
  *added = link_ptr;
  
  FINALLY(graph_add_link);
  RETURN();
}

/******************************************************************************/

result graph_for_each_node
(
  graph *target,
  node_function func,
  pointer params
)
{
  TRY();
  list_item *items, *end;
  node *current_node;
  
  CHECK_POINTER(target);

  items = target->nodes.first.next;
  end = &target->nodes.last;
  while (items != end) {
    current_node = (node*)items->data;
    CHECK(func(current_node, params));
    items = items->next;
  }
  
  FINALLY(graph_for_each_node);
  RETURN();
}

/******************************************************************************/

result graph_for_attrs_in_each_node
(
  graph *target,
  attribute_list_function func,
  pointer params
)
{
  TRY();
  list_item *items, *end;
  node *current_node;
  
  CHECK_POINTER(target);
  
  items = target->nodes.first.next;
  end = &target->nodes.last;
  while (items != end) {
    current_node = (node*)items->data;
    CHECK(func(&current_node->attributes, params));
    items = items->next;
  }
  
  FINALLY(graph_for_attrs_in_each_node);
  RETURN();
}

/******************************************************************************/

result graph_for_each_link
(
  graph *target,
  link_function func,
  pointer params
)
{
  TRY();
  list_item *items, *end;
  link *current_link;
  
  CHECK_POINTER(target);
  
  items = target->links.first.next;
  end = &target->links.last;
  while (items != end) {
    current_link = (link*)items->data;
    CHECK(func(current_link, params));
    items = items->next;
  }
  
  FINALLY(graph_for_each_link);
  RETURN();
}

/******************************************************************************/

result graph_for_attrs_in_each_link
(
  graph *target,
  attribute_list_function func,
  pointer params
)
{
  TRY();
  list_item *items, *end;
  link *current_link;
  
  CHECK_POINTER(target);
  
  items = target->links.first.next;
  end = &target->links.last;
  while (items != end) {
    current_link = (link*)items->data;
    CHECK(func(&current_link->attributes, params));
    items = items->next;
  }
  
  FINALLY(graph_for_attrs_in_each_link);
  RETURN();
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
  uint32 value_key,
  uint32 weight_key
)
{
  TRY();
  uint32 w, h, step, stride, offset, size, x, y, pixel_offset;
  uint32 node_w, node_h, node_size, node_i, node_j, link_size;
  void *image_data;
  pixel_type type;
  pixel_value value, *new_value;
  real weight, *new_weight;
  typed_pointer tptr_value, tptr_weight;
  attribute value_attr, *value_ptr;
  attribute weight_attr, *weight_ptr;
  node new_node, *node_ptr;
  link new_link, *link_ptr, *prev_e, **prev_s;/*, **curr_s, **temp_s;*/
  link_head *head_ptr;

  CHECK_POINTER(target);
  CHECK_POINTER(source);

  w = source->width;
  h = source->height;
  step = source->step;
  stride = source->stride;
  offset = source->offset;
  size = w*h;
  image_data = source->data;
  type = source->type;

  CHECK_PARAM(node_step_x > 0 && node_step_x < w);
  CHECK_PARAM(node_step_y > 0 && node_step_y < h);
  CHECK_PARAM(node_offset_x < w);
  CHECK_PARAM(node_offset_y < h);
  CHECK_PARAM(neighborhood == NEIGHBORHOOD_4 || neighborhood == NEIGHBORHOOD_8);

  /* TODO: allow null label and create a default attribute with key 1 */
  value.offset = 0;
  value.token = 0;
  value.cache = 0;
  weight = 0;
  
  typed_pointer_nullify(&tptr_value);
  CHECK(typed_pointer_create(&tptr_value, t_pixel_value, 1, 1, 
                             (pointer)&value));
  
  attribute_nullify(&value_attr);
  CHECK(attribute_create(&value_attr, value_key, &tptr_value));
  
  typed_pointer_nullify(&tptr_weight);
  CHECK(typed_pointer_create(&tptr_weight, t_real, 1, 1, (pointer)&weight));
  
  attribute_nullify(&weight_attr);
  CHECK(attribute_create(&weight_attr, weight_key, &tptr_weight));

  /* create structures */
  node_w = (uint32)((w - node_offset_x - 1) / node_step_x) + 1;
  node_h = (uint32)((h - node_offset_y - 1) / node_step_y) + 1;
  node_size = node_w * node_h;
  link_size = (uint32)neighborhood;
  
  CHECK(graph_create(target, node_size, link_size*node_size));
  /*
  CHECK(list_create(&target->nodes, node_size, sizeof(node), 1));
  CHECK(list_create(&target->links, link_size*node_size, sizeof(link), 1));
  */
  /*CHECK(attribute_list_create(&target->sources, 4));*/

  /* create temporary arrays for caching links */
  CHECK(memory_allocate((data_pointer*)&prev_s, node_w, sizeof(link*)));
  CHECK(memory_clear((data_pointer)prev_s, node_w, sizeof(link*)));
  prev_e = NULL;
  
  /* initialize nodes */
  node_j = 0;
  for (y = node_offset_y; y < h; y += node_step_y) {
    /*image_pos = image_data + y * stride + (offset + node_offset_x) * step;*/
    pixel_offset = y * stride + (offset + node_offset_x) * step;
    node_i = 0;
    for (x = node_offset_x; x < w; x += node_step_x) {
      CHECK(graph_add_node(target, 4, 4, &node_ptr));
      /*
      CHECK(list_append_return_pointer(&target->nodes, (pointer)&new_node,
                                       (pointer*)&node_ptr));
      CHECK(node_create(node_ptr, 4, 4));
      */
      node_ptr->pos.x = (real)x;
      node_ptr->pos.y = (real)y;
      CHECK(attribute_add(&node_ptr->attributes, &value_attr, &value_ptr));
      /*printf("init value attr\n");*/
      new_value = ((pixel_value*)value_ptr->value.value);
      new_value->offset = pixel_offset;
      pixel_value_cache(new_value, image_data, type, 1);
      node_ptr->weight_attribute = value_ptr;
      /*(*((sint32*)new_attr->value.value)) = value;*/
      /* got pixel value, can move pixel offset */
      /*image_pos += (node_step_x*step);*/
      pixel_offset += (node_step_x*step);

      /* initialize links */
      /* add n link by using cached node's link */
      if (node_j > 0) {
        link_ptr = prev_s[node_i];
        head_ptr = &link_ptr->b;
        head_ptr->body = link_ptr;
        head_ptr->other = &link_ptr->a;
        head_ptr->origin = node_ptr;
        head_ptr->dir = d_N;
        new_weight = ((real*)link_ptr->weight_attribute->value.value);
        *new_weight = fabs(*new_weight - new_value->cache);
        node_ptr->links.items[0] = head_ptr;
      }
      /* add w link by using cached node's link */
      if (node_i > 0) {
        link_ptr = prev_e;
        head_ptr = &link_ptr->b;
        head_ptr->body = link_ptr;
        head_ptr->other = &link_ptr->a;
        head_ptr->origin = node_ptr;
        head_ptr->dir = d_W;
        new_weight = ((real*)link_ptr->weight_attribute->value.value);
        *new_weight = fabs(*new_weight - new_value->cache);
        node_ptr->links.items[3] = head_ptr;
      }
      /* add s link if not at edge, cache */
      if (node_j < (node_h-1)) {
        CHECK(graph_add_link(target, 4, &link_ptr));
        /*
        CHECK(list_append_return_pointer(&target->links, (pointer)&new_link,
                                         (pointer*)&link_ptr));
        CHECK(link_create(link_ptr, 4));
        */
        CHECK(attribute_add(&link_ptr->attributes, &weight_attr, &weight_ptr));
        new_weight = ((real*)weight_ptr->value.value);
        link_ptr->weight_attribute = weight_ptr;
        head_ptr = &link_ptr->a;
        head_ptr->body = link_ptr;
        head_ptr->other = &link_ptr->b;
        head_ptr->origin = node_ptr;
        head_ptr->dir = d_S;
        node_ptr->links.items[2] = head_ptr;
        *new_weight = new_value->cache;
        prev_s[node_i] = link_ptr;
      }
      /* add e link if not at edge, cache */
      if (node_i < (node_w-1)) {
        CHECK(graph_add_link(target, 4, &link_ptr));
        /*
        CHECK(list_append_return_pointer(&target->links, (pointer)&new_link,
                                         (pointer*)&link_ptr));
        CHECK(link_create(link_ptr, 4));
        */
        CHECK(attribute_add(&link_ptr->attributes, &weight_attr, &weight_ptr));
        new_weight = ((real*)weight_ptr->value.value);
        link_ptr->weight_attribute = weight_ptr;
        head_ptr = &link_ptr->a;
        head_ptr->body = link_ptr;
        head_ptr->other = &link_ptr->b;
        head_ptr->origin = node_ptr;
        head_ptr->dir = d_E;
        node_ptr->links.items[1] = head_ptr;
        *new_weight = new_value->cache;
        prev_e = link_ptr;
      }
      node_ptr->links.count = (uint32)neighborhood;
      node_i++;
    }
    node_j++;
  }

  FINALLY(graph_create_from_image);

  prev_e = NULL;
  memory_deallocate((data_pointer*)&prev_s);

  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
