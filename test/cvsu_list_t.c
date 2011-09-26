/**
 * @file list_t.c
 * @author Matti Eskelinen (matti dot j dot eskelinen at jyu dot fi)
 * @brief Testing code for list.c
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

#include "cvsu_list.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int static_data[] = { 3, 6, 5, 7, 1, 9, 8, 2, 4 };

void print_array(int *array, size_t size)
{
    int i;
    printf("Array: [ ");
    for (i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("]\n");
}

void print_pointer_array(int **array, size_t size)
{
    int i;
    int *data;
    printf("Pointer Array: [ ");
    for (i = 0; i < size; i++) {
        data = array[i];
        if (data == NULL) {
            printf("0 ");
        }
        else {
            printf("->%d ", *data);
        }
    }
    printf("]\n");
}

result print_item(const list_item *item)
{
    if (item == NULL || item->data == NULL) {
        return BAD_POINTER;
    }
    printf("%d ", *(int *)item->data);
    return SUCCESS;
}

result print_pointer_item(const list_item *item)
{
    //printf("print_pointer_item i\n");
    int *data;
    if (item == NULL || item->data == NULL) {
        return BAD_POINTER;
    }
    data = *((int **)item->data);
    //printf("0x%x 0x%x ", item->data, data);
    if (data == NULL) {
        printf("0 ");
    }
    else {
        printf("->%d ", *data);
    }

    //printf("print_pointer_item o\n");
    return SUCCESS;
}

result print_list(const list *l)
{
    result r;
    printf("List: ( ");
    r = list_iterate_forward(&l->first, &l->last, &print_item);
    printf(") ( ");
    r = list_iterate_forward(&l->first_free, &l->last_free, &print_item);
    printf(")\n");
    return r;
}

result print_pointer_list(const pointer_list *l)
{
    result r;
    printf("Pointer List: ( ");
    r = list_iterate_forward(&l->data_list.first, &l->data_list.last, &print_pointer_item);
    printf(") ( ");
    //r = list_iterate_forward(&l->data_list.first_free, &l->data_list.last_free, &print_pointer_item);
    printf(")\n");
    return r;
}

bool match_item(const void *a, const void *b)
{
    /*
    int la, lb;
    bool r;
    la = *(int *)a;
    lb = *(int *)b;
    printf("match_item a=%d b=%d\n", la, lb);
    if (la == lb) r = 1; else r = 0;
    //r = (la == lb);//(*(long *)a == *(long *)b);
    printf("match_item result=%d\n", (int)r);
    return r;
    */
    return (*(int *)a == *(int *)b);
}

int compare_item(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int main(int argc, char *argv[])
{
    printf("Starting list tests\n");

    result r;
    list test_list;
    list sub_list;
    list sub_list_2;
    pointer_list ptr_list;
    pointer_list ptr_sublist_1;
    pointer_list ptr_sublist_2;
    pointer_list ptr_sublist_3;
    int i;
    int value;
    int *test_data = (int *)malloc(10 * sizeof(int));
    memcpy(test_data, static_data, 10 * sizeof(int));

    int **pointer_data = (int **)malloc(10 * sizeof(int *));
    memset(pointer_data, 0, 10 * sizeof(int *));
/*
    r = list_create(&test_list, 3, sizeof(int), 3);
    printf("list_allocate result=%d\n", (int)r);
    r = sublist_create(&sub_list, &test_list);
    printf("sublist_create result=%d\n", (int)r);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n", test_list.item_chunk.count);
    r = sublist_create(&sub_list_2, &test_list);
    printf("sublist_create result=%d\n", (int)r);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 1;
    r = list_append(&test_list, &value);
    printf("list_append result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 2;
    r = list_append(&test_list, &value);
    printf("list_append result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 3;
    r = list_prepend(&test_list, &value);
    printf("list_prepend result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 4;
    r = list_append(&test_list, &value);
    printf("list_append result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 1;
    r = list_remove(&test_list, &value, &match_item);
    printf("list_remove result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_append(&test_list, &value);
    printf("list_append result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_insert_sorted_index(&sub_list, 0, &compare_item);
    printf("list_insert_sorted_index result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_insert_sorted_index(&sub_list, 1, &compare_item);
    printf("list_insert_sorted_index result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_insert_sorted_index(&sub_list, 2, &compare_item);
    printf("list_insert_sorted_index result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_insert_sorted_index(&sub_list, 0, &compare_item);
    printf("list_insert_sorted_index result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_append_index(&sub_list_2, 1);
    printf("list_append_index result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list_2);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_append_index(&sub_list_2, 0);
    printf("list_append_index result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list_2);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_append_index(&sub_list_2, 2);
    printf("list_append_index result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list_2);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_append_index(&sub_list_2, 1);
    printf("list_append_index result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list_2);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 3;
    r = list_remove(&test_list, &value, &match_item);
    printf("list_remove result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list_2);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 2;
    r = list_remove(&test_list, &value, &match_item);
    printf("list_remove result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list_2);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 1;
    r = list_remove(&test_list, &value, &match_item);
    printf("list_remove result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list_2);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 4;
    r = list_remove(&test_list, &value, &match_item);
    printf("list_remove result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list_2);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_clear(&sub_list);
    printf("list_clear result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list_2);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_clear(&sub_list_2);
    printf("list_clear result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list);
    printf("print_list result=%d\n", (int)r);
    r = print_list(&sub_list_2);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_destroy(&test_list);
    printf("list_destroy result=%d\n\n", (int)r);

    r = list_create_from_data(&test_list, (byte *)test_data, 10, sizeof(int), 3);
    printf("list_create result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 4;
    r = list_append(&test_list, &value);
    printf("list_append result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    for (i = 0; i < test_list.data_chunk.size; i++) {
        r = list_insert_sorted_index(&test_list, i, &compare_item);
        if (r != SUCCESS) {
            break;
        }
    }
    printf("list_sort result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    value = 4;
    r = list_append(&test_list, &value);
    printf("list_append result=%d\n", (int)r);
    r = print_list(&test_list);
    printf("print_list result=%d\n", (int)r);
    print_array((int *)test_list.data_chunk.chunk, test_list.data_chunk.size);
    printf("data chunk count = %d\n", test_list.data_chunk.count);
    printf("item chunk count = %d\n\n", test_list.item_chunk.count);

    r = list_destroy(&test_list);
    printf("list_destroy result=%d\n", (int)r);
*/
    r = pointer_list_create(&ptr_list, 10, sizeof(int), 3, 3);
    printf("pointer_list_allocate result=%d\n", (int)r);
    r = print_pointer_list(&ptr_list);
    printf("print_pointer_list result=%d\n", (int)r);
    print_pointer_array((int **)ptr_list.pointer_chunk.chunk, ptr_list.pointer_chunk.size);
    printf("pointer chunk count = %d/%d\n", ptr_list.pointer_chunk.count, ptr_list.pointer_chunk.size);
    printf("data chunk count = %d/%d\n", ptr_list.data_list.data_chunk.count, ptr_list.data_list.data_chunk.size);
    printf("item chunk count = %d/%d\n\n", ptr_list.data_list.item_chunk.count, ptr_list.data_list.item_chunk.size);

    value = 2;
    pointer_list_append(&ptr_list, 1, &value);
    printf("pointer_list_append result=%d\n", (int)r);
    r = print_pointer_list(&ptr_list);
    printf("print_pointer_list result=%d\n", (int)r);
    print_pointer_array((int **)ptr_list.pointer_chunk.chunk, ptr_list.pointer_chunk.size);
    printf("pointer chunk count = %d/%d\n", ptr_list.pointer_chunk.count, ptr_list.pointer_chunk.size);
    printf("data chunk count = %d/%d\n", ptr_list.data_list.data_chunk.count, ptr_list.data_list.data_chunk.size);
    printf("item chunk count = %d/%d\n\n", ptr_list.data_list.item_chunk.count, ptr_list.data_list.item_chunk.size);

    value = 1;
    pointer_list_append(&ptr_list, 4, &value);
    printf("pointer_list_append result=%d\n", (int)r);
    r = print_pointer_list(&ptr_list);
    printf("print_pointer_list result=%d\n", (int)r);
    print_pointer_array((int **)ptr_list.pointer_chunk.chunk, ptr_list.pointer_chunk.size);
    printf("pointer chunk count = %d/%d\n", ptr_list.pointer_chunk.count, ptr_list.pointer_chunk.size);
    printf("data chunk count = %d/%d\n", ptr_list.data_list.data_chunk.count, ptr_list.data_list.data_chunk.size);
    printf("item chunk count = %d/%d\n\n", ptr_list.data_list.item_chunk.count, ptr_list.data_list.item_chunk.size);

    value = 3;
    pointer_list_append(&ptr_list, 7, &value);
    printf("pointer_list_append result=%d\n", (int)r);
    r = print_pointer_list(&ptr_list);
    printf("print_pointer_list result=%d\n", (int)r);
    print_pointer_array((int **)ptr_list.pointer_chunk.chunk, ptr_list.pointer_chunk.size);
    printf("pointer chunk count = %d/%d\n", ptr_list.pointer_chunk.count, ptr_list.pointer_chunk.size);
    printf("data chunk count = %d/%d\n", ptr_list.data_list.data_chunk.count, ptr_list.data_list.data_chunk.size);
    printf("item chunk count = %d/%d\n\n", ptr_list.data_list.item_chunk.count, ptr_list.data_list.item_chunk.size);

    r = pointer_sublist_create(&ptr_sublist_1, &ptr_list, 0, 3);
    printf("pointer_sublist_create result=%d\n", (int)r);
    r = print_pointer_list(&ptr_sublist_1);
    printf("print_pointer_list result=%d\n", (int)r);
    print_pointer_array((int **)ptr_sublist_1.ptr, ptr_sublist_1.size);
    printf("pointer chunk count = %d/%d\n", ptr_list.pointer_chunk.count, ptr_list.pointer_chunk.size);
    printf("data chunk count = %d/%d\n", ptr_list.data_list.data_chunk.count, ptr_list.data_list.data_chunk.size);
    printf("item chunk count = %d/%d\n\n", ptr_list.data_list.item_chunk.count, ptr_list.data_list.item_chunk.size);

    r = pointer_sublist_create(&ptr_sublist_2, &ptr_list, 3, 3);
    printf("pointer_sublist_create result=%d\n", (int)r);
    r = print_pointer_list(&ptr_sublist_2);
    printf("print_pointer_list result=%d\n", (int)r);
    print_pointer_array((int **)ptr_sublist_2.ptr, ptr_sublist_2.size);
    printf("pointer chunk count = %d/%d\n", ptr_list.pointer_chunk.count, ptr_list.pointer_chunk.size);
    printf("data chunk count = %d/%d\n", ptr_list.data_list.data_chunk.count, ptr_list.data_list.data_chunk.size);
    printf("item chunk count = %d/%d\n\n", ptr_list.data_list.item_chunk.count, ptr_list.data_list.item_chunk.size);

    r = pointer_sublist_create(&ptr_sublist_3, &ptr_list, 6, 3);
    printf("pointer_sublist_create result=%d\n", (int)r);
    r = print_pointer_list(&ptr_sublist_3);
    printf("print_pointer_list result=%d\n", (int)r);
    print_pointer_array((int **)ptr_sublist_3.ptr, ptr_sublist_3.size);
    printf("pointer chunk count = %d/%d\n", ptr_list.pointer_chunk.count, ptr_list.pointer_chunk.size);
    printf("data chunk count = %d/%d\n", ptr_list.data_list.data_chunk.count, ptr_list.data_list.data_chunk.size);
    printf("item chunk count = %d/%d\n\n", ptr_list.data_list.item_chunk.count, ptr_list.data_list.item_chunk.size);

    r = pointer_list_destroy(&ptr_list);
    printf("pointer_list_destroy result=%d\n", (int)r);

    free(test_data);
    free(pointer_data);

}
