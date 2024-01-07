// #define _XOPEN_SOURCE_EXTENDED 1
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include "kalloc.h"
#include "print.h"
#include <stdlib.h> // for time and srand, rand

// cc -Wall -Wextra -Wpedantic -static -g main.c kalloc.c print.c -o a.out && ./a.out

// STATS:
// An allocator that DOES NOT submerge freed blocks, will, at 200 pointers,
// for 100000 iterations of freeing and allocing random pointers for random
// sizes between 0 and 10000 give: 5011989, 5186591, 4961949 as watermark

// with merging a single block to the right: 897004, 911532, 945807, 933199. 6 times less memory used!

// merging as many blocks as possible to the right: 838301, 830599, 842365, 825858. A little less mem use

// merging to both right and left, truly coalescing: 782998, 750396, 756740, 782717. Even less

int main(/*int argc, char *argv[]*/)
{
    print("time: %ld\n", time(NULL));
    srand(time(NULL));
    print("hello\n");
    // get current break
    long a = syscall(SYS_brk, NULL);
    print("brk at: %ld\n", a);

    struct KallocStruct ks = kalloc_init();

    long pointers = 200;
    void **pointer_arr = kalloc_malloc(&ks, pointers * sizeof(void *));
    for (int i = 0; i < pointers; ++i) // initialize to zero
    {
        pointer_arr[i] = NULL;
    }

    for (int i = 0; i < 1000; ++i)
    {
        long idx = rand() % pointers;
        if (pointer_arr[idx] == NULL)
        {
            long size = rand() % 10000;
            pointer_arr[idx] = kalloc_malloc(&ks, size);
            if (pointer_arr[idx] == NULL)
            {
                print("failed allocation!\n");
                return 1;
            }
            for (long j = 0; j < size; ++j)
            {
                ((unsigned char *)pointer_arr[idx])[j] = rand() % 256;
            }
        }
        else
        {
            kalloc_free(&ks, pointer_arr[idx]);
            pointer_arr[idx] = NULL;
        }
    }

    long nr_not_null = 0;
    for (int i = 0; i < pointers; ++i)
    {
        if (pointer_arr[i] != NULL)
        {
            ++nr_not_null;
        }
    }
    print("nr not null: %ld \n", nr_not_null);

    kalloc_free(&ks, pointer_arr);

    print("watermark: %ld \n", ks.watermark - ks.allocat);

    print("hello: %c!%c%c %ld my string: %s \n", 'x', 'y', 'z', 2382147483647, "coolio");

    return 0;
}
