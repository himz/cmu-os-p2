#include <stdio.h>
#include <stdlib.h>
#include <thread.h>

void *
thr_func(void *input) 
{
    char *ptr = NULL;

    ptr = malloc(sizeof(char));

    printf("[DBG_%s], after malloc \n", __FUNCTION__);

    return 0;

}

int main()
{
    char *ptr = NULL;

    thr_init(10);
    thr_create(thr_func, NULL);
    ptr = malloc(sizeof(char));

    return 0;
}
