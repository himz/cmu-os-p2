/**
 * @file largetest.c
 *
 * @brief A large-number-of-threads test of the thread implementation
 *
 * This test thr_creates() an "infinite" number of times and joins on some of
 * the threads.  The purpose is to test what happens when you run out of stack
 * space, and is good for stress testing (you will probibly run into a road 
 * block somewhere along the way whether it is a page fault or dead/live lock).
 *
 * It has been observed that this gets into ~2700 threads on some
 * implementations, so that may be a reasonable target.  Further, it was noted
 * that some implementations may suffer "hicups" from time to time, but
 * if such "hicups" last more than a few minutes, this is probably signs of a
 * bug.  Without deadlock detection it is hard to know for certain.
 *
 * @author Alejandro Lince (alince)
 * @author Spencer Whitman (swhitman)
 * @edited Nathaniel Wesley Filardo (nwf)
 *
 * @bug None known
 */
#include <syscall.h>
#include <simics.h>
#include <mutex.h>
#include <stdio.h>
#include <thread.h>
#include <sem.h>
#include <assert.h>
#include <thrgrp.h>

void* wtf(void* what) {
  int id = thr_getid();
  printf("hi from %d\n",id);

  thr_yield((int)what);
  thr_yield(-1);

  if((id % 8) == 0) {
    int tid;
    tid = thr_create(wtf, (void *)id);

    if (tid >= 0) {
      int status;
      thr_join(tid, (void **)&status);

      lprintf("%d joined with %d which exited with status %d\n",
	    id,tid,status);
      assert(tid == status);
    }
  } else {
    (void) thr_create(wtf, (void *)what);
  }
  return (void *)id;
}

int main()
{ 
  int status;
  int count = 0;

  thr_init(PAGE_SIZE);
  int id = thr_getid();

  thrgrp_group_t tg;

  thrgrp_init_group(&tg);

  while (1) {
    if(++count % 5) {
      thrgrp_create(&tg, wtf, (void*)id);
    }
    sleep(1);
    lprintf("-----------main is trying to join-------------------\n");
    thrgrp_join(&tg, (void **)&status);
    lprintf("%d joined with a worker which exited with status %d\n",
	   id,status);
  }

  thr_exit((void*)-1);
  return 1;
  
}
