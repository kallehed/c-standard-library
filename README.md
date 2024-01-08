# my own C standard library

it has malloc, free, and print(printf), works by using linux system calls.
So linux exclusive currently.

## COMMAND TO COMPILE AND RUN:

`cc -Wall -Wextra -Wpedantic -static -g main.c kalloc.c print.c -o a.out && ./a.out`

### STATS:

the following paragraphs record benchmarks I made for the malloc implementation
They record a watermark, which was the highest amount of memory used in bytes

An allocator that DOES NOT submerge freed blocks, will, at 200 pointers, for
100000 iterations of freeing and allocing random pointers for random sizes
between 0 and 10000 give: 5011989, 5186591, 4961949 as watermark

with merging a single block to the right: 897004, 911532, 945807, 933199. 6
times less memory used!

merging as many blocks as possible to the right: 838301, 830599, 842365, 825858.
A little less mem use

merging to both right and left, truly coalescing: 782998, 750396,
756740, 782717. Even less
