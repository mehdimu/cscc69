/* BEGIN A3 SETUP */
/*
 * File handles and file tables.
 * New for ASST3
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/limits.h>
#include <kern/stat.h>
#include <kern/unistd.h>
#include <kern/fcntl.h>
#include <synch.h>
#include <current.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>
#include <syscall.h>

/*** openfile functions ***/

/*
 * file_open
 * opens a file, places it in the filetable, sets RETFD to the file
 * descriptor. the pointer arguments must be kernel pointers.
 * NOTE -- the passed in filename must be a mutable string.
 *
 * A3: As per the OS/161 man page for open(), you do not need
 * to do anything with the "mode" argument.
 */
int
file_open(char *filename, int flags, int mode, int *retfd)
{

	(void) mode;
	struct vnode *vn;
	int fd_index;
	int result;
	struct filetable *ft = curthread->t_filetable;

	result = vfs_open(filename, flags, 0, &vn);
	if (result) {
		return result;
	}

	struct fd_entry *new_file = kmalloc(sizeof(struct fd_entry));
	if (new_file == NULL) {
		vfs_close(vn);
		return ENOMEM;
	}
	new_file->fd_lock = lock_create("file lock");
	if (new_file->fd_lock == NULL) {
		vfs_close(vn);
		kfree(new_file);
		return ENOMEM;
	}

	/* Copy the bookkeeping */
	new_file->fname = filename;
	new_file->vn = vn;
	new_file->offset = 0;
	new_file->num_counter = 1;
	new_file->flags = flags;

	fd_index = filetable_getfd(ft);
	ft->entries[fd_index] = new_file;
	*retfd = fd_index;

	return 0;
}


/*
 * file_close
 * Called when a process closes a file descriptor.  Think about how you plan
 * to handle fork, and what (if anything) is shared between parent/child after
 * fork.  Your design decisions will affect what you should do for close.
 */
int
file_close(int fd)
{
	if (fd < 0 || fd >= __OPEN_MAX) {
		return EBADF;
	}

	struct filetable *ft = curthread->t_filetable;
	struct fd_entry *file = ft->entries[fd];
	if (file == NULL) {
		return EBADF;
	}

	lock_acquire(file->fd_lock);
	ft->entries[fd] = NULL;

	if (file->num_counter == 1) {
		vfs_close(file->vn);
		lock_release(file->fd_lock);
		lock_destroy(file->fd_lock);
		kfree(file);
	}
	else {
		KASSERT(file->num_counter > 1);
		file->num_counter--;
		lock_release(file->fd_lock);
	}

	return 0;
}

/*** filetable functions ***/

/*
 * filetable_init
 * pretty straightforward -- allocate the space, set up
 * first 3 file descriptors for stdin, stdout and stderr,
 * and initialize all other entries to NULL.
 *
 * Should set curthread->t_filetable to point to the
 * newly-initialized filetable.
 *
 * Should return non-zero error code on failure.  Currently
 * does nothing but returns success so that loading a user
 * program will succeed even if you haven't written the
 * filetable initialization yet.
 */

int
filetable_init(void)
{
	int result;
    int retval;
    char file_name[32];

    curthread->t_filetable = kmalloc(sizeof(struct filetable));
    if (curthread->t_filetable == NULL){
        return ENOMEM;
    }

 	//clear the filetable
    for(int i = 0; i < __OPEN_MAX; i++){
        curthread->t_filetable->entries[i]=NULL;
    }

 	//stdin is settle on fd, 0
	strcpy(file_name, "con:");
    result = file_open(file_name, O_RDONLY, 0, &retval);
    if (result){
        return result;
    }

	//fd 1 is for stdout
    strcpy(file_name, "con:");
    result = file_open(file_name, O_WRONLY, 0, &retval);
    if (result){
        return result;
    }

    //fd 2 is for stderr
    strcpy(file_name, "con:");
    result = file_open(file_name, O_WRONLY, 0, &retval);
    if (result){
        return result;
    }
    return 0;
}


int
filetable_fork(struct filetable *new_table){

  struct filetable *ft = curthread->t_filetable;
  if (ft == NULL){
    new_table = NULL;
    return 0;
  }
  new_table = kmalloc(sizeof(struct filetable));

  if (new_table == NULL){
  	return ENOMEM;
  }

  for (int i=0;i<__OPEN_MAX;i++){
    if(ft->entries[i] != NULL){
      lock_acquire(ft->entries[i]->fd_lock);
      ft->entries[i]->num_counter++;
      new_table->entries[i]=ft->entries[i];
      lock_release(ft->entries[i]->fd_lock);
    }else{
      new_table->entries[i] = NULL;
    }
  }
  return 0;
}


/*
 * filetable_destroy
 * closes the files in the file table, frees the table.
 * This should be called as part of cleaning up a process (after kill
 * or exit).
 */
void
filetable_destroy(struct filetable *ft)
{

	int result;

	for (size_t i = 0;i < __OPEN_MAX; i++) {
		/* code */
		if (ft->entries[i]) {
			result = file_close(i);
			KASSERT(result == 0);
		}
	}

	kfree(ft);
}


/*
 * You should add additional filetable utility functions here as needed
 * to support the system calls.  For example, given a file descriptor
 * you will want some sort of lookup function that will check if the fd is
 * valid and return the associated vnode (and possibly other information like
 * the current file position) associated with that open file.
 */

int filetable_getfd(struct filetable *ft) {

	// struct filetable *ft = curthread->t_filetable;

	for (size_t i = 0; i < __OPEN_MAX; i++) {
		if (ft->entries[i] == NULL) {
			return i;
		}
	}

	/* If not found any NULL entries, e.g. the table is full */
	return -1;

}

int file_dup2(int oldfd, int newfd, int *retval){
    struct filetable *ftable = curthread->t_filetable;
    if (newfd >=  __OPEN_MAX || newfd < 0 ||oldfd >= __OPEN_MAX || oldfd < 0){
      return EBADF;
    }
    if (oldfd==newfd){
      *retval = newfd;
      return 0;
    }
    if (ftable->entries[newfd] != NULL){
      file_close(newfd);
    }else{
    }
    if (ftable->entries[oldfd] == NULL){
      return EBADF;
    }
    lock_acquire(ftable->entries[oldfd]->fd_lock);
    ftable->entries[oldfd]->num_counter++;
    ftable->entries[newfd]=ftable->entries[oldfd];
    lock_release(ftable->entries[oldfd]->fd_lock);
    *retval = newfd;
    return 0;
  }

/* END A3 SETUP */
