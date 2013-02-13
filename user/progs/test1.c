#include <stdio.h>
#include <thread.h>
#include <simics.h>
#include <syscall.h>

int main()
{

    int rc = 0;

    rc = thr_init(10);

    lprintf("[APP_%s],  after thr_init  rc = %d\n", __FUNCTION__, rc);

    return 0;
}
