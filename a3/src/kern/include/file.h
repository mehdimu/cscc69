/* BEGIN A3 SETUP */
/*
 * Declarations for file handle and file table management.
 * New for A3.
 */

#ifndef _FILE_H_
#define _FILE_H_

#include <kern/limits.h>

struct vnode;

struct fd_entry {
	const char *fname;
	struct vnode *vn;
	int flags;
	int num_counter;
	struct lock *fd_lock;
	off_t offset;
};
/*
 * filetable struct
 * just an array, nice and simple.
 * It is up to you to design what goes into the array.  The current
 * array of ints is just intended to make the compiler happy.
 */
struct filetable {
	struct fd_entry *entries[__OPEN_MAX];
};

/* these all have an implicit arg of the curthread's filetable */
int filetable_init(void);
void filetable_destroy(struct filetable *ft);
int filetable_getfd(struct filetable *ft);

/* opens a file (must be kernel pointers in the args) */
int file_open(char *filename, int flags, int mode, int *retfd);

/* closes a file */
int file_close(int fd);

int file_dup2(int oldfd, int newfd, int *retval);

/* A3: You should add additional functions that operate on
 * the filetable to help implement some of the filetable-related
 * system calls.
 */
int
filetable_fork(struct filetable *new_table);
#endif /* _FILE_H_ */

/* END A3 SETUP */
