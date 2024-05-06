#ifndef MACRO_H__
#define MACRO_H__

#define KiB (1024LL)
#define MiB (1024LL * 1024)
#define PHYSICAL_PAGE (4 * KiB)
#define SLAB_PAGE (64 * KiB)
#define SLAB_MASK (0xffff)

#define FREE_MEM (0xaaaaaaaa)
#define SLAB_MEM (0xbbbbbbbb)
#define BIG_MEM (0xcccccccc)

#endif
