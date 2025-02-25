#include "ext2fs.h"
#include "bitops.h"

#define _INLINE_

/*
 * The inline routines themselves...
 *
 * If NO_INLINE_FUNCS is defined, then we won't try to do inline
 * functions at all; they will be included as normal functions in
 * inline.c
 */
#ifdef NO_INLINE_FUNCS
#if (defined(__GNUC__) && (defined(__i386__) || defined(__i486__) || \
			   defined(__i586__) || defined(__mc68000__)))
	/* This prevents bitops.c from trying to include the C */
	/* function version of these functions */
#define _EXT2_HAVE_ASM_BITOPS_
#endif
#endif /* NO_INLINE_FUNCS */

#if (defined(INCLUDE_INLINE_FUNCS) || !defined(NO_INLINE_FUNCS))
#ifdef INCLUDE_INLINE_FUNCS
#define _INLINE_ extern
#else
#ifdef __GNUC__
#define _INLINE_ extern __inline__
#else				/* For Watcom C */
#define _INLINE_ extern inline
#endif
#endif

/*
 * Fast bit set/clear functions that doesn't need to return the
 * previous bit value.
 */

_INLINE_ void ext2fs_fast_set_bit(unsigned int nr,void * addr)
{
	unsigned char	*ADDR = (unsigned char *) addr;

	ADDR += nr >> 3;
	*ADDR |= (1 << (nr & 0x07));
}

_INLINE_ void ext2fs_fast_clear_bit(unsigned int nr, void * addr)
{
	unsigned char	*ADDR = (unsigned char *) addr;

	ADDR += nr >> 3;
	*ADDR &= ~(1 << (nr & 0x07));
}


_INLINE_ void ext2fs_fast_set_bit64(__u64 nr, void * addr)
{
	unsigned char	*ADDR = (unsigned char *) addr;

	ADDR += nr >> 3;
	*ADDR |= (1 << (nr & 0x07));
}

_INLINE_ void ext2fs_fast_clear_bit64(__u64 nr, void * addr)
{
	unsigned char	*ADDR = (unsigned char *) addr;

	ADDR += nr >> 3;
	*ADDR &= ~(1 << (nr & 0x07));
}


#if ((defined __GNUC__) && !defined(_EXT2_USE_C_VERSIONS_) && \
     (defined(__i386__) || defined(__i486__) || defined(__i586__)))

#define _EXT2_HAVE_ASM_BITOPS_
#define _EXT2_HAVE_ASM_SWAB_

/*
 * These are done by inline assembly for speed reasons.....
 *
 * All bitoperations return 0 if the bit was cleared before the
 * operation and != 0 if it was not.  Bit 0 is the LSB of addr; bit 32
 * is the LSB of (addr+1).
 */

/*
 * Some hacks to defeat gcc over-optimizations..
 */
struct __dummy_h { unsigned long a[100]; };
#define EXT2FS_ADDR (*(struct __dummy_h *) addr)
#define EXT2FS_CONST_ADDR (*(const struct __dummy_h *) addr)

_INLINE_ int ext2fs_set_bit(unsigned int nr, void * addr)
{
	int oldbit;

	addr = (void *) (((unsigned char *) addr) + (nr >> 3));
	__asm__ __volatile__("btsl %2,%1\n\tsbbl %0,%0"
		:"=r" (oldbit),"+m" (EXT2FS_ADDR)
		:"r" (nr & 7));
	return oldbit;
}

_INLINE_ int ext2fs_clear_bit(unsigned int nr, void * addr)
{
	int oldbit;

	addr = (void *) (((unsigned char *) addr) + (nr >> 3));
	__asm__ __volatile__("btrl %2,%1\n\tsbbl %0,%0"
		:"=r" (oldbit),"+m" (EXT2FS_ADDR)
		:"r" (nr & 7));
	return oldbit;
}

_INLINE_ int ext2fs_test_bit(unsigned int nr, const void * addr)
{
	int oldbit;

	addr = (const void *) (((const unsigned char *) addr) + (nr >> 3));
	__asm__ __volatile__("btl %2,%1\n\tsbbl %0,%0"
		:"=r" (oldbit)
		:"m" (EXT2FS_CONST_ADDR),"r" (nr & 7));
	return oldbit;
}

_INLINE_ __u32 ext2fs_swab32(__u32 val)
{
#ifdef EXT2FS_REQUIRE_486
	__asm__("bswap %0" : "=r" (val) : "0" (val));
#else
	__asm__("xchgb %b0,%h0\n\t"	/* swap lower bytes	*/
		"rorl $16,%0\n\t"	/* swap words		*/
		"xchgb %b0,%h0"		/* swap higher bytes	*/
		:"=q" (val)
		: "0" (val));
#endif
	return val;
}

_INLINE_ __u16 ext2fs_swab16(__u16 val)
{
	__asm__("xchgb %b0,%h0"		/* swap bytes		*/ \
		: "=q" (val) \
		:  "0" (val)); \
		return val;
}

#undef EXT2FS_ADDR

#endif	/* i386 */

#if ((defined __GNUC__) && !defined(_EXT2_USE_C_VERSIONS_) && \
     (defined(__mc68000__)))

#define _EXT2_HAVE_ASM_BITOPS_

_INLINE_ int ext2fs_set_bit(unsigned int nr,void * addr)
{
	char retval;

	__asm__ __volatile__ ("bfset %2@{%1:#1}; sne %0"
	     : "=d" (retval) : "d" (nr^7), "a" (addr));

	return retval;
}

_INLINE_ int ext2fs_clear_bit(unsigned int nr, void * addr)
{
	char retval;

	__asm__ __volatile__ ("bfclr %2@{%1:#1}; sne %0"
	     : "=d" (retval) : "d" (nr^7), "a" (addr));

	return retval;
}

_INLINE_ int ext2fs_test_bit(unsigned int nr, const void * addr)
{
	char retval;

	__asm__ __volatile__ ("bftst %2@{%1:#1}; sne %0"
	     : "=d" (retval) : "d" (nr^7), "a" (addr));

	return retval;
}

#endif /* __mc68000__ */


#if !defined(_EXT2_HAVE_ASM_SWAB_)

_INLINE_ __u16 ext2fs_swab16(__u16 val)
{
	return (val >> 8) | (val << 8);
}

_INLINE_ __u32 ext2fs_swab32(__u32 val)
{
	return ((val>>24) | ((val>>8)&0xFF00) |
		((val<<8)&0xFF0000) | (val<<24));
}

#endif /* !_EXT2_HAVE_ASM_SWAB */

_INLINE_ __u64 ext2fs_swab64(__u64 val)
{
	return (ext2fs_swab32(val >> 32) |
		(((__u64)ext2fs_swab32(val & 0xFFFFFFFFUL)) << 32));
}

_INLINE_ int ext2fs_mark_block_bitmap(ext2fs_block_bitmap bitmap,
				       blk_t block)
{
	return ext2fs_mark_generic_bitmap((ext2fs_generic_bitmap) bitmap,
					  block);
}

_INLINE_ int ext2fs_unmark_block_bitmap(ext2fs_block_bitmap bitmap,
					 blk_t block)
{
	return ext2fs_unmark_generic_bitmap((ext2fs_generic_bitmap) bitmap,
					    block);
}

_INLINE_ int ext2fs_test_block_bitmap(ext2fs_block_bitmap bitmap,
				       blk_t block)
{
	return ext2fs_test_generic_bitmap((ext2fs_generic_bitmap) bitmap,
					  block);
}

_INLINE_ int ext2fs_mark_inode_bitmap(ext2fs_inode_bitmap bitmap,
				       ext2_ino_t inode)
{
	return ext2fs_mark_generic_bitmap((ext2fs_generic_bitmap) bitmap,
					  inode);
}

_INLINE_ int ext2fs_unmark_inode_bitmap(ext2fs_inode_bitmap bitmap,
					 ext2_ino_t inode)
{
	return ext2fs_unmark_generic_bitmap((ext2fs_generic_bitmap) bitmap,
				     inode);
}

_INLINE_ int ext2fs_test_inode_bitmap(ext2fs_inode_bitmap bitmap,
				       ext2_ino_t inode)
{
	return ext2fs_test_generic_bitmap((ext2fs_generic_bitmap) bitmap,
					  inode);
}

_INLINE_ void ext2fs_fast_mark_block_bitmap(ext2fs_block_bitmap bitmap,
					    blk_t block)
{
	ext2fs_mark_generic_bitmap((ext2fs_generic_bitmap) bitmap, block);
}

_INLINE_ void ext2fs_fast_unmark_block_bitmap(ext2fs_block_bitmap bitmap,
					      blk_t block)
{
	ext2fs_unmark_generic_bitmap((ext2fs_generic_bitmap) bitmap, block);
}

_INLINE_ int ext2fs_fast_test_block_bitmap(ext2fs_block_bitmap bitmap,
					    blk_t block)
{
	return ext2fs_test_generic_bitmap((ext2fs_generic_bitmap) bitmap,
					  block);
}

_INLINE_ void ext2fs_fast_mark_inode_bitmap(ext2fs_inode_bitmap bitmap,
					    ext2_ino_t inode)
{
	ext2fs_mark_generic_bitmap((ext2fs_generic_bitmap) bitmap, inode);
}

_INLINE_ void ext2fs_fast_unmark_inode_bitmap(ext2fs_inode_bitmap bitmap,
					      ext2_ino_t inode)
{
	ext2fs_unmark_generic_bitmap((ext2fs_generic_bitmap) bitmap, inode);
}

_INLINE_ int ext2fs_fast_test_inode_bitmap(ext2fs_inode_bitmap bitmap,
					   ext2_ino_t inode)
{
	return ext2fs_test_generic_bitmap((ext2fs_generic_bitmap) bitmap,
					  inode);
}

_INLINE_ blk_t ext2fs_get_block_bitmap_start(ext2fs_block_bitmap bitmap)
{
	return ext2fs_get_generic_bitmap_start((ext2fs_generic_bitmap) bitmap);
}

_INLINE_ ext2_ino_t ext2fs_get_inode_bitmap_start(ext2fs_inode_bitmap bitmap)
{
	return ext2fs_get_generic_bitmap_start((ext2fs_generic_bitmap) bitmap);
}

_INLINE_ blk_t ext2fs_get_block_bitmap_end(ext2fs_block_bitmap bitmap)
{
	return ext2fs_get_generic_bitmap_end((ext2fs_generic_bitmap) bitmap);
}

_INLINE_ ext2_ino_t ext2fs_get_inode_bitmap_end(ext2fs_inode_bitmap bitmap)
{
	return ext2fs_get_generic_bitmap_end((ext2fs_generic_bitmap) bitmap);
}

_INLINE_ int ext2fs_fast_test_block_bitmap_range(ext2fs_block_bitmap bitmap,
						 blk_t block, int num)
{
	return ext2fs_test_block_bitmap_range(bitmap, block, num);
}

_INLINE_ void ext2fs_fast_mark_block_bitmap_range(ext2fs_block_bitmap bitmap,
						  blk_t block, int num)
{
	ext2fs_mark_block_bitmap_range(bitmap, block, num);
}

_INLINE_ void ext2fs_fast_unmark_block_bitmap_range(ext2fs_block_bitmap bitmap,
						    blk_t block, int num)
{
	ext2fs_unmark_block_bitmap_range(bitmap, block, num);
}

/* 64-bit versions */

_INLINE_ int ext2fs_mark_block_bitmap2(ext2fs_block_bitmap bitmap,
				       blk64_t block)
{
	return ext2fs_mark_generic_bmap((ext2fs_generic_bitmap) bitmap,
					block);
}

_INLINE_ int ext2fs_unmark_block_bitmap2(ext2fs_block_bitmap bitmap,
					 blk64_t block)
{
	return ext2fs_unmark_generic_bmap((ext2fs_generic_bitmap) bitmap, block);
}

_INLINE_ int ext2fs_test_block_bitmap2(ext2fs_block_bitmap bitmap,
				       blk64_t block)
{
	return ext2fs_test_generic_bmap((ext2fs_generic_bitmap) bitmap,
					block);
}

_INLINE_ int ext2fs_mark_inode_bitmap2(ext2fs_inode_bitmap bitmap,
				       ext2_ino_t inode)
{
	return ext2fs_mark_generic_bmap((ext2fs_generic_bitmap) bitmap,
					inode);
}

_INLINE_ int ext2fs_unmark_inode_bitmap2(ext2fs_inode_bitmap bitmap,
					 ext2_ino_t inode)
{
	return ext2fs_unmark_generic_bmap((ext2fs_generic_bitmap) bitmap,
					  inode);
}

_INLINE_ int ext2fs_test_inode_bitmap2(ext2fs_inode_bitmap bitmap,
				       ext2_ino_t inode)
{
	return ext2fs_test_generic_bmap((ext2fs_generic_bitmap) bitmap,
					inode);
}

_INLINE_ void ext2fs_fast_mark_block_bitmap2(ext2fs_block_bitmap bitmap,
					     blk64_t block)
{
	ext2fs_mark_generic_bmap((ext2fs_generic_bitmap) bitmap, block);
}

_INLINE_ void ext2fs_fast_unmark_block_bitmap2(ext2fs_block_bitmap bitmap,
					       blk64_t block)
{
	ext2fs_unmark_generic_bmap((ext2fs_generic_bitmap) bitmap, block);
}

_INLINE_ int ext2fs_fast_test_block_bitmap2(ext2fs_block_bitmap bitmap,
					    blk64_t block)
{
	return ext2fs_test_generic_bmap((ext2fs_generic_bitmap) bitmap,
					block);
}

_INLINE_ void ext2fs_fast_mark_inode_bitmap2(ext2fs_inode_bitmap bitmap,
					     ext2_ino_t inode)
{
	ext2fs_mark_generic_bmap((ext2fs_generic_bitmap) bitmap, inode);
}

_INLINE_ void ext2fs_fast_unmark_inode_bitmap2(ext2fs_inode_bitmap bitmap,
					       ext2_ino_t inode)
{
	ext2fs_unmark_generic_bmap((ext2fs_generic_bitmap) bitmap, inode);
}

_INLINE_ int ext2fs_fast_test_inode_bitmap2(ext2fs_inode_bitmap bitmap,
					    ext2_ino_t inode)
{
	return ext2fs_test_generic_bmap((ext2fs_generic_bitmap) bitmap,
					inode);
}

_INLINE_ blk64_t ext2fs_get_block_bitmap_start2(ext2fs_block_bitmap bitmap)
{
	return ext2fs_get_generic_bmap_start((ext2fs_generic_bitmap) bitmap);
}

_INLINE_ ext2_ino_t ext2fs_get_inode_bitmap_start2(ext2fs_inode_bitmap bitmap)
{
	return ext2fs_get_generic_bmap_start((ext2fs_generic_bitmap) bitmap);
}

_INLINE_ blk64_t ext2fs_get_block_bitmap_end2(ext2fs_block_bitmap bitmap)
{
	return ext2fs_get_generic_bmap_end((ext2fs_generic_bitmap) bitmap);
}

_INLINE_ ext2_ino_t ext2fs_get_inode_bitmap_end2(ext2fs_inode_bitmap bitmap)
{
	return ext2fs_get_generic_bmap_end((ext2fs_generic_bitmap) bitmap);
}

_INLINE_ int ext2fs_fast_test_block_bitmap_range2(ext2fs_block_bitmap bitmap,
						  blk64_t block,
						  unsigned int num)
{
	return ext2fs_test_block_bitmap_range2(bitmap, block, num);
}

_INLINE_ void ext2fs_fast_mark_block_bitmap_range2(ext2fs_block_bitmap bitmap,
						   blk64_t block,
						   unsigned int num)
{
	ext2fs_mark_block_bitmap_range2(bitmap, block, num);
}

_INLINE_ void ext2fs_fast_unmark_block_bitmap_range2(ext2fs_block_bitmap bitmap,
						     blk64_t block,
						     unsigned int num)
{
	ext2fs_unmark_block_bitmap_range2(bitmap, block, num);
}

#undef _INLINE_
#endif
