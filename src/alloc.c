#include <stdlib.h>

#include "alloc.h"
#include "mlvalues.h"
#include "config.h"
#include "domain_state.h"
#include "interp.h"

mlvalue* caml_alloc(size_t size) {
  return aligned_alloc(8,size);
}


typedef struct _block_list
{
  mlvalue *block;
  block_list * next;
} block_list;


void set_color(mlvalue* hd,color_t color){
  *hd &=  0xFFFFFFFFFFFFFCFF; /* on met le champ couleur a 0 */
  *hd |= (color << 8); /* on met la couleur dans le champ correspondant */
}

void enfile(block_list** racine,mlvalue* ptr_block){
  if(Color_hd(*(ptr_block-1))==WHITE){
        set_color((ptr_block-1),GRAY);
        block_list *new_elem = (block_list*)malloc(sizeof(block_list));
        new_elem->block = ptr_block;  /* création de la liste chainé d'elem a traité*/
        new_elem->next = *racine;             /* ajout en tete (parcrous en profondeur) */
        *racine = new_elem;
      }

}

void make_TODO(block_list** racine, mlvalue* stack){
  for(unsigned int i = 0; i < sp; i++){
    if(Is_block(stack[i])){
      mlvalue* ptr_block = Ptr_val(stack[i]);
      enfile(racine,ptr_block);
    }
  }
}



uint64_t mark(mlvalue* stack){
  uint64_t taille_mem_used = 0;
  block_list * todo = NULL;
  make_TODO(&todo,stack);

  while(todo != NULL){
    mlvalue* addr_hd_block = (todo->block) -1;
    int64_t size_block = Size_hd(*addr_hd_block);
    for(int64_t i =0; i<size_block; i++){
      if(Is_block((addr_hd_block+1)[i])){
        mlvalue* ptr_block = Ptr_val((addr_hd_block+1)[i]);
        enfile(&todo,ptr_block);
      }
    }
    set_color((todo->block-1),BLACK);
    taille_mem_used = Size(todo->block);
    todo = todo->next;
  }
  return taille_mem_used;
}

void mark_and_compact(){
  mlvalue * heap =  Caml_state->heap;
  mlvalue * stack = Caml_state->stack;
  uint64_t taille_mem;

  taille_mem = mark(stack);


}
