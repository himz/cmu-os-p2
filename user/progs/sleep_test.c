#include <stdio.h>
#include <thread.h>
#include <mutex_type.h>
#include <mutex.h>
#include <simics.h>
#include <syscall.h>
#include <stdlib.h>


/* Define a global mutex */
mutex_t mp ;


int main()
{
    sleep(100);
    printf("After SLeep \n");
    return 0;
}
