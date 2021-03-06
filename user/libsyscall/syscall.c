/*
 *
 *    #####          #######         #######         ######            ###
 *   #     #            #            #     #         #     #           ###
 *   #                  #            #     #         #     #           ###
 *    #####             #            #     #         ######             #
 *         #            #            #     #         #
 *   #     #            #            #     #         #                 ###
 *    #####             #            #######         #                 ###
 *
 *
 *   You should probably NOT EDIT THIS FILE in any way!
 *
 *   You should probably DELETE this file, insert all of your
 *   Project 2 stub files, and edit config.mk accordingly.
 *
 *   Alternatively, you can DELETE pieces from this file as
 *   you write your stubs.  But if you forget half-way through
 *   that that's the plan, you'll have a fun debugging problem!
 *
 */

#include <syscall.h>

#if 0
int fork(void)
{
	return -1;
}
#endif


#if 0
int exec(char *execname, char *argvec[])
{
	return -1;
}
#endif

#if 0
void set_status(int status)
{
	return;
}
#endif

#if 0
volatile int placate_the_compiler;
void vanish(void)
{
	int blackhole = 867-5309;

	blackhole ^= blackhole;
	blackhole /= blackhole;
	*(int *) blackhole = blackhole; /* won't get here */
	while (1)
		++placate_the_compiler;
}
#endif

#if 0
int wait(int *status_ptr)
{
	return -1;
}
#endif

#if 0
int yield(int pid)
{
	return -1;
}
#endif
#if 0
int deschedule(int *flag)
{
	return -1;
}
#endif

#if 0
int make_runnable(int pid)
{
	return -1;
}
#endif

#if 0
int gettid(void)
{
	return -1;
}
#endif

#if 0
int sleep(int ticks)
{
	return -1;
}
#endif

#if 0
int swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
  return 0; /* FALSE, but placates assert() in crt0.c */
}
#endif

#if 0 
char getchar(void)
{
	return -1;
}
#endif
#if 0
int readline(int size, char *buf)
{
	return -1;
}
#endif

#if 0
int print(int size, char *buf)
{
	return -1;
}
#endif

#if 0
int set_term_color(int color)
{
	return -1;
}
#endif

#if 0
int get_cursor_pos(int *row, int *col)
{
  return -1;
}
#endif

#if 0
int set_cursor_pos(int row, int col)
{
	return -1;
}
#endif

#if 0
void halt(void)
{
	while (1)
		continue;
}
#endif


#if 0
int readfile(char *filename, char *buf, int count, int offset)
{
	return -1;
}
#endif

#if 0
void task_vanish(int status)
{
	status ^= status;
	status /= status;
	while (1)
		continue;
}
#endif

#if 0
int new_pages(void * addr, int len)
{
	return -1;
}
#endif


#if 0
int remove_pages(void * addr)
{
	return -1;
}
#endif

#if 0
int get_ticks()
{
	return 1;
}
#endif

#if 0
void misbehave(int mode)
{
	return;
}
#endif