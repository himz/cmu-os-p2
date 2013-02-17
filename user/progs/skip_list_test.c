#include <stdio.h>
#include <stdlib.h>
#include <skip_list_common.h>
#include <string.h>

int main()
{
    skip_list_global_t list_glb;
    int data = 10;
    memset(&list_glb, 0, sizeof(skip_list_global_t));

    skip_list_init(&list_glb, 5, 5);

    skip_list_insert(&list_glb, 20, 20, &data);
    skip_list_insert(&list_glb, 20, 30, &data);
    skip_list_insert(&list_glb, 20, 10, &data);
    skip_list_insert(&list_glb, 20, 5, &data);

    skip_list_insert(&list_glb, 10, 20, &data);
    skip_list_insert(&list_glb, 10, 30, &data);
    skip_list_insert(&list_glb, 10, 10, &data);
    skip_list_insert(&list_glb, 10, 5, &data);

    skip_list_insert(&list_glb, 30, 20, &data);
    skip_list_insert(&list_glb, 30, 30, &data);
    skip_list_insert(&list_glb, 30, 10, &data);
    skip_list_insert(&list_glb, 30, 5, &data);

    skip_list_remove(&list_glb, 30, 30);
    skip_list_remove(&list_glb, 30, 10);
    skip_list_remove(&list_glb, 30, 5);
    skip_list_remove(&list_glb, 30, 20);

    skip_list_remove(&list_glb, 20, 30);
    skip_list_remove(&list_glb, 20, 10);
    skip_list_remove(&list_glb, 20, 5);
    skip_list_remove(&list_glb, 20, 20);

    skip_list_remove(&list_glb, 10, 30);
    skip_list_remove(&list_glb, 10, 10);
    skip_list_remove(&list_glb, 10, 5);
    skip_list_remove(&list_glb, 10, 20);


    skip_list_dbg_dump_all(&list_glb);

    return (0);
}
