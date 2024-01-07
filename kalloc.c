#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>

#include "kalloc.h"

// free list, for each memory allocated, we put some data before it
// containing the length of the allocation as unsigned long, and whether it is free or not
struct BeforeData
{
    unsigned long size;            // contains the size including both this part and the allocation memory
    unsigned long prev_block_size; // the size of the previous block (including BeforeData)
                                   // if this is 0, we are at the first block
    bool used;
};

struct KallocStruct kalloc_init(void)
{

    struct KallocStruct ks;
    ks.allocat = 0;
    ks.size = 0x100000000;

    ks.allocat = mmap(NULL, ks.size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ks.watermark = 0;

    unsigned long first_block_size = ks.size - sizeof(struct BeforeData);

    *((struct BeforeData *)ks.allocat) =
        (struct BeforeData){.size = first_block_size, .prev_block_size = 0, .used = false};

    // at the end we want one BeforeData with a supposed size 0, this signifies that it's the end of memory.
    *((struct BeforeData *)(ks.allocat + first_block_size)) =
        (struct BeforeData){.size = 0, .prev_block_size = first_block_size, .used = false};

    return ks;
}

void *kalloc_malloc(struct KallocStruct *ks, unsigned long size)
{
    size += sizeof(struct BeforeData);
    // traverse free list to find spot where `size` fits, for a block that is free
    unsigned char *at = ks->allocat;
    for (;;)
    {
        struct BeforeData *before_data = ((struct BeforeData *)at);
        // reached last before_block, therefore memory is out
        if (before_data->size == 0)
        {
            return NULL;
        }
        if (!before_data->used)
        {
            if (before_data->size == size)
            {
            GOTO_FILL_BLOCK:
                before_data->used = true;
                goto GOTO_RETURN;
            }
            else if (before_data->size > size)
            {
                // the block is bigger than us, split it into parts
                unsigned long size_for_next_block = before_data->size - size;
                if (size_for_next_block <= sizeof(struct BeforeData)) // TODO clean this up
                {
                    // this blocks sucks, we should not create it at all.
                    goto GOTO_FILL_BLOCK;
                }

                // the block that was originally after this one should now have a different prev_block_size
                ((struct BeforeData *)(at + before_data->size))->prev_block_size = size_for_next_block;

                before_data->used = true;
                // create new BeforeData block after this allocation
                *((struct BeforeData *)(at + size)) =
                    (struct BeforeData){.size = size_for_next_block, .prev_block_size = size, .used = false};
                before_data->size = size;

                goto GOTO_RETURN;
            }
        }
        at += before_data->size;
    }
GOTO_RETURN:
    if (at > ks->watermark)
    {
        ks->watermark = at;
    }
    return at + sizeof(struct BeforeData);
}

void kalloc_free(struct KallocStruct *ks, void *addr)
{
    register unsigned char *at = ((unsigned char *)addr) - sizeof(struct BeforeData);
    register unsigned long our_size = ((struct BeforeData *)at)->size;
    register unsigned long our_prev_block_size = ((struct BeforeData *)at)->prev_block_size;

    // begin by exploring to the right, eating them up
    for (;;)
    {
        // find block to the right of this one, if it's free, make this before_data block cover that one as well.
        struct BeforeData *right_block = ((struct BeforeData *)(at + our_size));
        // it has to be unused, and it can't be the last one
        if (right_block->used || right_block->size == 0)
        {
            break;
        }
        our_size += right_block->size;
    }

    // explore to the left, eating up that as well
    for (;;)
    {
        // we are at the first block
        if (our_prev_block_size == 0)
            break;

        struct BeforeData *left_block = (struct BeforeData *)(at - our_prev_block_size);
        if (left_block->used)
            break;
        our_size += left_block->size;
        our_prev_block_size = left_block->prev_block_size;
        at = (unsigned char *)left_block;
    }

    // set prev size of block after the new one we have
    ((struct BeforeData *)(at + our_size))->prev_block_size = our_size;

    // really initialize the final new free block in memory
    *((struct BeforeData *)at) =
        (struct BeforeData){.used = false, .size = our_size, .prev_block_size = our_prev_block_size};
}
