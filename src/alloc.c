#include <stdlib.h>
#include <stdio.h>

#include "alloc.h"
#include "mlvalues.h"
#include "config.h"
#include "domain_state.h"
#include "interp.h"
#include "debug.h"

int heap_free = 0;

typedef struct _block_list
{
  mlvalue *block;
  struct _block_list *next;
} block_list;

void set_color(mlvalue *hd, color_t color)
{
  *hd &= 0xFFFFFFFFFFFFFCFF;   /* on met le champ couleur a 0 */
  *hd |= (((color) & 3) << 8); /* on met la couleur dans le champ correspondant */
}

void enfile(block_list **racine, mlvalue *ptr_block)
{

  block_list *new_elem = (block_list *)malloc(sizeof(block_list));
  new_elem->block = ptr_block; /* création de la liste chainé d'elem a traité*/
  new_elem->next = *racine;    /* ajout en tete (parcrous en profondeur) */
  *racine = new_elem;
}

int make_TODO(block_list **racine, mlvalue *stack)
{

  mlvalue *ptr_block;
  int cpt_obj_mem = 0;
  for (unsigned int i = 0; i < Caml_state->sp; i++)
  {
    if (Is_block(stack[i]) && (Color((stack[i])) == WHITE))
    {
      ptr_block = Ptr_val(stack[i]); /* on chaine sur le header des bloc*/
      // printf("stack[i] : %lx stack[i]-1 %lx  Ptr_val(stack[i]) : %lx Ptr_val(stack[i])-1 : %lx Ptr_val(stack[i]-1) : %lx\n",stack[i],stack[i]-1,Ptr_val(stack[i]),Ptr_val(stack[i])-1,Ptr_val(stack[i]-1));
      set_color((ptr_block - 1), GRAY);
      if (Color((stack[i])) == WHITE)
      {
        printf("is still white");
      }
      enfile(racine, ptr_block);
      cpt_obj_mem++;
    }
  } // printf("fin stack\n");
  if (Is_block(Caml_state->accu) && (Color((Caml_state->accu)) == WHITE))
  {
    ptr_block = Ptr_val(Caml_state->accu);
    set_color((ptr_block - 1), GRAY); /* on chaine sur le header des bloc */
    enfile(racine, ptr_block);
    cpt_obj_mem++;
  }
  if (Is_block(Caml_state->env) && (Color((Caml_state->env)) == WHITE))
  {
    ptr_block = Ptr_val(Caml_state->env); /* on chaine sur le header des bloc */
    set_color((ptr_block - 1), GRAY);
    enfile(racine, ptr_block);
    cpt_obj_mem++;
  }
  return cpt_obj_mem;
}

int64_t mark(mlvalue *stack)
{
  block_list *todo = NULL;
  block_list *tmp;
  mlvalue *block;
  int64_t size_block;
  mlvalue *ptr_block;
  int cpt_obj_mem = make_TODO(&todo, stack);

  while (todo != NULL)
  {
    // printf("todo : %p todo_next %p block : %p\n",todo,todo->next,todo->block);
    tmp = todo;
    block = todo->block;
    todo = todo->next;
    free(tmp);
    size_block = Size(block);

    for (int64_t i = 0; i < size_block; i++)
    {
      if (Is_block(block[i]) && (Color((block[i])) == WHITE))
      {
        ptr_block = Ptr_val(block[i]);
        // printf("avant gris head : %lx addr %p ",*(ptr_block-1),(ptr_block-1));
        set_color((ptr_block - 1), GRAY);
        // printf("après gris head : %lx \n",*(ptr_block-1));
        enfile(&todo, ptr_block);
        cpt_obj_mem++;
      }
    }

    // printf("avant black head : %lx addr %p ",*(block-1),(block-1));
    set_color((block - 1), BLACK);
    // printf("après black head : %lx \n",*(block-1));
  }

  return cpt_obj_mem;
}

int64_t calc_new_addr(mlvalue ***tab_mem_addr, int64_t cpt_obj_mem)
{
  mlvalue *current_heap_top = Caml_state->heap;
  mlvalue *heap_pointer = Caml_state->heap;
  int new_heap_free = 0;
  int64_t i = 0;
  int64_t cpt_obj_vu = 0;
  // printf("debut while\n");
  while ((cpt_obj_vu < cpt_obj_mem) && (heap_pointer < Caml_state->heap + Caml_state->heap_size))
  {
    header_t head_block = *heap_pointer;
    int64_t size = Size_hd(head_block);
    char *tag;
    if (Tag_hd(head_block) == ENV_T)
    {
      tag = "env";
    }
    else
    {
      if (Tag_hd(head_block) == CLOSURE_T)
      {
        tag = "closure";
      }
      else
      {
        if (Tag_hd(head_block) == BLOCK_T)
        {
          tag = "block";
        }
        else
        {
          tag = "erreur";
        }
      }
    }
    if (Color_hd(head_block) == BLACK)
    {
      // printf("debut if\n");
      tab_mem_addr[0][i] = (mlvalue *)heap_pointer; /*old addr*/
      // printf("debut if2\n");
      tab_mem_addr[1][i] = (mlvalue *)current_heap_top; /*new addr*/
      // printf("heap_top(new addr) :%p, heap_pointer(old addr) :%p, taille : %ld, tag : 0x%x color : 0x%x val : 0x%lx\n",(tab_mem_addr)[1][i],(tab_mem_addr)[0][i],size+1,Tag_hd(head_block),Color_hd(head_block),head_block);
      // printf("current_heap_top   :%p heap_pointer            :%p \n",current_heap_top,heap_pointer);
      for (int t = 0; t < size + 1; t++)
      {
        //  printf("content : %lx\n",heap_pointer[t]);
      }
      // print_tab(tab_mem_addr,cpt_obj_mem);
      current_heap_top += (size + 1);
      new_heap_free += (size + 1);
      cpt_obj_vu++;
      i++;
    }
    else
    {
      // printf("bloc_blanc\n");
      // printf("current_heap_top   :%p heap_pointer            :%p, taille : %ld, tag : 0x%x color : 0x%x val : 0x%lx\n\n",current_heap_top,heap_pointer,size+1,Tag_hd(head_block),Color_hd(head_block),head_block);
    }
    heap_pointer += (size + 1);
    if (heap_pointer > Caml_state->heap + Caml_state->heap_size)
    {

      // printf("fin du buzz heap_pointer : %p current_heap_top :%p obj_mem %ld obj_vu : %ld\n",heap_pointer,current_heap_top,cpt_obj_mem,cpt_obj_vu);
    }
  }
  // print_tab(tab_mem_addr,cpt_obj_mem);
  return new_heap_free;
}

void apply_new_addr(mlvalue ***tab_mem_addr, int64_t cpt_obj_mem, mlvalue *stack)
{
  /* debut de la modif de  la pile */

  for (int i = 0; i < Caml_state->sp; i++)
  {

    if (Is_block(stack[i]))
    {

      for (int j = 0; j < cpt_obj_mem; j++)
      { /* recherche de la nouvel adresse du bloc pointé */
        // printf("(((mlvalue*)stack[i])-1): 0x%lx,stack[i] : 0x%lx,(tab_mem_addr)[0][j]) : 0x%lx,(((tab_mem_addr)[1][j])-1) : 0x%lx\n",(((mlvalue*)stack[i])-1),stack[i],(tab_mem_addr)[0][j],(((tab_mem_addr)[1][j])-1) );
        if ((((mlvalue *)stack[i]) - 1) == (tab_mem_addr)[0][j])
        {
          stack[i] = (((tab_mem_addr)[1][j]) + 1);

          // printf("stack is modified\n");
          break;
        }
      }
    }

  } /* fin de la modif de la pile */

  // printf("fin modif pile\n");
  mlvalue *heap_pointer = Caml_state->heap;
  int64_t cpt_obj_vu = 0;

  while (cpt_obj_vu < cpt_obj_mem)
  { /* on parcours la memoire */
    header_t head_block = *heap_pointer;
    int64_t size = Size_hd(head_block);

    if (Color_hd(head_block) == BLACK)
    { /* si on est dans la memoire vivante */
      for (int64_t i = 0; i < size; i++)
      { /* parcours des valeurs du bloc */
        if (Is_block((heap_pointer + 1)[i]))
        { /* si on a un pointeur dans le bloc */
          for (int j = 0; j < cpt_obj_mem; j++)
          { /* recherche de la nouvel adresse du bloc pointé */
            if (((mlvalue *)(heap_pointer + 1)[i]) - 1 == (tab_mem_addr)[0][j])
            {                                                     /* old addr */
              (heap_pointer + 1)[i] = ((tab_mem_addr)[1][j] + 1); /* new addr */
            }
          }
        }
      }
      cpt_obj_vu++;
    }

    heap_pointer += (size + 1);
  }

  if (Is_block(Caml_state->accu))
  {

    for (int j = 0; j < cpt_obj_mem; j++)
    { /* recherche de la nouvel adresse du bloc pointé */
      // printf("(((mlvalue*)stack[i])-1): 0x%lx,stack[i] : 0x%lx,(tab_mem_addr)[0][j]) : 0x%lx,(((tab_mem_addr)[1][j])-1) : 0x%lx\n",(((mlvalue*)stack[i])-1),stack[i],(tab_mem_addr)[0][j],(((tab_mem_addr)[1][j])-1) );
      if ((((mlvalue *)Caml_state->accu) - 1) == (tab_mem_addr)[0][j])
      {
        Caml_state->accu = (((tab_mem_addr)[1][j]) + 1);

        // printf("stack is modified\n");
        break;
      }
    }
  }
  if (Is_block(Caml_state->env))
  {

    for (int j = 0; j < cpt_obj_mem; j++)
    { /* recherche de la nouvel adresse du bloc pointé */
      // printf("(((mlvalue*)stack[i])-1): 0x%lx,stack[i] : 0x%lx,(tab_mem_addr)[0][j]) : 0x%lx,(((tab_mem_addr)[1][j])-1) : 0x%lx\n",(((mlvalue*)stack[i])-1),stack[i],(tab_mem_addr)[0][j],(((tab_mem_addr)[1][j])-1) );
      if ((((mlvalue *)Caml_state->env) - 1) == (tab_mem_addr)[0][j])
      {
        Caml_state->env = (((tab_mem_addr)[1][j]) + 1);

        // printf("stack is modified\n");
        break;
      }
    }
  }
}

void slide(mlvalue ***tab_mem_addr, int64_t nb_obj_mem)
{

  for (int64_t i = 0; i < nb_obj_mem; i++)
  {
    mlvalue *src = (tab_mem_addr)[0][i]; /* old addr */
    mlvalue *dst = (tab_mem_addr)[1][i]; /* new addr */
    int64_t size = Size_hd(*src);        /* taille du block */
    if (src != dst)
    {
      for (int64_t i = 0; i < size + 1; i++)
      {
        dst[i] = src[i]; /*on fait passer tout les element du bloque a leur nouvelle adresse */
      }
    }
    set_color(Ptr_val(dst), WHITE); /* on repasse la couleur du block d'on le traitement est fini a blanc */
    // printf("src : %p dst : %p size : %ld \n",src,dst,size);
  }
}

mlvalue ***make_tab_addr(int64_t taille)
{
  mlvalue ***addr_tab = (mlvalue ***)malloc(sizeof(mlvalue **) * 2);
  addr_tab[0] = (mlvalue **)malloc(sizeof(mlvalue *) * taille);
  addr_tab[1] = (mlvalue **)malloc(sizeof(mlvalue *) * taille);
  return addr_tab;
}

void free_tab_addr(mlvalue ***tab_addr)
{
  free(tab_addr[0]);
  free(tab_addr[1]);
  free(tab_addr);
}

void print_tab(mlvalue ***tab, int64_t taille)
{

  printf("tab : \n ");
  for (int i = 0; i < taille; i++)
  {
    printf("%p :%lx |", &(tab)[0][i], (tab)[0][i]);
  }
  printf("\n ");
  for (int j = 0; j < taille; j++)
  {
    printf("%p :%lx |", &(tab)[1][j], (tab)[1][j]);
  }
  printf("\n ");
}

int mark_and_compact()
{
// printf("debut gc \n");
// printf("heap_free :%d\n",heap_free);
#ifdef DEBUG
  printf("Caml_state->accu=%s  Caml_state->sp=%d stack=[",
         val_to_str(Caml_state->accu), Caml_state->sp);
  if (Caml_state->sp > 0)
  {
    printf("%s", val_to_str(Caml_state->stack[Caml_state->sp - 1]));
  }
  for (int i = Caml_state->sp - 2; i >= 0; i--)
  {
    printf(";%s  0x%lx\n", val_to_str(Caml_state->stack[i]), Caml_state->stack[i]);
  }
  printf("]  Caml_state->env=%s\n", val_to_str(Caml_state->env));

#endif
  mlvalue *stack = Caml_state->stack;
  int64_t nb_obj_mem;
  // printf("Caml_state->heap addr : %p, stack addr :%p\n",Caml_state->heap,stack);
  nb_obj_mem = mark(stack); /* mark */
  // printf("nb_obj :%ld\n",nb_obj_mem);
  mlvalue ***tab_mem_addr = make_tab_addr(nb_obj_mem);
  // printf("avant\n");
  // print_tab(tab_mem_addr,nb_obj_mem);
  heap_free = calc_new_addr(tab_mem_addr, nb_obj_mem); /*compact P1 calcules les nvls addrs */
  // printf("\n après \n");
  // print_tab(tab_mem_addr,nb_obj_mem);
  // printf("new heap_free :%d\n",heap_free);
  apply_new_addr(tab_mem_addr, nb_obj_mem, stack); /*compact P2 modification de toute les valeurs de la memoire vivante */
// printf("application new addr terminé\n");
#define NOT
#ifdef DEBUG
  printf("Caml_state->accu=%lx  Caml_state->sp=%d stack=[",
         (Caml_state->accu), Caml_state->sp);
  if (Caml_state->sp > 0)
  {
    printf("%s", val_to_str(Caml_state->stack[Caml_state->sp - 1]));
  }
  for (int i = Caml_state->sp - 2; i >= 0; i--)
  {
    printf(";%s  0x%lx\n", val_to_str(Caml_state->stack[i]), Caml_state->stack[i]);
  }
#endif
  slide(tab_mem_addr, nb_obj_mem); /*compact P3 glissement de touts les bloques vers leur nouvelle addr*/
  // printf("fin slide \n");
  free_tab_addr(tab_mem_addr);
  // printf("fin gc \n");
  return nb_obj_mem;
}

int nb_alloc = 0;
int nb_alloc_prec = 0;
mlvalue *caml_alloc(size_t size)
{
  if (heap_free + size >= (Caml_state->heap_size) / sizeof(mlvalue)) // Les mlvalues étant des entiers 64 bits, le nombre de cases est égal à Caml_state->heap_size/8
  {
    // if(nb_alloc-nb_alloc_prec==50){
    //  if((heap_free+size)*sizeof(mlvalue)>Caml_state->heap_size){
    //  printf("heap_free : %d\n",heap_free);
    //  printf("on a marqué %d block\n",mark(Caml_state->stack)); //print_stack();
    //  print_heap();
    // printf("Appel au GC\n");
    int nb_obj = mark_and_compact();
    // mlvalue m = *(Caml_state->Caml_state->heap+Caml_state->heap_size*1000000000000);
    nb_alloc_prec = nb_alloc;
    while (heap_free + size >= (Caml_state->heap_size) / sizeof(mlvalue))
    {
      // printf("Appel à realloc\n");
      caml_realloc(nb_obj);
    }
  }
  // printf("on me demande un bloc de taille %d\n",size);
  mlvalue *addr_block = Caml_state->heap + heap_free;
  heap_free += size;
  nb_alloc++;
  return addr_block;
}

void caml_realloc(int64_t cpt_obj_mem)
{
  mlvalue *old_heap = Caml_state->heap;
  Caml_state->heap_size = Caml_state->heap_size * 2;
  Caml_state->heap = realloc(Caml_state->heap, Caml_state->heap_size);
  if (old_heap == Caml_state->heap)
  {
    // printf("rien à faire\n");
    return; // Rien à faire
  }
  int64_t decalage = ((void *)Caml_state->heap - (void *)old_heap);
  printf("old addr : %p, new addr : %p decalage %p\n", old_heap, Caml_state->heap, decalage);

  mlvalue *heap_pointer = Caml_state->heap;
  int64_t cpt_obj_vu = 0;

  while (cpt_obj_vu < cpt_obj_mem)
  { /* on parcours la memoire */
    header_t head_block = *heap_pointer;
    int64_t size = Size_hd(head_block);

    for (int64_t i = 0; i < size; i++)
    { /* parcours des valeurs du bloc */
      if (Is_block((heap_pointer + 1)[i]))
      { /* si on a un pointeur dans le bloc */
        // Recalcul de l'adresse
        // printf("\nancienne adresse %p\n",Ptr_val((heap_pointer + 1)[i]));
        // printf("Position relative : %d\n" , Ptr_val((heap_pointer + 1)[i])- old_heap);
        (heap_pointer + 1)[i] += decalage;
        // printf("nouvelle adresse %p\n",Ptr_val((heap_pointer + 1)[i]));
        // printf("Position relative : %d\n" , Ptr_val((heap_pointer + 1)[i])- Caml_state->heap);
      }
    }
    cpt_obj_vu++;

    heap_pointer += (size + 1);
  }

  for (int i = 0; i < Caml_state->sp; i++)
  {
    mlvalue *stack = Caml_state->stack;
    if (Is_block(stack[i]))
    {
      // Recalcul de la nouvelle adresse : new = old + decalage (voir comment bien gérer l'arithmétique des pointeurs..)
      stack[i] += decalage;
    }

  } /* fin de la modif de la pile */

  if (Is_block(Caml_state->accu))
  {
    // Recalcul de la nouvelle adresse : new = old + decalage (voir comment bien gérer l'arithmétique des pointeurs..)
    Caml_state->accu += decalage;
  }
  printf("ancienne adresse de env %p\n",Ptr_val(Caml_state->env));
  printf("Position relative : %d\n" , Ptr_val(Caml_state->env)- old_heap);

  Caml_state->env += decalage;
  printf("nouvelle adresse de env %p\n",Ptr_val(Caml_state->env));
  printf("Position relative : %d\n" , Ptr_val(Caml_state->env)- Caml_state->heap);

  // Recalcul de la nouvelle adresse : new = old + decalage (voir comment bien gérer l'arithmétique des pointeurs..)
}
