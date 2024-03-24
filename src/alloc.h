#ifndef _ALLOC_H
#define _ALLOC_H

#include "mlvalues.h"

typedef struct _block_list
{
    mlvalue *block;
    struct _block_list *next;
} block_list;


/*Fonction appelée par l'interprèteur de bytecode à chaque allocation*/
mlvalue *caml_alloc(size_t size);
int mark_and_compact();

int64_t mark(mlvalue *stack);

int64_t calc_new_addr(mlvalue ***tab_mem_addr, int64_t cpt_obj_mem);

void apply_new_addr(mlvalue ***tab_mem_addr, int64_t cpt_obj_mem, mlvalue *stack);

void slide(mlvalue ***tab_mem_addr, int64_t nb_obj_mem);

/*Fonction de réallocation du tas lorsque celui ci n'est pas assez grand*/
void caml_realloc(int64_t cpt_obj_mem);

/*Helpers*/

void set_color(mlvalue *hd, color_t color);

void enfile(block_list **racine, mlvalue *ptr_block);

int make_TODO(block_list **racine, mlvalue *stack);

mlvalue ***make_tab_addr(int64_t taille);
void free_tab_addr(mlvalue ***tab_addr);

void print_tab(mlvalue ***tab, int64_t taille);

#endif
