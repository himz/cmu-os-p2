#include <stdio.h>
#include <thread.h>
#include <mutex_type.h>
#include <mutex.h>
#include <simics.h>
#include <syscall.h>


/* Define a global mutex */
mutex_t mp ;


int main()
{
    int rm = 0;
    rm = mutex_init(&mp);

    mutex_lock(&mp);
    mutex_unlock(&mp);
    mutex_lock(&mp);

    return 0;
}
