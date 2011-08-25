/**
 * @file list.c
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

#include "list.h"
#include "alloc.h"

#include <string.h>

#include <stdio.h>

result chunk_allocate(chunk *dst, size_t max_size, size_t item_size)
{
    //printf("chunk_allocate 1\n");
    result r;
    if (dst == NULL) {
        //printf("chunk_allocate dst == NULL\n");
        return BAD_POINTER;
    }
    //printf("chunk_allocate 2\n");
    r = allocate(&dst->chunk, max_size, item_size);
    if (r != SUCCESS) {
        //printf("chunk_allocate allocate result=%d\n", (int)r);
        return r;
    }
    //printf("chunk_allocate 3\n");
    r = reset(dst->chunk, max_size, item_size);
    if (r != SUCCESS) {
        //printf("chunk_allocate reset result=%d\n", (int)r);
        return r;
    }
    //printf("chunk_allocate 4\n");
    dst->item_size = item_size;
    dst->size = max_size;
    dst->count = 0;

    //printf("chunk_allocate SUCCESS\n");
    return SUCCESS;
}

result chunk_destroy(chunk *dst)
{
    result r;
    if (dst == NULL) {
        return BAD_POINTER;
    }
    r = deallocate(&dst->chunk);
    if (r != SUCCESS) {
        return r;
    }

    dst->item_size = 0;
    dst->size = 0;
    dst->count = 0;

    return SUCCESS;
}

result chunk_clear(chunk *dst)
{
    if (dst == NULL) {
        return BAD_POINTER;
    }
    dst->count = 0;
    return reset(dst->chunk, dst->size, dst->item_size);
}

result chunk_allocate_item(byte **dst, chunk *src)
{
    //printf("chunk_allocate_item 1\n");
    if (src == NULL || dst == NULL) {
        //printf("chunk_allocate_item src == NULL || dst == NULL\n");
        return BAD_POINTER;
    }
    //printf("chunk_allocate_item 2\n");
    if (src->chunk == NULL) {
        //printf("chunk_allocate_item chunk == NULL\n");
        return BAD_POINTER;
    }
    //printf("chunk_allocate_item dst=NULL\n");
    *dst = NULL;
    src->count++;
    if (src->count >= src->size) {
        return BAD_SIZE;
    }
    else {
        //printf("chunk_allocate_item set dst\n");
        *dst = src->chunk + src->count * src->item_size;
    }
    return SUCCESS;
}

result chunk_deallocate_item(byte *dst, chunk *src)
{

    return SUCCESS;
}

result list_allocate(list *dst, size_t max_size, size_t item_size)
{
    result r;

    if (dst == NULL) {
        return BAD_POINTER;
    }

    /* allocate chunks */
    r = chunk_allocate(&dst->item_chunk, max_size + 6, sizeof(list_item));
    if (r != SUCCESS) {
        return r;
    }
    r = chunk_allocate(&dst->data_chunk, max_size, item_size);
    if (r != SUCCESS) {
        return r;
    }

    r = chunk_allocate_item((byte **)&dst->first, &dst->item_chunk);
    if (r != SUCCESS) {
        return r;
    }
    r = chunk_allocate_item((byte **)&dst->last, &dst->item_chunk);
    if (r != SUCCESS) {
        return r;
    }
    r = chunk_allocate_item((byte **)&dst->first_free, &dst->item_chunk);
    if (r != SUCCESS) {
        return r;
    }
    r = chunk_allocate_item((byte **)&dst->last_free, &dst->item_chunk);
    if (r != SUCCESS) {
        return r;
    }

    /* set pointers; chunk_allocate resets all other values */
    dst->first->next = dst->last;
    dst->last->prev = dst->first;
    dst->first_free->next = dst->last_free;
    dst->last_free->prev = dst->first_free;

    /* this becomes a master list because it owns the chunks */
    dst->parent = NULL;

    return SUCCESS;
}

result list_destroy(list *dst)
{
    if (dst == NULL) {
        return BAD_POINTER;
    }
    /* master list can be destroyed by destroying chunks */
    if (dst->parent == NULL) {
        chunk_destroy(&dst->item_chunk);
        chunk_destroy(&dst->data_chunk);
        dst->first = NULL;
        dst->last = NULL;
        dst->first_free = NULL;
        dst->last_free = NULL;
    }
    return SUCCESS;
}

result list_clear(list *dst)
{
    result r;
    if (dst == NULL) {
        return BAD_POINTER;
    }
    r = chunk_clear(&dst->item_chunk);
    if (r != SUCCESS) {
        return r;
    }
    r = chunk_clear(&dst->data_chunk);
    if (r != SUCCESS) {
        return r;
    }
    r = chunk_allocate_item((byte **)&dst->first, &dst->item_chunk);
    if (r != SUCCESS) {
        return r;
    }
    r = chunk_allocate_item((byte **)&dst->last, &dst->item_chunk);
    if (r != SUCCESS) {
        return r;
    }
    r = chunk_allocate_item((byte **)&dst->first_free, &dst->item_chunk);
    if (r != SUCCESS) {
        return r;
    }
    r = chunk_allocate_item((byte **)&dst->last_free, &dst->item_chunk);
    if (r != SUCCESS) {
        return r;
    }

    /* set pointers; chunk_allocate resets all other values */
    dst->first->next = dst->last;
    dst->last->prev = dst->first;
    dst->first_free->next = dst->last_free;
    dst->last_free->prev = dst->first_free;

    return SUCCESS;
}

result list_pack(list *dst)
{
    return SUCCESS;
}

result sublist_create(list *dst, list *src)
{
    result r;
    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    /* the src list may not be sublist itself, and it must have the chunks */
    if (src->parent != NULL || src->data_chunk.chunk == NULL || src->item_chunk.chunk == NULL) {
        return BAD_PARAM;
    }
    r = chunk_allocate_item((byte **)&dst->first, &src->item_chunk);
    if (r != SUCCESS) {
        return r;
    }
    r = chunk_allocate_item((byte **)&dst->last, &src->item_chunk);
    if (r != SUCCESS) {
        return r;
    }

    dst->first->next = dst->last;
    dst->last->prev = dst->first;

    dst->parent = src;
    dst->data_chunk.chunk = NULL;
    dst->item_chunk.chunk = NULL;
    dst->first_free = NULL;
    dst->last_free = NULL;

    return SUCCESS;
}

result list_create_item(list *dst, list_item **item, void *data)
{
    result r;
    if (dst == NULL || item == NULL) {
        return BAD_POINTER;
    }
    /* get item from free item list, or allocate if no free items available */
    if (dst->first_free->next != dst->last_free) {
        *item = dst->first_free->next;
        r = item_remove(*item);
        if (r != SUCCESS) {
            return r;
        }
    }
    else {
        r = chunk_allocate_item((byte **)item, &dst->item_chunk);
        if (r != SUCCESS) {
            return r;
        }
        /* allocate data for item */
        r = chunk_allocate_item((byte **)&(*item)->data, &dst->data_chunk);
        if (r != SUCCESS) {
            return r;
        }
    }
    memcpy((*item)->data, data, (*dst).data_chunk.item_size);
    return SUCCESS;
}

result list_create_sublist_item(list *dst, list_item **item, void *data)
{
    result r;
    if (dst == NULL || item == NULL) {
        return BAD_POINTER;
    }
    /* get item from free item list, or allocate if no free items available */
    if (dst->first_free->next != dst->last_free) {
        *item = dst->first_free->next;
        r = item_remove(*item);
        if (r != SUCCESS) {
            return r;
        }
    }
    else {
        r = chunk_allocate_item((byte **)item, &dst->item_chunk);
        if (r != SUCCESS) {
            return r;
        }
    }
    (*item)->data = data;

    return SUCCESS;
}

result list_remove_item(list *dst, list_item *item)
{
    result r;
    list_item *next, *prev;

    if (dst == NULL || item == NULL) {
        return BAD_POINTER;
    }
    if (item->prev == NULL || item->next == NULL) {
        return BAD_POINTER;
    }

    reset(item->data, (*dst).data_chunk.item_size, 1);
    r = item_remove(item);
    if (r != SUCCESS) {
        return r;
    }
    //next = item->next;
    //prev = item->prev;
    r = item_insert_before(dst->last_free, item);
    if (r != SUCCESS) {
        //item->next = next;
        //item->prev = prev;
        return r;
    }

    return SUCCESS;
}

result item_insert_before(list_item *item, list_item *prev)
{
    if (item == NULL || prev == NULL) {
        return BAD_POINTER;
    }
    if (item->prev == NULL) {
        return BAD_POINTER;
    }

    item->prev->next = prev;
    prev->prev = item->prev;
    item->prev = prev;
    prev->next = item;

    return SUCCESS;
}

result item_insert_after(list_item *item, list_item *next)
{
    //printf("item_insert_after item=0x%x next=ox%x\n", item, next);
    if (item == NULL || next == NULL) {
        return BAD_POINTER;
    }
    if (item->next == NULL) {
        //printf("item->next not set");
        return BAD_POINTER;
    }

    next->prev = item;
    next->next = item->next;
    item->next->prev = next;
    item->next = next;

    return SUCCESS;
}

result item_remove(list_item *item)
{
    if (item == NULL) {
        return BAD_POINTER;
    }
    if (item->prev == NULL || item->next == NULL) {
        return BAD_POINTER;
    }

    item->prev->next = item->next;
    item->next->prev = item->prev;
    item->next = NULL;
    item->prev = NULL;

    return SUCCESS;
}

result list_append(list *dst, void *data)
{
    //printf("list_append\n");
    result r;
    list_item *item;
    /* no need to check pointers, list_create_item does that */
    r = list_create_item(dst, &item, data);
    //printf("list_create_item result=%d\n", (int)r);
    if (r != SUCCESS) {
        return r;
    }
    r = item_insert_before(dst->last, item);
    //printf("item_insert_before result=%d\n", (int)r);
    if (r != SUCCESS) {
        return r;
    }
    //printf("list_append SUCCESS\n");
    return SUCCESS;
}

result list_prepend(list *dst, void *data)
{
    result r;
    list_item *item;
    /* no need to check pointers, list_create_item does that */
    r = list_create_item(dst, &item, data);
    if (r != SUCCESS) {
        return r;
    }
    r = item_insert_after(dst->first, item);
    if (r != SUCCESS) {
        return r;
    }
    return SUCCESS;
}

result list_remove(list *dst, void *data, list_item_indicator is_match)
{
    list_item *i;
    if (dst == NULL || data == NULL) {
        return BAD_POINTER;
    }

    i = dst->first->next;
    while (i != dst->last && i != NULL) {
        if (is_match(i->data, data)) {
            return list_remove_item(dst, i);
        }
        i = i->next;
    }
    return NOT_FOUND;
}

result list_iterate_forward(list_item *begin, list_item *end, list_item_handler operation)
{
    result r;
    list_item *i;
    if (begin == NULL) {
        return BAD_POINTER;
    }

    i = begin->next;
    while (i != end && i != NULL) {
        r = operation(i);
        if (r != SUCCESS) {
            return r;
        }
        i = i->next;
    };
    return SUCCESS;
}

result list_iterate_backward(list_item *begin, list_item *end, list_item_handler operation)
{
    return SUCCESS;
}

result sublist_append(list *dst, list_item *src)
{
    result r;
    list_item *item;

    if (src == NULL || dst == NULL) {
        return BAD_POINTER;
    }
    if (src->data == NULL) {
        return BAD_PARAM;
    }

    /* no need to check pointers, list_create_sublist_item does that */
    r = list_create_sublist_item(dst->parent, &item, src->data);
    if (r != SUCCESS) {
        return r;
    }
    r = item_insert_before(dst->last, item);
    if (r != SUCCESS) {
        return r;
    }

    return SUCCESS;
}
