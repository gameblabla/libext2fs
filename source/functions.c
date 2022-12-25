#include "ext2fs.h"

#define _INLINE_

#ifndef EXT2_CUSTOM_MEMORY_ROUTINES
#include <string.h>
#include "mem_allocate.h"
/*
 *  Allocate memory
 */
_INLINE_ errcode_t ext2fs_get_mem(unsigned long size, void *ptr)
{
	void *pp;

	pp = mem_alloc(size);
	if (!pp)
		return EXT2_ET_NO_MEMORY;
	memcpy(ptr, &pp, sizeof (pp));
	return 0;
}

_INLINE_ errcode_t ext2fs_get_memzero(unsigned long size, void *ptr)
{
	void *pp;

	pp = mem_alloc(size);
	if (!pp)
		return EXT2_ET_NO_MEMORY;
	memset(pp, 0, size);
	memcpy(ptr, &pp, sizeof(pp));
	return 0;
}

_INLINE_ errcode_t ext2fs_get_array(unsigned long count, unsigned long size, void *ptr)
{
	if (count && (-1UL)/count<size)
		return EXT2_ET_NO_MEMORY;
	return ext2fs_get_mem(count*size, ptr);
}

_INLINE_ errcode_t ext2fs_get_arrayzero(unsigned long count,
					unsigned long size, void *ptr)
{
	void *pp;

	if (count && (-1UL)/count<size)
		return EXT2_ET_NO_MEMORY;
	pp = mem_calloc(count, size);
	if (!pp)
		return EXT2_ET_NO_MEMORY;
	memcpy(ptr, &pp, sizeof(pp));
	return 0;
}

/*
 * Free memory
 */
_INLINE_ errcode_t ext2fs_free_mem(void *ptr)
{
	void *p;

	memcpy(&p, ptr, sizeof(p));
	mem_free(p);
	p = 0;
	memcpy(ptr, &p, sizeof(p));
	return 0;
}

/*
 *  Resize memory
 */
_INLINE_ errcode_t ext2fs_resize_mem(unsigned long EXT2FS_ATTR((unused)) old_size,
				     unsigned long size, void *ptr)
{
	void *p;

	/* Use "memcpy" for pointer assignments here to avoid problems
	 * with C99 strict type aliasing rules. */
	memcpy(&p, ptr, sizeof(p));
	p = mem_realloc(p, size);
	if (!p)
		return EXT2_ET_NO_MEMORY;
	memcpy(ptr, &p, sizeof(p));
	return 0;
}
#endif	/* Custom memory routines */

/*
 * Mark a filesystem superblock as dirty
 */
_INLINE_ void ext2fs_mark_super_dirty(ext2_filsys fs)
{
	fs->flags |= EXT2_FLAG_DIRTY | EXT2_FLAG_CHANGED;
}

/*
 * Mark a filesystem as changed
 */
_INLINE_ void ext2fs_mark_changed(ext2_filsys fs)
{
	fs->flags |= EXT2_FLAG_CHANGED;
}

/*
 * Check to see if a filesystem has changed
 */
_INLINE_ int ext2fs_test_changed(ext2_filsys fs)
{
	return (fs->flags & EXT2_FLAG_CHANGED);
}

/*
 * Mark a filesystem as valid
 */
_INLINE_ void ext2fs_mark_valid(ext2_filsys fs)
{
	fs->flags |= EXT2_FLAG_VALID;
}

/*
 * Mark a filesystem as NOT valid
 */
_INLINE_ void ext2fs_unmark_valid(ext2_filsys fs)
{
	fs->flags &= ~EXT2_FLAG_VALID;
}

/*
 * Check to see if a filesystem is valid
 */
_INLINE_ int ext2fs_test_valid(ext2_filsys fs)
{
	return (fs->flags & EXT2_FLAG_VALID);
}

/*
 * Mark the inode bitmap as dirty
 */
_INLINE_ void ext2fs_mark_ib_dirty(ext2_filsys fs)
{
	fs->flags |= EXT2_FLAG_IB_DIRTY | EXT2_FLAG_CHANGED;
}

/*
 * Mark the block bitmap as dirty
 */
_INLINE_ void ext2fs_mark_bb_dirty(ext2_filsys fs)
{
	fs->flags |= EXT2_FLAG_BB_DIRTY | EXT2_FLAG_CHANGED;
}

/*
 * Check to see if a filesystem's inode bitmap is dirty
 */
_INLINE_ int ext2fs_test_ib_dirty(ext2_filsys fs)
{
	return (fs->flags & EXT2_FLAG_IB_DIRTY);
}

/*
 * Check to see if a filesystem's block bitmap is dirty
 */
_INLINE_ int ext2fs_test_bb_dirty(ext2_filsys fs)
{
	return (fs->flags & EXT2_FLAG_BB_DIRTY);
}

/*
 * Return the group # of a block
 */
_INLINE_ int ext2fs_group_of_blk(ext2_filsys fs, blk_t blk)
{
	return ext2fs_group_of_blk2(fs, blk);
}
/*
 * Return the group # of an inode number
 */
_INLINE_ int ext2fs_group_of_ino(ext2_filsys fs, ext2_ino_t ino)
{
	return (ino - 1) / fs->super->s_inodes_per_group;
}

/*
 * Return the first block (inclusive) in a group
 */
_INLINE_ blk_t ext2fs_group_first_block(ext2_filsys fs, dgrp_t group)
{
	return ext2fs_group_first_block2(fs, group);
}

/*
 * Return the last block (inclusive) in a group
 */
_INLINE_ blk_t ext2fs_group_last_block(ext2_filsys fs, dgrp_t group)
{
	return ext2fs_group_last_block2(fs, group);
}

_INLINE_ blk_t ext2fs_inode_data_blocks(ext2_filsys fs,
					struct ext2_inode *inode)
{
	return ext2fs_inode_data_blocks2(fs, inode);
}

/*
 * This is an efficient, overflow safe way of calculating ceil((1.0 * a) / b)
 */
_INLINE_ unsigned int ext2fs_div_ceil(unsigned int a, unsigned int b)
{
	if (!a)
		return 0;
	return ((a - 1) / b) + 1;
}

_INLINE_ __u64 ext2fs_div64_ceil(__u64 a, __u64 b)
{
	if (!a)
		return 0;
	return ((a - 1) / b) + 1;
}

_INLINE_ int ext2fs_open_file(const char *pathname, int flags, mode_t mode)
{
	va_list args;

	if (mode)
#if defined(HAVE_OPEN64) && !defined(__OSX_AVAILABLE_BUT_DEPRECATED)
		return open64(pathname, flags, mode);
	else
		return open64(pathname, flags);
#else
		return open(pathname, flags, mode);
	else
		return open(pathname, flags);
#endif
}

_INLINE_ int ext2fs_stat(const char *path, ext2fs_struct_stat *buf)
{
#if defined(HAVE_FSTAT64) && !defined(__OSX_AVAILABLE_BUT_DEPRECATED)
	return stat64(path, buf);
#else
	return stat(path, buf);
#endif
}

_INLINE_ int ext2fs_fstat(int fd, ext2fs_struct_stat *buf)
{
#if defined(HAVE_FSTAT64) && !defined(__OSX_AVAILABLE_BUT_DEPRECATED)
	return fstat64(fd, buf);
#else
	return fstat(fd, buf);
#endif
}
