#ifndef _ALLOC_H
#define _ALLOC_H

#include "mlvalues.h"

mlvalue *caml_alloc(size_t size);
void caml_realloc(int64_t cpt_obj_mem);
int mark_and_compact();

typedef struct _block_list
{
    mlvalue *block;
    struct _block_list *next;
} block_list;

#endif
