/**
 * @file cvsu_list.c
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief A double-linked list that stores any object as void pointer.
 *
 * Copyright (c) 2011-2013, Matti Johannes Eskelinen
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
#include "cvsu_list.h"

/******************************************************************************/
/* constants for reporting function names in error messages                   */

string chunk_create_name = "chunk_create";
string chunk_destroy_name = "chunk_destroy";
string chunk_nullify_name = "chunk_nullify";
string chunk_clear_name = "chunk_clear";
string chunk_allocate_item_name = "chunk_allocate_item";
string chunk_get_item_name = "chunk_get_item";

string list_item_nullify_name = "list_item_nullify";

string list_alloc_name = "list_alloc";
string list_free_name = "list_free";
string list_create_name = "list_create";
string list_create_from_data_name = "list_create_from_data";
string list_destroy_name = "list_destroy";
string list_nullify_name = "list_nullify";
string list_clear_name = "list_clear";
string list_pack_name = "list_pack";
string sublist_create_name = "sublist_create";

string list_create_item_name = "list_create_item";
string list_link_item_name = "list_link_item";
string list_remove_item_name = "list_remove_item";
string item_insert_before_name = "item_insert_before";
string item_insert_after_name = "item_insert_after";
string item_remove_name = "item_remove";

string list_append_name = "list_append";
string list_append_return_pointer_name = "list_append_return_pointer";
string list_append_index_name = "list_append_index";
string sublist_append_name = "sublist_append";
string list_prepend_name = "list_prepend";
string list_prepend_index_name = "list_prepend_index";
string list_insert_at_name = "list_insert_at";
string list_insert_sorted_name = "list_insert_sorted";
string list_insert_sorted_index_name = "list_insert_sorted_index";
string list_insert_unique_name = "list_insert_unique";
string list_insert_unique_index_name = "list_insert_unique_index";
string list_remove_name = "list_remove";
string list_remove_between_name = "list_remove_between";
string list_remove_rest_name = "list_remove_rest";
string list_iterate_forward_name = "list_iterate_forward";
string list_iterate_backward_name = "list_iterate_backward";

string pointer_list_create_name = "pointer_list_create";
string pointer_sublist_create_name = "pointer_sublist_create";
string pointer_list_destroy_name = "pointer_list_destroy";
string pointer_list_append_name = "pointer_list_append";
string pointer_list_prepend_name = "pointer_list_prepend";

/******************************************************************************/

result chunk_create
(
  chunk *target,
  uint32 max_size,
  uint32 item_size
)
{
  TRY();

  CHECK_POINTER(target);

  /* allocate space for the chunk array */
  CHECK(memory_allocate((data_pointer*)&target->chunks, 10, sizeof(byte*)));
  CHECK(memory_clear((data_pointer)target->chunks, 10, sizeof(byte*)));
  /* allocate the first chunk */
  CHECK(memory_allocate(&target->chunk, max_size, item_size));
  CHECK(memory_clear(target->chunk, max_size, item_size));

  target->chunks[0] = target->chunk;
  target->current_chunk = 0;
  target->chunk_count = 10;
  target->item_size = item_size;
  target->size = max_size;
  target->count = 0;

  FINALLY(chunk_create);
  RETURN();
}

/******************************************************************************/

result chunk_destroy
(
  chunk *target
)
{
  TRY();
  uint32 i;

  CHECK_POINTER(target);

  if (target->chunks != NULL) {
    for (i = 0; i <= target->current_chunk; i++) {
      CHECK(memory_deallocate(&target->chunks[i]));
    }
    CHECK(memory_deallocate((data_pointer*)&target->chunks));
  }
  chunk_nullify(target);

  FINALLY(chunk_destroy);
  RETURN();
}

/******************************************************************************/

result chunk_nullify
(
  chunk *target
)
{
  TRY();

  CHECK_POINTER(target);

  target->item_size = 0;
  target->size = 0;
  target->count = 0;
  target->chunk_count = 0;
  target->current_chunk = 0;
  target->chunks = NULL;
  target->chunk = NULL;

  FINALLY(chunk_nullify);
  RETURN();
}

/******************************************************************************/

truth_value chunk_is_null
(
  chunk *target
)
{
  if (target != NULL) {
    if (target->chunks == NULL && target->chunk == NULL) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

result chunk_clear
(
  chunk *target
)
{
  TRY();
  uint32 i;

  CHECK_POINTER(target);

  target->count = 0;

  for (i = 0; i <= target->current_chunk; i++) {
    CHECK(memory_clear(target->chunks[i], target->size, target->item_size));
  }

  FINALLY(chunk_clear);
  RETURN();
}

/******************************************************************************/

result chunk_allocate_item
(
  data_pointer *target,
  chunk *source
)
{
  TRY();
  byte *new_chunk, **new_chunks;

  new_chunk = NULL;
  new_chunks = NULL;

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_POINTER(source->chunk);

  *target = NULL;
  /* if there are items left in the current chunk, take the first free one */
  if (source->count < source->size) {
      *target = source->chunk + source->count * source->item_size;
      source->count++;
  }
  /* if current chunk is full, allocate a new one */
  else {
    CHECK(memory_allocate(&new_chunk, source->size, source->item_size));
    CHECK(memory_clear(new_chunk, source->size, source->item_size));

    source->current_chunk++;
    /* if the chunk array is full, need to allocate a bigger one */
    if (!(source->current_chunk < source->chunk_count)) {
      uint32 new_count, i;

      new_count = source->chunk_count + 10;
      CHECK(memory_allocate((data_pointer*)&new_chunks, new_count, sizeof(byte*)));
      CHECK(memory_clear((data_pointer)new_chunks, new_count, sizeof(byte*)));
      for (i = 0; i < source->chunk_count; i++) {
        new_chunks[i] = source->chunks[i];
      }
      CHECK(memory_deallocate((data_pointer*)&source->chunks));
      source->chunks = new_chunks;
      source->chunk_count = new_count;
    }
    source->chunks[source->current_chunk] = new_chunk;
    source->chunk = new_chunk;
    *target = new_chunk;
    source->count = 1;
  }

  FINALLY(chunk_allocate_item);

  if (r != SUCCESS) {
    if (new_chunk != NULL && source->chunk != new_chunk) {
      memory_deallocate(&new_chunk);
    }
    if (new_chunks != NULL && source->chunks != new_chunks) {
      memory_deallocate((data_pointer*)&new_chunks);
    }
  }

  RETURN();
}

/******************************************************************************/

result chunk_get_item
(
  data_pointer *target,
  chunk *source,
  list_index index
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_POINTER(source->chunk);

  *target = NULL;
  if (index < (source->current_chunk * source->size) + source->count) {
    list_index chunk_index, new_index;

    chunk_index = (list_index)(index / source->size);
    new_index = (list_index)(index % source->size);
    *target = source->chunks[chunk_index] + new_index * source->item_size;
  }
  else {
    ERROR(BAD_PARAM);
  }

  FINALLY(chunk_get_item);
  RETURN();
}

/******************************************************************************/

truth_value chunk_contains_item
(
  chunk *source,
  data_pointer item
)
{
  uint32 i, size;

  if (source == NULL || source->chunks == NULL || item == NULL) {
      return FALSE;
  }
  size = source->size * source->item_size;
  for (i = 0; i <= source->current_chunk; i++) {
    if (item >= source->chunks[i] && item < source->chunks[i] + size) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

data_pointer chunk_return_item
(
  chunk *source,
  list_index index
)
{
  data_pointer target;
  chunk_get_item(&target, source, index);
  return target;
}

/******************************************************************************/

result list_item_nullify
(
  list_item *target
)
{
  TRY();

  CHECK_POINTER(target);

  target->prev = NULL;
  target->next = NULL;
  target->data = NULL;

  FINALLY(list_item_nullify);
  RETURN();
}

/******************************************************************************/

truth_value list_item_is_null
(
  list_item *target
)
{
  if (target != NULL) {
    if (target->data == NULL) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/
/* private function for inserting an item before another item                 */

result item_insert_before
(
  list_item *item,
  list_item *prev
)
{
  TRY();

  CHECK_POINTER(item);
  CHECK_POINTER(prev);
  CHECK_POINTER(item->prev);

  item->prev->next = prev;
  prev->prev = item->prev;
  item->prev = prev;
  prev->next = item;

  FINALLY(item_insert_before);
  RETURN();
}

/******************************************************************************/
/* private function for inserting an item after another item                  */

result item_insert_after
(
  list_item *item,
  list_item *next
)
{
  TRY();

  CHECK_POINTER(item);
  CHECK_POINTER(next);
  CHECK_POINTER(item->next);

  next->prev = item;
  next->next = item->next;
  item->next->prev = next;
  item->next = next;

  FINALLY(item_insert_after);
  RETURN();
}

/******************************************************************************/
/* private function for removing an item between two other items              */

result item_remove
(
  list_item *item
)
{
  TRY();

  CHECK_POINTER(item);
  CHECK_POINTER(item->prev);
  CHECK_POINTER(item->next);

  item->prev->next = item->next;
  item->next->prev = item->prev;
  item->next = NULL;
  item->prev = NULL;

  FINALLY(item_remove);
  RETURN();
}

/******************************************************************************/
/* private function for initializing a list item                              */

result list_create_item
(
  list *target,
  list_item **item,
  void *data
)
{
  TRY();
  list *master;
  truth_value is_master;

  CHECK_POINTER(target);
  CHECK_POINTER(item);
  CHECK_POINTER(data);

  /* for sublists, use the parent list for allocating items */
  if (target->parent != NULL) {
    master = target->parent;
    is_master = FALSE;
    /* data for sublist item must be within the data chunk */
    CHECK_TRUE(chunk_contains_item(&master->data_chunk, (data_pointer)data));
  }
  /* for master lists, use the list itself */
  else {
    master = target;
    is_master = TRUE;
  }

  /* get item from free item list, or allocate if no free items available */
  if (master->first_free.next != &master->last_free) {
    *item = master->first_free.next;
    CHECK(item_remove(*item));
  }
  else {
    CHECK(chunk_allocate_item((data_pointer *)item, &master->item_chunk));
    /* if this is a master list, allocate data for item as well */
    if (IS_TRUE(is_master)) {
      CHECK(chunk_allocate_item((data_pointer *)&(*item)->data, &target->data_chunk));
    }
    target->count++;
  }

  /* for master lists, copy the object to allocated memory */
  if (IS_TRUE(is_master)) {
    CHECK(memory_copy((*item)->data, data, 1, target->data_chunk.item_size));
  }
  /* for sublists, data is not allocated but the same pointer is used */
  else {
    (*item)->data = data;
  }

  FINALLY(list_create_item);
  RETURN();
}

/******************************************************************************/
/* private function for linking data in data chunk by index to a list item    */

result list_link_item
(
  list *target,
  list_item **item,
  list_index index
)
{
  TRY();
  list *master;
  byte *chunk_item;

  CHECK_POINTER(target);
  CHECK_POINTER(item);

  /* for sublists, use the parent list for allocating items */
  if (target->parent != NULL) {
    master = target->parent;
  }
  /* for master lists, use the list itself */
  else {
    master = target;
  }

  /* check that the item pointed to by index is within this list's chunk */
  chunk_item = chunk_return_item(&master->data_chunk, index);
  CHECK_PARAM(chunk_item != NULL);

  if (target->count < target->max_size) {
    /* get item from free item list, or allocate if no free items available */
    if (master->first_free.next != &master->last_free) {
      *item = master->first_free.next;
      CHECK(item_remove(*item));
    }
    else {
      CHECK(chunk_allocate_item((data_pointer *)item, &master->item_chunk));
    }
    target->count++;
    (*item)->data = (pointer)chunk_item;
  }
  else {
    ERROR(BAD_SIZE);
  }

  FINALLY(list_link_item);
  RETURN();
}

/******************************************************************************/

list *list_alloc()
{
  TRY();
  list *ptr;
  CHECK(memory_allocate((data_pointer *)&ptr, 1, sizeof(list)));
  CHECK(list_nullify(ptr));
  FINALLY(list_alloc);
  return ptr;
}

/******************************************************************************/

void list_free
(
  list *target
)
{
  TRY();
  /*printf("free list\n");*/
  r = SUCCESS;
  if (target != NULL) {
      CHECK(list_destroy(target));
      CHECK(memory_deallocate((data_pointer *)&target));
  }
  FINALLY(list_free);
}

/******************************************************************************/

result list_create
(
  list *target,
  uint32 max_size,
  uint32 item_size,
  uint32 link_rate
)
{
  TRY();

  CHECK_POINTER(target);

  /* link rate should be at least 1 */
  CHECK_PARAM(link_rate > 0);

  /* allocate chunks */
  CHECK(chunk_create(&target->item_chunk, max_size * link_rate, sizeof(list_item)));
  CHECK(chunk_create(&target->data_chunk, max_size, item_size));

  target->first.prev = NULL;
  target->first.next = &target->last;
  target->first.data = NULL;
  target->last.prev = &target->first;
  target->last.next = NULL;
  target->last.data = NULL;
  target->first_free.prev = NULL;
  target->first_free.next = &target->last_free;
  target->first_free.data = NULL;
  target->last_free.prev = &target->first_free;
  target->last_free.next = NULL;
  target->last_free.data = NULL;

  /* this becomes a master list because it owns the chunks */
  target->parent = NULL;
  target->count = 0;
  target->max_size = max_size;

  FINALLY(list_create);
  RETURN();
}

/******************************************************************************/

result list_create_from_data
(
  list *target,
  data_pointer data,
  uint32 max_size,
  uint32 item_size,
  uint32 link_rate
)
{
  TRY();

  CHECK_POINTER(target);

  /* link rate should be at least 1 */
  CHECK_PARAM(link_rate > 0);

  /* allocate item chunk */
  CHECK(chunk_create(&target->item_chunk, max_size * link_rate, sizeof(list_item)));

  /* instead of allocating a data chunk, appropriate the provided data array */
  target->data_chunk.chunk = data;
  target->data_chunk.size = max_size;
  target->data_chunk.item_size = item_size;
  target->data_chunk.count = max_size;

  target->first.prev = NULL;
  target->first.next = &target->last;
  target->first.data = NULL;
  target->last.prev = &target->first;
  target->last.next = NULL;
  target->last.data = NULL;
  target->first_free.prev = NULL;
  target->first_free.next = &target->last_free;
  target->first_free.data = NULL;
  target->last_free.prev = &target->first_free;
  target->last_free.next = NULL;
  target->last_free.data = NULL;

  /* this becomes a master list because it owns the chunks */
  target->parent = NULL;
  target->count = 0;
  target->max_size = max_size;

  FINALLY(list_create_from_data);
  RETURN();
}

/******************************************************************************/

result list_destroy
(
  list *target
)
{
  TRY();

  CHECK_POINTER(target);

  /* master list can be destroyed by destroying chunks */
  if (target->parent == NULL) {
      chunk_destroy(&target->item_chunk);
      chunk_destroy(&target->data_chunk);
      target->first.next = NULL;
      target->last.prev = NULL;
      target->first_free.next = NULL;
      target->last_free.prev = NULL;
  }
  /* sublist is destroyed by removing all items */
  else {
      /* TODO: implement destroying sublist */
      ERROR(NOT_IMPLEMENTED);
  }

  FINALLY(list_destroy);
  RETURN();
}

/******************************************************************************/

result list_nullify
(
  list *target
)
{
  TRY();

  CHECK_POINTER(target);

  target->parent = NULL;
  CHECK(list_item_nullify(&target->first));
  CHECK(list_item_nullify(&target->last));
  CHECK(list_item_nullify(&target->first_free));
  CHECK(list_item_nullify(&target->last_free));
  target->count = 0;
  target->max_size = 0;
  CHECK(chunk_nullify(&target->item_chunk));
  CHECK(chunk_nullify(&target->data_chunk));

  FINALLY(list_nullify);
  RETURN();
}

/******************************************************************************/

truth_value list_is_null
(
  list *target
)
{
  if (target != NULL) {
    if (chunk_is_null(&target->item_chunk) && chunk_is_null(&target->data_chunk)) {
      return TRUE;
    }
  }
  return FALSE;
}

/******************************************************************************/

result list_clear
(
  list *target
)
{
  TRY();
  list_item *i;
  list_item *next;

  CHECK_POINTER(target);

  if (target->parent == NULL) {
    CHECK(chunk_clear(&target->item_chunk));
    CHECK(chunk_clear(&target->data_chunk));
  }
  else {
    i = target->first.next;
    while (i != &target->last) {
      next = i->next;
      /* no need for explicit remove, just add to list of free items */
      CHECK(item_insert_before(&target->parent->last_free, i));
      i = next;
    }
  }

  target->first.prev = NULL;
  target->first.next = &target->last;
  target->first.data = NULL;
  target->last.prev = &target->first;
  target->last.next = NULL;
  target->last.data = NULL;
  target->first_free.prev = NULL;
  target->first_free.next = &target->last_free;
  target->first_free.data = NULL;
  target->last_free.prev = &target->first_free;
  target->last_free.next = NULL;
  target->last_free.data = NULL;

  target->count = 0;

  FINALLY(list_clear);
  RETURN();
}

/******************************************************************************/

result list_pack
(
  list *target
)
{
  TRY();

  CHECK_POINTER(target);
  ERROR(NOT_IMPLEMENTED);

  FINALLY(list_pack);
  RETURN();
}

/******************************************************************************/

result sublist_create
(
  list *target,
  list *source
)
{
  TRY();

  CHECK_POINTER(target);
  CHECK_POINTER(source);

  /* the src list may not be sublist itself, and it must have the chunks */
  CHECK_PARAM(source->parent == NULL && source->data_chunk.chunk != NULL && source->item_chunk.chunk != NULL);

  target->first.prev = NULL;
  target->first.next = &target->last;
  target->first.data = NULL;
  target->last.prev = &target->first;
  target->last.next = NULL;
  target->last.data = NULL;
  target->first_free.prev = NULL;
  target->first_free.next = &target->last_free;
  target->first_free.data = NULL;
  target->last_free.prev = &target->first_free;
  target->last_free.next = NULL;
  target->last_free.data = NULL;

  target->parent = source;
  target->count = 0;
  target->max_size = source->max_size;
  target->data_chunk.chunk = NULL;
  target->item_chunk.chunk = NULL;

  FINALLY(sublist_create);
  RETURN();
}

/******************************************************************************/

result list_append
(
  list *target,
  pointer data
)
{
  TRY();
  list_item *item;

  /* no need to check pointers, list_create_item does that */
  CHECK(list_create_item(target, &item, data));
  CHECK(item_insert_before(&target->last, item));

  FINALLY(list_append);
  RETURN();
}

/******************************************************************************/

result list_append_return_pointer
(
  list *target,
  pointer data,
  pointer *list_data
)
{
  TRY();
  list_item *item;

  *list_data = NULL;
  /* no need to check pointers, list_create_item does that */
  CHECK(list_create_item(target, &item, data));
  CHECK(item_insert_before(&target->last, item));
  *list_data = item->data;

  FINALLY(list_append_return_pointer);
  RETURN();
}

/******************************************************************************/

result list_append_index
(
  list *target,
  list_index index
)
{
  TRY();
  list_item *item;

  /* no need to check pointers, list_link_item does that */
  CHECK(list_link_item(target, &item, index));
  CHECK(item_insert_before(&target->last, item));

  FINALLY(list_append_index);
  RETURN();
}

/******************************************************************************/

result sublist_append
(
  list *target,
  pointer data
)
{
  TRY();
  list_item *item;

  CHECK_POINTER(target);

  /* for sublists, append first to the parent list */
  if (target->parent != NULL) {
    CHECK(list_create_item(target->parent, &item, data));
    CHECK(item_insert_before(&target->last, item));
  }
  /* for master lists, append directly to the list itself */
  else {
    CHECK(list_append(target, data));
  }

  FINALLY(sublist_append);
  RETURN();
}

/******************************************************************************/

result list_prepend
(
  list *target,
  pointer data
)
{
  TRY();
  list_item *item;

  /* no need to check pointers, list_create_item does that */
  CHECK(list_create_item(target, &item, data));
  CHECK(item_insert_after(&target->first, item));

  FINALLY(list_prepend);
  RETURN();
}

/******************************************************************************/

result list_prepend_index
(
  list *target,
  list_index index
)
{
  TRY();
  list_item *item;

  /* no need to check pointers, list_link_item does that */
  CHECK(list_link_item(target, &item, index));
  CHECK(item_insert_after(&target->first, item));

  FINALLY(list_prepend_index);
  RETURN();
}

/******************************************************************************/

result list_insert_at
(
  list *target,
  list_item *at,
  pointer data
)
{
  TRY();
  list_item *item;

  /* no need to check pointers, list_create_item does that */
  CHECK(list_create_item(target, &item, data));
  CHECK(item_insert_after(at, item));

  FINALLY(list_insert_at);
  RETURN();
}


/******************************************************************************/

result sublist_insert_at
(
  list *target,
  list_item *at,
  pointer data
)
{
  TRY();
  pointer new_data;

  CHECK_POINTER(target);

  /* for sublists, append first to the parent list */
  if (target->parent != NULL) {
    CHECK(list_append_return_pointer(target->parent, data, (pointer)&new_data));
    CHECK(list_insert_at(target, at, new_data));
  }
  /* for master lists, append directly to the list itself */
  else {
    CHECK(list_insert_at(target, at, data));
  }

  FINALLY(list_insert_at);
  RETURN();
}

/******************************************************************************/

result list_insert_sorted
(
  list *target,
  pointer data,
  list_item_comparator comparator
)
{
  TRY();
  list_item *item;
  list_item *i;

  CHECK(list_create_item(target, &item, data));

  i = target->first.next;
  while (i != &target->last) {
    if (i == NULL) {
      /* iteration finished without encountering the end item */
      ERROR(NOT_FOUND);
    }
    if (comparator(data, i->data) <= 0) {
      break;
    }
  }
  CHECK(item_insert_before(i, item));

  FINALLY(list_insert_sorted);
  RETURN();
}

/******************************************************************************/

result list_insert_sorted_index
(
  list *target,
  list_index index,
  list_item_comparator comparator
)
{
  TRY();
  list_item *item;
  list_item *i;

  CHECK(list_link_item(target, &item, index));

  i = target->first.next;
  while (i != &target->last) {
    if (i == NULL) {
      /* iteration finished without encountering the end item */
      ERROR(NOT_FOUND);
    }
    if (comparator(item->data, i->data) <= 0) {
      break;
    }
    i = i->next;
  }
  CHECK(item_insert_before(i, item));

  FINALLY(list_insert_sorted_index);
  RETURN();
}

/******************************************************************************/

result list_insert_unique
(
  list *target,
  pointer data,
  list_item_comparator comparator
)
{
  TRY();
  list_item *item;
  list_item *i;
  int comparison;

  i = target->first.next;
  while (i != &target->last) {
    if (i == NULL) {
      /* iteration finished without encountering the end item */
      ERROR(NOT_FOUND);
    }
    comparison = comparator(data, i->data);
    /* if equal item already exists, abort */
    if (comparison == 0) {
      TERMINATE(SUCCESS);
    }
    else
    /* otherwise, if larger item was found, insert before that */
    if (comparison < 0) {
      break;
    }
    i = i->next;
  }
  CHECK(list_create_item(target, &item, data));
  CHECK(item_insert_before(i, item));

  FINALLY(list_insert_unique);
  RETURN();
}

/******************************************************************************/

result list_insert_unique_index
(
  list *target,
  list_index index,
  list_item_comparator comparator
)
{
  TRY();
  byte *chunk_item;
  list *master;
  list_item *item;
  list_item *i;
  int comparison;

  master = target->parent;
  if (master == NULL) {
    ERROR(BAD_PARAM);
  }
  else {
    chunk_item = chunk_return_item(&master->data_chunk, index);

    i = target->first.next;
    while (i != &target->last) {
      if (i == NULL) {
        /* iteration finished without encountering the end item */
        ERROR(NOT_FOUND);
      }
      comparison = comparator(item->data, i->data);
      /* if equal item already exists, abort */
      if (comparison == 0) {
        TERMINATE(SUCCESS);
      }
      else
      /* otherwise, if larger item was found, insert before that */
      if (comparison < 0) {
        break;
      }
      i = i->next;
    }
    CHECK(list_link_item(master, &item, index));
    CHECK(item_insert_before(i, item));
  }
  FINALLY(list_insert_unique_index);
  RETURN();
}

/******************************************************************************/

result list_remove
(
  list *target,
  pointer data,
  list_item_indicator is_match
)
{
  TRY();
  list_item *i;

  CHECK_POINTER(target);
  CHECK_POINTER(data);

  i = target->first.next;
  while (i != &target->last) {
    if (i == NULL) {
      /* iteration finished without encountering the end item */
      ERROR(NOT_FOUND);
    }
    if (IS_TRUE(is_match(i->data, data))) {
      TERMINATE(list_remove_item(target, i));
    }
    i = i->next;
  }
  ERROR(NOT_FOUND);

  FINALLY(list_remove);
  RETURN();
}

/******************************************************************************/

result list_remove_item
(
  list *target,
  list_item *item
)
{
  TRY();
  list *master;
  truth_value is_master;

  CHECK_POINTER(target);
  CHECK_POINTER(item);
  CHECK_POINTER(item->prev);
  CHECK_POINTER(item->next);

  /* for sublists, use the parent list for allocating items */
  if (target->parent != NULL) {
    master = target->parent;
    is_master = FALSE;
  }
  /* for master lists, use the list itself */
  else {
    master = target;
    is_master = TRUE;
  }

  if (IS_TRUE(is_master)) {
    /* data for item removed from master list is filled with 0 */
    CHECK(memory_clear(item->data, target->data_chunk.item_size, 1));
  }
  CHECK(item_remove(item));
  target->count--;
  CHECK(item_insert_before(&master->last_free, item));

  FINALLY(list_remove_item);
  RETURN();
}

/******************************************************************************/

result list_remove_between
(
  list *target,
  list_item *start,
  list_item *end
)
{
  TRY();
  list_item *item;

  CHECK_POINTER(start);
  CHECK_POINTER(end);

  item = start->next;
  while (item != end) {
    if (item == NULL) {
      /* end item not found in sequence, starting from start item */
      ERROR(BAD_PARAM);
    }
    item = item->next;
  }

  while (start->next != end) {
    list_remove_item(target, start->next);
  }

  FINALLY(list_remove_between);
  RETURN();
}

/******************************************************************************/

result list_remove_rest
(
  list *target,
  list_item *last
)
{
  TRY();
  list_item *item;

  CHECK_POINTER(target);
  CHECK_POINTER(last);

  item = last;
  while (item != &target->last) {
    if (item == NULL) {
      /* end item not found in sequence, starting from given item */
      ERROR(BAD_PARAM);
    }
    item = item->next;
  }

  while (last->next != &target->last) {
    list_remove_item(target, last->next);
  }

  FINALLY(list_remove_rest);
  RETURN();
}

/******************************************************************************/

result list_iterate_forward
(
  const list_item *begin,
  const list_item *end,
  list_item_handler operation
)
{
  TRY();
  list_item *i;

  CHECK_POINTER(begin);
  CHECK_POINTER(end);

  i = begin->next;
  while (i != end) {
    if (i == NULL) {
      /* iteration finished without encountering the end item */
      ERROR(NOT_FOUND);
    }
    CHECK(operation(i));
    i = i->next;
  };

  FINALLY(list_iterate_forward);
  RETURN();
}

/******************************************************************************/

result list_iterate_backward
(
  const list_item *begin,
  const list_item *end,
  list_item_handler operation
)
{
  TRY();
  list_item *i;

  CHECK_POINTER(begin);
  CHECK_POINTER(end);

  i = begin->prev;
  while (i != end) {
    if (i == NULL) {
      /* iteration finished without encountering the end item */
      ERROR(NOT_FOUND);
    }
    CHECK(operation(i));
    i = i->prev;
  };

  FINALLY(list_iterate_backward);
  RETURN();
}

/******************************************************************************/

result pointer_list_create
(
  pointer_list *target,
  uint32 max_size,
  uint32 item_size,
  uint32 link_rate,
  uint32 sparsity
)
{
  TRY();

  CHECK_POINTER(target);

  /* link rate should be at least 1 */
  CHECK_PARAM(link_rate > 0);

  /* sparsity should be at least 1 */
  CHECK_PARAM(sparsity > 0);

  /* allocate chunks */
  CHECK(chunk_create(&target->pointer_chunk, max_size, sizeof(data_pointer)));
  target->pointer_chunk.count = max_size;

  CHECK(list_create(&target->data_list, (uint32)(max_size / sparsity), item_size, link_rate));

  target->parent = NULL;
  target->ptr = (data_pointer *)target->pointer_chunk.chunk;
  target->size = max_size;
  target->count = 0;

  FINALLY(pointer_list_create);
  RETURN();
}

/******************************************************************************/

result pointer_sublist_create
(
  pointer_list *target,
  pointer_list *source,
  list_index index,
  uint32 max_size
)
{
  TRY();
  list_item *item;
  uint32 i;

  CHECK_POINTER(target);
  CHECK_POINTER(source);
  CHECK_PARAM(index + max_size < source->pointer_chunk.size);

  CHECK(sublist_create(&target->data_list, &source->data_list));

  target->parent = source;
  target->ptr = ((data_pointer *)source->pointer_chunk.chunk) + index;
  target->size = max_size;
  target->pointer_chunk.chunk = NULL;
  target->data_list.data_chunk.chunk = NULL;
  target->data_list.item_chunk.chunk = NULL;

  for (i = 0; i < max_size; i++) {
    if (target->ptr[i] != NULL) {
      r = list_create_item(&target->data_list, &item, target->ptr[i]);
      item->data = &target->ptr[i];
      r = item_insert_before(&target->data_list.last, item);
      target->count++;
    }
  }

  FINALLY(pointer_sublist_create);
  RETURN();
}

/******************************************************************************/

result pointer_list_destroy
(
  pointer_list *target
)
{
  TRY();

  CHECK_POINTER(target);

  CHECK(list_destroy(&target->data_list));
  /* master list can be destroyed by destroying chunks */
  if (target->parent == NULL) {
    CHECK(chunk_destroy(&target->pointer_chunk));
    target->size = 0;
  }
  /* sublist is destroyed by removing all items */
  else {
    /* TODO: implement destroying sublist */
    ERROR(NOT_IMPLEMENTED);
  }

  FINALLY(pointer_list_destroy);
  RETURN();
}

/******************************************************************************/

result pointer_list_append
(
  pointer_list *target,
  list_index index,
  pointer data
)
{
  TRY();
  list_item *item;

  CHECK_POINTER(target);
  CHECK_POINTER(data);
  CHECK_PARAM(index < target->pointer_chunk.size);

  /* no need to check pointers, list_create_item does that */
  CHECK(list_create_item(&target->data_list, &item, data));

  /* add pointer to data to the pointer array at specified index */
  ((data_pointer *)target->pointer_chunk.chunk)[index] = (data_pointer)item->data;
  /* add to list a pointer to the pointer array element */
  item->data = &((data_pointer*)target->pointer_chunk.chunk)[index];

  CHECK(item_insert_before(&target->data_list.last, item));
  target->count++;

  FINALLY(pointer_list_append);
  RETURN();
}

/******************************************************************************/

result pointer_list_prepend
(
  pointer_list *target,
  list_index index,
  pointer data
)
{
  TRY();
  list_item *item;

  CHECK_POINTER(target);
  CHECK_POINTER(data);
  CHECK_PARAM(index < target->pointer_chunk.size);

  /* no need to check pointers, list_create_item does that */
  CHECK(list_create_item(&target->data_list, &item, data));

  /* add pointer to data to the pointer array at specified index */
  ((data_pointer *)target->pointer_chunk.chunk)[index] = (data_pointer)item->data;
  /* add to list a pointer to the pointer array element */
  item->data = &((data_pointer *)target->pointer_chunk.chunk)[index];

  CHECK(item_insert_after(&target->data_list.first, item));

  FINALLY(pointer_list_prepend);
  RETURN();
}

/* end of file                                                                */
/******************************************************************************/
