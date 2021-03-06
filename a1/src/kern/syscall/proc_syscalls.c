/*
 * Process-related syscalls.
 * New for ASST1.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <current.h>
#include <pid.h>
#include <machine/trapframe.h>
#include <syscall.h>
#include <kern/wait.h>

/*
 * sys_fork
 *
 * create a new process, which begins executing in md_forkentry().
 */


int
sys_fork(struct trapframe *tf, pid_t *retval)
{
	struct trapframe *ntf; /* new trapframe, copy of tf */
	int result;

	/*
	 * Copy the trapframe to the heap, because we might return to
	 * userlevel and make another syscall (changing the trapframe)
	 * before the child runs. The child will free the copy.
	 */

	ntf = kmalloc(sizeof(struct trapframe));
	if (ntf==NULL) {
		return ENOMEM;
	}
	*ntf = *tf; /* copy the trapframe */

	result = thread_fork(curthread->t_name, enter_forked_process,
			     ntf, 0, retval);
	if (result) {
		kfree(ntf);
		return result;
	}

	return 0;
}

/*
 * sys_getpid
 * Placeholder to remind you to implement this.
 */
 pid_t sys_getpid(void) {
 	return curthread->t_pid;
 }

/*
 * sys_waitpid
 * Placeholder comment to remind you to implement this.
 */
pid_t sys_waitpid(pid_t pid, int *status, int options) {

	int result;

	/* Check the input options */
	if (options != 0 && options != WNOHANG) {
		return -EINVAL;
	}

	result = pid_validate_wait(pid, status, options, curthread->t_pid);
	if (result <0 ){
		return -1;
	}
	return result;

}

/*
 * sys_kill
 * Placeholder comment to remind you to implement this.
 */
int sys_kill(pid_t pid, int sig) {

	int result;

	result = pid_kill_helper(pid, sig);

	//if error occur, return -1;
	if (result < 0){
		return -1;
	}
	return 0;
}

