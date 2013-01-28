/**
 * @file cvsu_list.h
 * @author Matti J. Eskelinen <matti.j.eskelinen@gmail.com>
 * @brief A double-linked list that stores any object as void pointer.
 *
 * Copyright (c) 2011-2013, Matti Johannes Eskelinen
 * All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CVSU_LIST_H
#   define CVSU_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cvsu_types.h"

/**
 * The item stored in the list.
 */

typedef struct list_item_t {
    struct list_item_t *prev; /**< Link to previous item. */
    struct list_item_t *next; /**< Link to next item. */
    void *data;             /**< Pointer to data stored by the item. */
} list_item;

/**
 * A comparator function for list items.
 * The function must handle the possible NULL values.
 * The function must also cast the void pointers to appropriate values.

 * @param a A pointer to list item data
 * @param b A pointer to list item data
 * @return Positive value if (a > b), 0 if (a == b), negative value if (a < b)
*/

typedef int (*list_item_comparator)(const void *a, const void *b);

/**
 * An indicator function for equality of list items.
 * The function must handle the possible NULL values.
 * The function must also cast the void pointers to appropriate values.
 *
 * @param a A pointer to list item data
 * @param b A pointer to list item data
 * @return true if elements match, false otherwise
 */

typedef bool (*list_item_indicator)(const void *a, const void *b);

/**
 * A handler function for list iterators.
 * The function gets a const pointer to an item and does some operation on it.
 *
 * @param item A const pointer to a list item, given by iterator
 * @return SUCCESS is operation was succesful
 */

typedef result (*list_item_handler)(const list_item *item);

/**
 * A block of memory for storing equal-sized items.
 * Stores an array of items and allows dynamically activating and deactivating
 * items from it; acts like a buffer for dynamic memory allocation and
 * deallocation.
 *
 * Distributes items one by one, starting from the beginning of the array. When
 * items are 'freed', stores the pointers in a list so it can efficiently
 * re-distribute them.
 *
 * item_size  :
  size       : number of items available in array
  count      : number of items in use
  first_free : start of the list of freed items
  last_free  : end of the list of freed items
*/

typedef struct chunk_t {
  /** Size of one item in the array in bytes */
  size_t item_size;
  /** Total number of items available in a current chunk */
  size_t size;
  /** Number of items taken in use in current chunk (including freed items) */
  size_t count;
  /** Number of chunks currently allocated */
  size_t chunk_count;
  /** Index of the current chunk from which items are distributed */
  size_t current_chunk;
  /** Pointer to the array holding the chunk array */
  byte **chunks;
  /** Pointer to the current chunk holding the data */
  byte *chunk;
} chunk;

/**
 * Stores a double-linked list and a memory chunk for allocating the items.
 * It is also possible to create sub-lists that use the same chunks.
 */

typedef struct list_t {
    /** Parent list for sub-lists; NULL for the master list. */
    struct list_t *parent;
    /** Placeholder item for accessing the first item, not for storing data */
    list_item first;
    /** Placeholder item for accessing the last item, not for storing data */
    list_item last;
    /** Start of the list of freed items. */
    list_item first_free;
    /** End of the list of freed items. */
    list_item last_free;
    /** Number of items in the list */
    size_t count;
    /** Maximum number of items in the list */
    size_t max_size;
    /** Chunk for allocating the list items */
    chunk item_chunk;
    /** Chunk for allocating the data for list items */
    chunk data_chunk;
} list;

/**
 * Stores a double-linked list of pointers to data.
 * The pointer array is sparse by design, meaning that the pointers are stored
 * in an array, that has intentional empty slots, indicated by NULL pointers.
 */

typedef struct pointer_list_t {
    struct pointer_list_t *parent;
    list data_list;
    chunk pointer_chunk;
    data_pointer *ptr;
    size_t size;
    size_t count;
} pointer_list;

typedef unsigned long list_index;

/**
 * Allocates the data arrays for the chunk and initializes its structure.
 */

result chunk_create(
    chunk *target,
    size_t max_size,
    size_t item_size
);


/**
 * Deallocates the memory that was allocated for the chunk.
 */

result chunk_destroy(
    chunk *target
);

/**
 * Sets all variables to NULL, does not deallocate memory
 */

result chunk_nullify(
    chunk *target
);

/**
 * Everything that can be nullified should be able to tell if it's null
 */

bool chunk_is_null(
  chunk *target
);

/**
 * Clears the memory contained by the chunk
 */

result chunk_clear(
    chunk *target
);

/**
 * Allocates items from the chunk, providing a simulated dynamic heap.
 */

result chunk_allocate_item(
    data_pointer *target,
    chunk *source
);

/**
 * Gets an item from the chunk by index, and stores it in target.
 */

result chunk_get_item(
    data_pointer *target,
    chunk *source,
    list_index index
);

/**
 * Checks if a chunk contains an item or not.
 */

bool chunk_contains_item(
    chunk *source,
    data_pointer item
);

/**
 * Returns an existing item from the chunk, referenced by index.
 * Returns NULL if the index is invalid.
 */

data_pointer chunk_return_item(
    chunk *source,
    list_index index
);

/**
 * Sets all variables to NULL, does not deallocate memory.
 */

result list_item_nullify(
  list_item *target
);

/**
 * Everything that can be nullified should be able to tell if it's null
 */

bool list_item_is_null(
  list_item *target
);

/**
 * Allocates a list structure.
 */

list *list_alloc();

/**
 * Frees a list structure allocated with @see list_alloc
 */

void list_free(list *ptr);

/**
 * Creates a master list and allocates the chunks for it.
 * Allows to specify, how many links are reserved for each data item.
 * TODO: should allow optionally completely dynamic lists without using chunks
 *
 * @param dst List structure to be initialized.
 * @param max_size Maximum number of data items contained, size of data chunk.
 * @param item_size Size of data items stored in the list and data chunk.
 * @param link_rate Multiplier determining the size of list item chunk.
 * @returns SUCCESS if successful, BAD_POINTER if dst is NULL, BAD_PARAM if
 *          link_rate is 0.
 */

result list_create(
    list *target,
    size_t max_size,
    size_t item_size,
    size_t link_rate
);

/**
 * Creates a master list, using a pre-allocated array for data.
 * Items can be created only by linking to data items by array index.
 * Allows to specify, how many links are reserved for each data item.
 * This enables creating multiple sublists, that reference the same data items.
 */

result list_create_from_data(
    list *target,
    data_pointer data,
    size_t max_size,
    size_t item_size,
    size_t link_rate
);

/**
 * Destroys the list, deallocates the chunks if it is the master
 */

result list_destroy(
    list *target
);

/**
 * Sets all variables to null, does not deallocate memory.
 */

result list_nullify(
    list *target
);

/**
 * Everything that can be nullified should be able to tell if it's null
 */

bool list_is_null(
  list *target
);

/**
 * Clears the list and the contained data
 */

result list_clear(
    list *target
);

/**
 * Packs the list items in order into the chunk, so the list can be handled
 * like a normal array.
 */

result list_pack(
    list *target
);

/**
 * Creates a sublist, which uses the chunks from the master list.
 */

result sublist_create(
    list *target,
    list *source
);

/**
 * Appends data to the end of the list.
 * Copies the data to newly allocated item if list is a master list.
 * For sublists, uses the same item if data is within the data chunk.
 */

result list_append(
    list *target,
    pointer data
);

result list_append_reveal_data(
    list *target,
    pointer data,
    pointer *list_data
);

result sublist_append(
    list *target,
    pointer data
);

/**
 * Appends data to the end of the list.
 * Uses the item pointed to by list index.
 */

result list_append_index(
    list *target,
    list_index index
);

/**
 * Prepends data to the beginning of the list.
 * Copies the data to newly allocated item if list is a master list.
 * For sublists, uses the same item if data is within the data chunk.
 */

result list_prepend(
    list *target,
    pointer data
);

/**
 * Prepends data to the beginning of the list.
 * Uses the item pointed to by list index.
 */

result list_prepend_index(
    list *target,
    list_index index
);

result list_insert_at(
    list *target,
    list_item *at,
    pointer data
);

result sublist_insert_at(
    list *target,
    list_item *at,
    pointer data
);

/**
 * Inserts data to list in correct sorted order, determined by comparator.
 */

result list_insert_sorted(
    list *target,
    pointer data,
    list_item_comparator comparator
);

/**
 * Inserts data to list in correct sorted order, determined by comparator.
 * Uses the item pointed to by list index.
 */

result list_insert_sorted_index(
    list *target,
    list_index index,
    list_item_comparator comparator
);

/**
 * Finds and removes a data item from the list
 */

result list_remove(
    list *target,
    pointer data,
    list_item_indicator indicator
);

/**
 * Removes an item from the list
 */

result list_remove_item(
    list *target,
    list_item *item
);

result list_remove_between(
    list *target,
    list_item *start,
    list_item *end
);

result list_remove_rest(
    list *target,
    list_item *last
);

result list_remove_first(
    list *target,
    list_item *item
);

result list_remove_last(
    list *target,
    list_item *item
);

/**
 * Iterates through the list from begin to end in forward direction.
 * For each item, calls the operation provided as a function pointer.
 */

result list_iterate_forward(
    const list_item *begin,
    const list_item *end,
    list_item_handler operation
);

/**
 * Iterates through the list from begin to end in backward direction.
 * For each item, calls the operation provided as a function pointer.
 */

result list_iterate_backward(
    const list_item *begin,
    const list_item *end,
    list_item_handler operation
);

/**
 * Allocates a pointer list, which encloses a normal list and puts pointers
 * between list items and data.
 */

result pointer_list_create(
    pointer_list *target,
    size_t max_size,
    size_t item_size,
    size_t link_rate,
    size_t sparsity
);

/**
 * Creates a sublist to a pointer list.
 */

result pointer_sublist_create(
    pointer_list *target,
    pointer_list *source,
    list_index index,
    size_t max_size
);

/**
 * Destroys a pointer list and deallocates the memory.
 */

result pointer_list_destroy(
    pointer_list *target
);

/**
 * Appends data to the end of a pointer list.
 */

result pointer_list_append(
    pointer_list *target,
    list_index index,
    pointer data
);

/**
 * Prepends data to the beginning of a pointer list.
 */

result pointer_list_prepend(
    pointer_list *target,
    list_index index,
    pointer data
);

#ifdef __cplusplus
}
#endif

#endif /* CVSU_LIST_H */
