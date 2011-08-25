/**
 * @file list.h
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief A double-linked list that stores any object as void pointer.
 *
 * Copyright (c) 2011, Matti Johannes Eskelinen
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

#ifndef LIST_H
#   define LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

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
    /** Size of one item in the array in bytes. */
    size_t item_size;
    /** Total number of items available in the array. */
    size_t size;
    /** Number of items taken in use (including freed items). */
    size_t count;
    /** Pointer to the data array. */
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
    list_item *first;
    /** Placeholder item for accessing the last item, not for storing data */
    list_item *last;
    /** Start of the list of freed items. */
    list_item *first_free;
    /** End of the list of freed items. */
    list_item *last_free;
    /** Chunk for allocating the list items */
    chunk item_chunk;
    /** Chunk for allocating the data for list items */
    chunk data_chunk;
} list;

/**
 * Allocates the data arrays for the chunk and initializes its structure.
 */
result chunk_allocate(chunk *dst, size_t max_size, size_t item_size);

/**
 * Deallocates the memory that was allocated for the chunk.
 */
result chunk_destroy(chunk *dst);

/**
 * Clears the memory contained by the chunk
 */
result chunk_clear(chunk *dst);

/**
 * Allocates items from the chunk, providing a simulated dynamic heap.
 */
result chunk_allocate_item(byte **dst, chunk *src);

/**
 * Deallocates an item from the chunk, adding it to list of free items.
 */
result chunk_deallocate_item(byte *dst, chunk *src);

/**
 * Creates a master list and allocates the chunks for it
 *
 * TODO: should allow optionally completely dynamic lists without using chunks
 */

result list_allocate(list *dst, size_t max_size, size_t item_size);

/**
 * Destroys the list, deallocates the chunks if it is the master
*/

result list_destroy(list *dst);

/**
 * Clears the list and the contained data
 */
result list_clear(list *dst);

/**
 * Packs the list items in order into the chunk, so the list can be handled
 * like a normal array.
 */
result list_pack(list *dst);

/**
 * Creates a sublist, which uses the chunks from the master list.
 */
result sublist_create(list *dst, list *src);

/**
 * Creates a new item for the list, takes it from the chunk or free item list
 */
result list_create_item(list *dst, list_item **item, void *data);

/**
 * Creates a new item for a sublist, does not allocate data, uses the same
 */
result list_create_sublist_item(list *dst, list_item **item, void *data);

/**
 * Removes an item from list, and adds it to the free item list
 */
result list_remove_item(list *dst, list_item *item);

/**
 * Inserts 'prev' before 'item' in the list.
 * This cannot be done if 'prev' pointer of 'item' is not set; usually this is
 * the case if 'item' is the 'first' item of the list.
 */
result item_insert_before(list_item *item, list_item *prev);

/**
 * Inserts 'next' after 'item' in the list.
 * This cannot be done if 'next' pointer of 'item' is not set; usually this is
 * the case if 'item' is the 'last' item of the list.
 */
result item_insert_after(list_item *item, list_item *next);

/**
 * Removes 'item' from the list.
 * This cannot be done if 'prev' or 'next' pointer of 'item' is not set; usually
 * this is the case if 'item' is the 'first' or 'last' item of the list.
 */
result item_remove(list_item *item);

/**
 * Appends data to the end of the list.
 * Allocates the data for the item from the list data chunk if *data == NULL.
 */
result list_append(list *dst, void *data);

/**
 * Prepends data to the beginning of the list.
 * Allocates the data for the item from the list data chunk if *data == NULL.
 */
result list_prepend(list *dst, void *data);

/**
 * Finds and removes a data item from the list
 */
result list_remove(list *dst, void *data, list_item_indicator indicator);

/**
 * Iterates through the list from begin to end in forward direction.
 * For each item, calls the operation provided as a function pointer.
 */
result list_iterate_forward(list_item *begin, list_item *end, list_item_handler operation);

/**
 * Iterates through the list from begin to end in backward direction.
 * For each item, calls the operation provided as a function pointer.
 */
result list_iterate_backward(list_item *begin, list_item *end, list_item_handler operation);

/**
 * Appends to sublist data already contained in master list.
 */
result sublist_append(list *dst, list_item *src);

#ifdef __cplusplus
}
#endif

#endif // LIST_H
