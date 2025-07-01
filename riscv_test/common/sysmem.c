#include <stdio.h>
#include <unistd.h>

typedef struct block_metadata
{
    size_t size;
    struct block_metadata *next;
    struct block_metadata *prev;
} block;

static block *free_head = NULL;

#define BLOCK_MEM(ptr) (void *)((unsigned long)(ptr) + sizeof(block))
#define BLOCK_HEADER(ptr) (void *)((unsigned long)(ptr) + sizeof(block))

void malloc_stats(void)
{
    block *current = free_head;
    size_t total_free = 0;
    size_t block_count = 0;

    while (current != NULL)
    {
        total_free += current->size;
        block_count++;
        current = current->next;
        printf("--> block addr: %p, size: %zu bytes\n", BLOCK_HEADER(current), current->size);
    }

    printf("Total free memory: %zu bytes\n", total_free);
    printf("Total free blocks: %zu\n", block_count);
}

static block *splitBlock(block *current, size_t size)
{
    block *new_block = (block *)((unsigned long)current + sizeof(block) + size);
    new_block->size = current->size - size - sizeof(block);
    new_block->next = current->next;
    new_block->prev = current;
    current->size = size;
    current->next = new_block;
    if (new_block->next != NULL)
    {
        new_block->next->prev = new_block;
    }
    return new_block;
}

static void add_to_free_list(block *free_block)
{
    free_block->next = free_head;
    free_block->prev = NULL;

    if (!free_head || (unsigned long)free_head >= (unsigned long)free_block)
    {
        if (free_head)
        {
            free_head->prev = free_block;
        }
        free_block->next = free_head;
        free_head = free_block;
        return;
    }

    block *current = free_head;
    while (current->next != NULL && (unsigned long)current->next < (unsigned long)free_block)
    {
        current = current->next;
    }
    free_block->next = current->next;
    free_block->prev = current;
    (current->next)->prev = free_block;
    current->next = free_block;
    return;
}

static void remove_from_free_list(block *free_block)
{
    if (free_block->prev != NULL)
    {
        if (free_block->next)
        {
            free_head = free_block->next;
        }
        else
        {
            free_head = NULL; // This is the tail of the free list
        }
    }
    else
    {
        free_block->prev->next = free_block->next; // This is the head of the free list
    }

    if (free_block->next != NULL)
    {
        free_block->next->prev = free_block->prev;
    }
}

static void scan_and_coalesce()
{
    block *current = free_head;
    unsigned long curr_addr, next_addr;
    while (current != NULL && current->next != NULL)
    {
        curr_addr = (unsigned long)current;
        next_addr = (unsigned long)current->next;

        if (curr_addr + sizeof(block) + current->size == next_addr)
        {
            // Coalesce the current block with the next block
            current->size += sizeof(block) + current->next->size;
            current->next = current->next->next;
            if (current->next != NULL)
            {
                current->next->prev = current;
            }
            else
                break;
        }
        current = current->next;
    }
}

void *malloc(size_t size)
{
    if (size == 0)
        return NULL;
    // Align size to the nearest multiple of 4
    size = (size + 3) & ~3; // Align to 4 bytes

    block *current = free_head;
    while (current != NULL)
    {
        if (current->size >= size)
        {
            if (current->size > size + sizeof(block))
            {
                current = splitBlock(current, size);
            }
            remove_from_free_list(current);
            return BLOCK_MEM(current);
        }
        current = current->next;
    }
    return NULL; // No suitable block found
}

void free(void *ptr)
{
    if (ptr == NULL)
        return;

    block *current = (block *)((unsigned long)ptr - sizeof(block));
    add_to_free_list(current);
    scan_and_coalesce();
}

void init_malloc(void *start, size_t size)
{
    block *blk;
    blk = (block *)start;
    blk->size = size - sizeof(block);
    add_to_free_list(blk);
}