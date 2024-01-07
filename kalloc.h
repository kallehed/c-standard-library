#ifndef KALLOC_H
#define KALLOC_H

struct KallocStruct
{
    unsigned char *allocat;
    unsigned long size;
    unsigned char *watermark;
};

struct KallocStruct kalloc_init(void);

void *kalloc_malloc(struct KallocStruct *ks, unsigned long size);

void kalloc_free(struct KallocStruct *ks, void *addr);

#endif
