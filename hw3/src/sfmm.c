/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include "errno.h"

static sf_size_t min = 32;
static sf_block *pro;
static sf_block *epi;
static sf_size_t in_qklst = 0x00000001;
static sf_size_t prv_alloc = 0x00000002;
static sf_size_t alloc = 0x00000004;
static int total_payload = 0;
static int max_payload = 0;
static double total_block_size = 0;
static double total_pages = 0;

int sf_initialize();
void set_header(sf_block *block, sf_header value);
void set_entire_header(sf_block *block, sf_size_t paylod_size, sf_size_t block_size,
                       sf_size_t is_alloc, sf_size_t is_prv_alloc, sf_size_t is_in_qklst, int crete);
void set_payload_size(sf_block *block, sf_size_t payload_size);
void set_block_size(sf_block *block, sf_size_t block_size);
void set_alloc(sf_block *block, sf_size_t is_alloc);
void set_prv_alloc(sf_block *block, sf_size_t is_prv_allc);
void set_in_qklst(sf_block *block, sf_size_t is_qklst);
sf_size_t get_payload_size(sf_header header);
sf_size_t get_block_size(sf_header header);
sf_size_t get_alloc(sf_header header);
sf_size_t get_prv_alloc(sf_header header);
sf_size_t get_in_qklst(sf_header header);
void put_block(sf_block *block);
sf_size_t get_min_size(sf_size_t size);
sf_size_t get_index(sf_size_t size);
sf_size_t get_size_of_index(sf_size_t index);
sf_block *split_block(sf_block *block, size_t size);
void valid_pointer(sf_block *pp);
void release_block(sf_block *block);
sf_block *get_prev_block(sf_block *block);
sf_block *get_next_block(sf_block *block);
void remove_list(sf_block *block);
sf_size_t get_pow(sf_size_t pow);
void add_payload(int size);

void *sf_malloc(sf_size_t size)
{
    if (size == 0)
        return NULL;
    if (sf_mem_start() == sf_mem_end())
        if (sf_initialize())
            return NULL;
    sf_size_t min_size = get_min_size(size);

    sf_size_t index = (min_size - 32) / 16;
    if (index < 10)
        if (sf_quick_lists[index].length > 0)
        {
            sf_block *block = sf_quick_lists[index].first;
            sf_quick_lists[index].length -= 1;
            sf_quick_lists[index].first = block->body.links.next;
            set_entire_header(block, size, get_block_size(block->header), 1, get_prv_alloc(block->header), 0, 0);
            total_block_size += get_block_size(block->header);
            add_payload(size);
            return block->body.payload;
        }

    index = get_index(min_size);

    for (size_t i = index; i < NUM_FREE_LISTS; i++)
    {
        sf_block *header = &sf_free_list_heads[i];
        sf_block *ptr = header->body.links.next;
        while (ptr != header)
        {
            if (get_block_size(ptr->header) - min_size >= min)
            {
                remove_list(ptr);
                sf_block *block = split_block(ptr, min_size);
                sf_size_t block_size = get_block_size(block->header);
                set_entire_header(ptr, size, min_size, 1, get_prv_alloc(ptr->header), 0, 0);
                set_entire_header(block, 0, block_size, 0, get_prv_alloc(block->header), 0, 0);
                put_block(block);
                total_block_size += get_block_size(ptr->header);
                add_payload(size);
                return ptr->body.payload;
            }
            else if (get_block_size(ptr->header) >= min_size)
            {
                set_entire_header(ptr, size, get_block_size(ptr->header), 1, get_prv_alloc(ptr->header), 0, 0);
                total_block_size += get_block_size(ptr->header);
                add_payload(size);
                return ptr->body.payload;
            }
            ptr = ptr->body.links.next;
        }
    }

    sf_block *last = get_prev_block(epi);
    if (get_alloc(last->header) == 1)
        last = epi;
    while (get_block_size(last->header) < min_size)
    {
        if (sf_mem_grow() == NULL)
        {
            sf_errno = ENOMEM;
            return NULL;
        }
        total_pages += 1024;
        if (last->body.links.next != NULL)
            remove_list(last);
        set_entire_header(last, 0, (get_block_size(last->header) + PAGE_SZ), 0, get_prv_alloc(last->header), 0, 0);
        put_block(last);
    }
    sf_block *ptr = last;
    remove_list(last);
    if (get_block_size(last->header) - min_size >= min)
    {
        ptr = split_block(last, min_size);
        set_entire_header(ptr, 0, (get_block_size(ptr->header)), 0, get_prv_alloc(ptr->header), 0, 0);
        put_block(ptr);
    }
    epi = (sf_block *)(((intptr_t)sf_mem_end()) - 2 * sizeof(sf_header));
    set_entire_header(last, size, get_block_size(last->header), 1, get_prv_alloc(last->header), 0, 0);
    set_entire_header(epi, 0, 0, 1, get_alloc(epi->prev_footer), 0, 1);

    total_block_size += get_block_size(last->header);
    add_payload(size);

    return last->body.payload;
}

void sf_free(void *pp)
{
    sf_block *block = (sf_block *)(((intptr_t)pp) - 2 * sizeof(sf_header));
    valid_pointer(block);

    total_block_size -= get_block_size(block->header);
    add_payload(-1 * get_payload_size(block->header));
    sf_size_t index = (get_block_size(block->header) - 32) / 16;
    if (index < 10)
    {
        set_entire_header(block, 0, get_block_size(block->header), 1, get_prv_alloc(block->header), 1, 0);
        if (sf_quick_lists[index].length >= 5)
        {
            sf_block *ptr = sf_quick_lists[index].first;
            while (ptr != NULL)
            {
                sf_block *tmp = ptr;
                ptr = ptr->body.links.next;
                release_block(tmp);
            }
            sf_quick_lists[index].length = 0;
            sf_quick_lists->first = NULL;
        }
        block->body.links.next = sf_quick_lists[index].first;
        sf_quick_lists[index].length += 1;
        sf_quick_lists[index].first = block;
        return;
    }
    release_block(block);
}

void *sf_realloc(void *pp, sf_size_t rsize)
{
    sf_block *block = (sf_block *)(((intptr_t)pp) - 2 * sizeof(sf_header));
    valid_pointer(block);
    sf_size_t size = get_block_size(block->header);
    sf_size_t new_size = get_min_size(rsize);
    if (new_size > size)
    {
        int payload = (int)get_payload_size(block->header);
        add_payload(-1 * payload);
        sf_block *new;
        if ((new = sf_malloc(rsize)) == NULL)
            return NULL;
        new = (sf_block *)(((intptr_t) new) - 2 * sizeof(sf_header));
        memcpy(new->body.payload, block->body.payload, size);
        sf_free(pp);
        add_payload(payload);
        return new->body.payload;
    }
    else
    {
        total_block_size -= get_block_size(block->header);
        add_payload(-1 * get_payload_size(block->header));
        if (size - new_size < min)
        {
            set_entire_header(block, rsize, size, 1, get_prv_alloc(block->header), 0, 0);
            total_block_size += get_block_size(block->header);
            add_payload(rsize);
            return block->body.payload;
        }
        else
        {
            sf_block *ptr = split_block(block, new_size);
            set_entire_header(block, rsize, new_size, 1, get_prv_alloc(block->header), 0, 0);
            set_entire_header(ptr, 0, get_block_size(ptr->header), 0, get_prv_alloc(ptr->header), 0, 0);
            release_block(ptr);
            total_block_size += get_block_size(block->header);
            add_payload(rsize);
            return block->body.payload;
        }
    }
}

double sf_internal_fragmentation()
{
    if (total_payload == 0)
        return 0.0;
    return total_payload / total_block_size;
}

double sf_peak_utilization()
{
    if (max_payload == 0)
        return 0;
    return max_payload / total_pages;
}

void add_payload(int size)
{
    total_payload += size;
    if (total_payload > max_payload)
        max_payload = (double)total_payload;
}

int sf_initialize()
{
    if (sf_mem_grow() == NULL)
    {
        sf_errno = ENOMEM;
        return 1;
    }
    total_pages += 1024;
    pro = (sf_block *)sf_mem_start();
    set_entire_header(pro, 0, 32, 1, 1, 0, 1);
    epi = (sf_block *)(((intptr_t)sf_mem_end()) - 2 * sizeof(sf_header));

    set_entire_header(epi, 0, 0, 1, 1, 0, 1);
    for (size_t i = 0; i < NUM_FREE_LISTS; i++)
    {
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
    }
    for (size_t i = 0; i < NUM_QUICK_LISTS; i++)
    {
        sf_quick_lists[i].length = 0;
        sf_quick_lists[i].first = NULL;
    }
    sf_block *block = get_next_block(pro);
    sf_size_t size = PAGE_SZ - 6 * sizeof(sf_header);
    set_entire_header(block, 0, size, 0, 1, 0, 1);
    block->prev_footer = pro->header;
    put_block(block);
    return 0;
}

sf_size_t get_min_size(sf_size_t size)
{
    if (size <= 24)
        return min;
    else
        return (size + 8) % 16 == 0 ? (size + 8) : (size + 24) - ((size + 8) % 16);
}

void set_header(sf_block *block, sf_header value)
{
    value ^= MAGIC;
    block->header = value;
    if (block == epi)
        return;
    sf_block *next = get_next_block(block);
    if (get_alloc(block->header) == 0)
    {
        next->prev_footer = block->header;
        if ((prv_alloc & (sf_size_t)MAGIC) == 0)
            next->header = next->header & ~((uint64_t)prv_alloc);
        else
            next->header = next->header | (uint64_t)prv_alloc;
    }
    else
    {
        if ((prv_alloc & (sf_size_t)MAGIC) == 0)
            next->header = next->header | (uint64_t)prv_alloc;
        else
            next->header = next->header & ~((uint64_t)prv_alloc);
    }
}

void set_entire_header(sf_block *block, sf_size_t paylod_size, sf_size_t block_size,
                       sf_size_t is_alloc, sf_size_t is_prv_alloc, sf_size_t is_in_qklst, int create)
{
    if (create)
        block->header ^= MAGIC;
    set_alloc(block, is_alloc);
    set_payload_size(block, paylod_size);
    set_block_size(block, block_size);
    set_prv_alloc(block, is_prv_alloc);
    set_in_qklst(block, is_in_qklst);
}

void set_payload_size(sf_block *block, sf_size_t payload_size)
{
    sf_header header = block->header ^ MAGIC;
    sf_header value = (header & 0x00000000FFFFFFFF) | (uint64_t)payload_size << 32;
    set_header(block, value);
}

void set_block_size(sf_block *block, sf_size_t block_size)
{
    sf_header header = block->header ^ MAGIC;
    sf_header value = (header & 0xFFFFFFFF0000000F) | (uint64_t)block_size;
    set_header(block, value);
}

void set_alloc(sf_block *block, sf_size_t is_alloc)
{
    sf_header header = block->header ^ MAGIC;
    sf_header value = header;
    if (is_alloc)
        value |= (uint64_t)alloc;
    else
        value &= (~((uint64_t)alloc));
    set_header(block, value);
}

void set_prv_alloc(sf_block *block, sf_size_t is_prv_allc)
{
    sf_header header = block->header ^ MAGIC;
    sf_header value = header;
    if (is_prv_allc)
        value |= (uint64_t)prv_alloc;
    else
        value &= (~((uint64_t)prv_alloc));
    set_header(block, value);
}

void set_in_qklst(sf_block *block, sf_size_t is_in_qklst)
{
    sf_header value = block->header ^ MAGIC;
    if (is_in_qklst)
        value |= (uint64_t)in_qklst;
    else
        value &= (~((uint64_t)in_qklst));
    set_header(block, value);
}

sf_size_t get_payload_size(sf_header header)
{
    header ^= MAGIC;
    sf_size_t ans = ((uint64_t)header & 0xFFFFFFFF00000000) >> 32;
    return ans;
}

sf_size_t get_block_size(sf_header header)
{
    header ^= MAGIC;
    sf_size_t ans = ((uint64_t)header & 0x00000000FFFFFFF0);
    return ans;
}

sf_size_t get_alloc(sf_header header)
{
    header ^= MAGIC;
    sf_size_t ans = ((uint64_t)header & alloc) ? 1 : 0;
    return ans;
}

sf_size_t get_prv_alloc(sf_header header)
{
    header ^= MAGIC;
    sf_size_t ans = ((uint64_t)header & prv_alloc) ? 1 : 0;
    return ans;
}

sf_size_t get_in_qklst(sf_header header)
{
    header ^= MAGIC;
    sf_size_t ans = ((uint64_t)header & in_qklst) ? 1 : 0;
    return ans;
}

void put_block(sf_block *block)
{
    sf_size_t index = get_index(get_block_size(block->header));
    sf_block *header = &sf_free_list_heads[index];
    block->body.links.prev = header;
    block->body.links.next = header->body.links.next;
    header->body.links.next->body.links.prev = block;
    header->body.links.next = block;
}

sf_size_t get_index(sf_size_t size)
{
    size_t index = 0;
    for (; index < (NUM_FREE_LISTS - 1); index++)
        if (size <= get_pow(index) * 32)
            break;
    return index;
}

sf_size_t get_size_of_index(sf_size_t index)
{
    return 16 << index;
}

sf_size_t get_pow(sf_size_t pow)
{
    return 1 << pow;
}

sf_block *split_block(sf_block *block, size_t size)
{
    size_t original_size = get_block_size(block->header);
    size_t new_size = original_size - size;
    set_block_size(block, size);
    sf_block *ptr = get_next_block(block);
    set_entire_header(ptr, 0, new_size, 0, get_alloc(block->header), 0, 1);
    ptr->prev_footer = block->header;
    return ptr;
}

void valid_pointer(sf_block *pp)
{
    if (pp == NULL || (intptr_t)pp % 16 != 0)
        abort();
    if (get_block_size(pp->header) < 32 || get_block_size(pp->header) % 16 != 0)
        abort();
    if (((intptr_t)&pp->header) < (((intptr_t)&pro->header) + 32) || ((intptr_t)&pp->header) >= (((intptr_t)&epi->header) - 16))
        abort();
    if (get_alloc(pp->header) == 0 || (get_prv_alloc(pp->header) == 0 && get_alloc(pp->prev_footer) != 0))
        abort();
    return;
}

void release_block(sf_block *block)
{
    sf_block *next = get_next_block(block);
    sf_block *prev = get_prev_block(block);
    sf_size_t total_size = get_block_size(block->header);
    if (next != epi && get_alloc(next->header) == 0)
    {
        next->body.links.prev->body.links.next = next->body.links.next;
        next->body.links.next->body.links.prev = next->body.links.prev;
        total_size += get_block_size(next->header);
    }
    if (prev != pro && get_alloc(prev->header) == 0)
    {
        prev->body.links.prev->body.links.next = prev->body.links.next;
        prev->body.links.next->body.links.prev = prev->body.links.prev;
        total_size += get_block_size(prev->header);
        block = prev;
    }
    set_entire_header(block, 0, total_size, 0, get_prv_alloc(block->header), 0, 0);
    put_block(block);
    return;
}

sf_block *get_prev_block(sf_block *block)
{
    return (sf_block *)(((intptr_t)block) - get_block_size(block->prev_footer));
}

sf_block *get_next_block(sf_block *block)
{
    return (sf_block *)(((intptr_t)block) + get_block_size(block->header));
}

void remove_list(sf_block *block)
{
    block->body.links.next->body.links.prev = block->body.links.prev;
    block->body.links.prev->body.links.next = block->body.links.next;
}