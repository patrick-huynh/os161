#include <types.h>
#include <kern/errno.h>
#include <kern/unistd.h>
#include <kern/wait.h>
#include <lib.h>
#include <syscall.h>
#include <current.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <copyinout.h>


#if OPT_A2
#include <mips/trapframe.h>
#endif
#include "opt-A2.h"

  /* this implementation of sys__exit does not do anything with the exit code */
  /* this needs to be fixed to get exit() and waitpid() working properly */

void sys__exit(int exitcode) {

  struct addrspace *as;
  struct proc *p = curproc;
  /* for now, just include this to keep the compiler from complaining about
     an unused variable */

#if OPT_A2
  // Assigning exit code to the struct child_info
  struct proc *parent = p->parent;
  if (parent != NULL) {
    lock_acquire(parent->children_lk); // acquire children
    struct array *children = parent->children;
    // finding the child
    for (unsigned i = 0; i < array_num(children); ++i) {
        struct child_info *ci = array_get(children, i);
        if (ci->pid == p->pid) {     // find within the parents children arr
          ci->exitcode = exitcode; // set exit code
          break;;
        }
    }
    lock_release(parent->children_lk);
    cv_signal(p->is_exited, p->children_lk);
  }
#else
  (void)exitcode;
#endif

  DEBUG(DB_SYSCALL,"Syscall: _exit(%d)\n",exitcode);

  KASSERT(curproc->p_addrspace != NULL);
  as_deactivate();
  /*
   * clear p_addrspace before calling as_destroy. Otherwise if
   * as_destroy sleeps (which is quite possible) when we
   * come back we'll be calling as_activate on a
   * half-destroyed address space. This tends to be
   * messily fatal.
   */
  as = curproc_setas(NULL);
  as_destroy(as);

  /* detach this thread from its process */
  /* note: curproc cannot be used after this call */
  proc_remthread(curthread);

  /* if this is the last user process in the system, proc_destroy()
     will wake up the kernel menu thread */
  proc_destroy(p);
  
  thread_exit();
  /* thread_exit() does not return, so we should never get here */
  panic("return from thread_exit in sys_exit\n");
}


/* stub handler for getpid() system call                */
int
sys_getpid(pid_t *retval)
{
  /* for now, this is just a stub that always returns a PID of 1 */
  /* you need to fix this to make it work properly */
#if OPT_A2
  *retval = curproc->pid;
#else
  *retval = 1;
#endif
  return(0);
}

/* stub handler for waitpid() system call                */

int
sys_waitpid(pid_t pid,
	    userptr_t status,
	    int options,
	    pid_t *retval)
{
  int exitstatus;
  int result;

  /* this is just a stub implementation that always reports an
     exit status of 0, regardless of the actual exit status of
     the specified process.   
     In fact, this will return 0 even if the specified process
     is still running, and even if it never existed in the first place.

     Fix this!
  */

  if (options != 0) {
    return(EINVAL);
  }

  #if OPT_A2
  struct proc *p = curproc;
  struct child_info *ci = NULL;
  // find the correct child
  lock_acquire(p->children_lk);
  for (unsigned i = 0; i < array_num(p->children); ++i) {
      struct child_info *curproc_ci = array_get(p->children, i);
      if (curproc_ci->pid == pid) {
        ci = curproc_ci;
        break;
      }
  }
  if (ci != NULL) { // if the given pid is a child of the process
    while (ci->exitcode == -1) {
      cv_wait(ci->proc->is_exited, p->children_lk);
    }
    exitstatus = _MKWAIT_EXIT(ci->exitcode);
  } else {
    lock_release(p->children_lk);
    *retval = -1;
    return ESRCH;
    }
  lock_release(p->children_lk);
#else
  /* for now, just pretend the exitstatus is 0 */
  exitstatus = 0;
#endif

  result = copyout((void *)&exitstatus,status,sizeof(int));
  if (result) {
    return(result);
  }
  *retval = pid;
  return(0);
}


#if OPT_A2
/* fork stub handler  */
int sys_fork(struct trapframe *tf, pid_t *retval) {
  KASSERT(tf != NULL);
  KASSERT(retval != NULL);

  struct proc *p = curproc;

  struct proc *c = proc_create_runprogram(p->p_name);
  KASSERT(c != NULL);
  KASSERT(c->pid > 0);

  c->parent = p;
  struct child_info *ci = kmalloc(sizeof(struct child_info));
  ci->pid = c->pid;
  ci->exitcode = -1;
  ci->proc = c;

  lock_acquire(p->children_lk);
  array_add(p->children, ci, NULL);
  lock_release(p->children_lk); 

  // copy over addrspace
  int err = as_copy(curproc_getas(),  &(c->p_addrspace));
  if (err != 0) panic("copy addrspace err");

  // copy over trapframe
  c->tf = kmalloc(sizeof(struct trapframe));
  KASSERT(c->tf != NULL);
  memcpy(c->tf, tf, sizeof(struct trapframe));

  thread_fork(c->p_name, c, (void *)&enter_forked_process, c->tf, 10);
  *retval = c->pid; // retur pid for parent process

  return 0; // return 0 for child process
}
#endif

