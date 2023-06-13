#include <types.h>
#include <lib.h>
#include <synchprobs.h>
#include <synch.h>
#include <opt-A1.h>

/* 
 * This simple default synchronization mechanism allows only vehicle at a time
 * into the intersection.   The intersectionSem is used as a a lock.
 * We use a semaphore rather than a lock so that this code will work even
 * before locks are implemented.
 */

/* 
 * Replace this default synchronization mechanism with your own (better) mechanism
 * needed for your solution.   Your mechanism may use any of the available synchronzation
 * primitives, e.g., semaphores, locks, condition variables.   You are also free to 
 * declare other global variables if your solution requires them.
 */

/*
 * replace this with declarations of any synchronization and other variables you need here
 */
static struct semaphore *intersectionSem;
static struct cv *n, *e , *s, *w;
static struct lock *mutex;
Direction volatile dir = 4;
int volatile count [] = {0 , 0 , 0, 0};
int volatile waitQ [] = {0, 0, 0, 0};

/* 
 * The simulation driver will call this function once before starting
 * the simulation
 *
 * You can use it to initialize synchronization and other variables.
 * 
 */
void
intersection_sync_init(void)
{
  /* replace this default implementation with your own implementation */

  intersectionSem = sem_create("intersectionSem",1);
  n = cv_create("north");
  e = cv_create("east");
  s = cv_create("south");
  w = cv_create("west");
  mutex = lock_create("mutex");

  if (intersectionSem == NULL) {
    panic("could not create intersection semaphore");
  }
  if (n == NULL || e == NULL  || s == NULL || w == NULL) {
    panic("could not create intersection condition variable");
  }
  if (mutex == NULL) {
    panic("could not create intersection mutex");
  }
  return;
}

/* 
 * The simulation driver will call this function once after
 * the simulation has finished
 *
 * You can use it to clean up any synchronization and other variables.
 *
 */
void
intersection_sync_cleanup(void)
{
  /* replace this default implementation with your own implementation */
  KASSERT(intersectionSem != NULL);
  sem_destroy(intersectionSem);
  cv_destroy(n);
  cv_destroy(e);
  cv_destroy(s);
  cv_destroy(w);
  lock_destroy(mutex);
}

static bool isSameDir(int volatile arr [], int n, int dir) {
  for (int i = 0; i < n; ++i) {
    if (i != dir && arr[i] != 0) return false;
  }
  return true;
}


/*
 * The simulation driver will call this function each time a vehicle
 * tries to enter the intersection, before it enters.
 * This function should cause the calling simulation thread 
 * to block until it is OK for the vehicle to enter the intersection.
 *
 * parameters:
 *    * origin: the Direction from which the vehicle is arriving
 *    * destination: the Direction in which the vehicle is trying to go
 *
 * return value: none
 */

void
intersection_before_entry(Direction origin, Direction destination) 
{
  /* replace this default implementation with your own implementation */
  (void)origin;  /* avoid compiler complaint about unused parameter */
  (void)destination; /* avoid compiler complaint about unused parameter */
  KASSERT(intersectionSem != NULL);
  // P(intersectionSem);

  lock_acquire(mutex);
  
  if (dir == 4) {
    dir = origin;
    waitQ[dir] = 0;
    count[dir]++;
    lock_release(mutex);
    return;
  }

  while (dir != origin || !isSameDir(count, 4, origin))  {
    waitQ[origin]++;
    if (origin == north) {
      cv_wait(n, mutex);
    } else if (origin == east) {
      cv_wait(e, mutex);
    } else if (origin == south) {
      cv_wait(s, mutex);
    } else if (origin == west) {
      cv_wait(w, mutex);
    } 
  }
  waitQ[dir] = 0;
  count[dir]++;

  lock_release(mutex);

  // if (origin == north) {
  //   kprintf("Car going in || Origin: N");
  //   kprintf("\n");
  // } else if (origin == east) {
  //   kprintf("Car going in || Origin: E");
  //   kprintf("\n");
  // } else if (origin == south) {
  //   kprintf("Car going in || Origin: S");
  //   kprintf("\n");
  // } else if (origin == west) {
  //   kprintf("Car going in || Origin: W");
  //   kprintf("\n");
  // }
}


/*
 * The simulation driver will call this function each time a vehicle
 * leaves the intersection.
 *
 * parameters:
 *    * origin: the Direction from which the vehicle arrived
 *    * destination: the Direction in which the vehicle is going
 *
 * return value: none
 */

void
intersection_after_exit(Direction origin, Direction destination) 
{
  /* replace this default implementation with your own implementation */
  (void)origin;  /* avoid compiler complaint about unused parameter */
  (void)destination; /* avoid compiler complaint about unused parameter */
  KASSERT(intersectionSem != NULL);
  // V(intersectionSem);

  lock_acquire(mutex);
  count[origin]--;

  // Case where theres no more cars in q
  int totalCount = 0;
  int totalWaitQ = 0;
  for (int i = 0; i < 4; ++i) {
    totalCount += count[i];
    totalWaitQ += waitQ[i]; 
  }
  if (totalCount + totalWaitQ == 0) {
    dir = 4;
    lock_release(mutex);
    return;
  } else if (totalCount != 0) {
    lock_release(mutex);
    return;
  }

  // Case where there is cars waiting or in the intersection
  Direction oldDir = dir;
  do {
    dir = (dir + 1) % 4;
  } while (waitQ[dir] <= 0 && oldDir != dir);
  if (oldDir == dir) { 
    dir = 4; // set to no direction
    lock_release(mutex);
    return;
  }

  if (dir == north) {
    cv_broadcast(n, mutex);
  } else if (dir == east) {
    cv_broadcast(e, mutex);
  } else if (dir == south) {
    cv_broadcast(s, mutex);
  } else if (dir == west) {
    cv_broadcast(w, mutex);
  }
  
  lock_release(mutex);
}
