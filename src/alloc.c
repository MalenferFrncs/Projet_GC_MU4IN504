#include <stdlib.h>
#include <stdio.h>

#include "alloc.h"
#include "mlvalues.h"
#include "config.h"
#include "domain_state.h"
#include "interp.h"
#include "debug.h"

int heap_free = 0;

/*Fonction appelée par l'interprèteur de bytecode à chaque allocation*/
mlvalue *caml_alloc(size_t size)
{
  if (heap_free + size >= (Caml_state->heap_size) / sizeof(mlvalue)) // Les mlvalues étant des entiers 64 bits, le nombre de cases est égal à Caml_state->heap_size/8
  {
#ifdef DEBUG
    printf("Appel au GC\n");
#endif
    int nb_obj = mark_and_compact();

    while (heap_free + size >= (Caml_state->heap_size) / sizeof(mlvalue)) // Malloc n'a pas suffit...
    {
#ifdef DEBUG
      printf("Appel à realloc\n");
#endif
      caml_realloc(nb_obj);
    }
  }
  mlvalue *addr_block = Caml_state->heap + heap_free;
  heap_free += size;
  return addr_block;
}

int mark_and_compact()
{
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
  nb_obj_mem = mark(stack); /* mark */
  mlvalue ***tab_mem_addr = make_tab_addr(nb_obj_mem);
  heap_free = calc_new_addr(tab_mem_addr, nb_obj_mem); /*compact P1 calcules les nvls addrs */
  apply_new_addr(tab_mem_addr, nb_obj_mem, stack);     /*compact P2 modification de toute les valeurs de la memoire vivante */
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
  free_tab_addr(tab_mem_addr);
  return nb_obj_mem;
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
        set_color((ptr_block - 1), GRAY);
        enfile(&todo, ptr_block);
        cpt_obj_mem++;
      }
    }

    set_color((block - 1), BLACK);
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
      tab_mem_addr[0][i] = (mlvalue *)heap_pointer;     /*old addr*/
      tab_mem_addr[1][i] = (mlvalue *)current_heap_top; /*new addr*/

      current_heap_top += (size + 1);
      new_heap_free += (size + 1);
      cpt_obj_vu++;
      i++;
    }
    else
    {
    }
    heap_pointer += (size + 1);
  }
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
        if ((((mlvalue *)stack[i]) - 1) == (tab_mem_addr)[0][j])
        {
          stack[i] = (((tab_mem_addr)[1][j]) + 1);

          break;
        }
      }
    }

  } /* fin de la modif de la pile */

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
      if ((((mlvalue *)Caml_state->accu) - 1) == (tab_mem_addr)[0][j])
      {
        Caml_state->accu = (((tab_mem_addr)[1][j]) + 1);

        break;
      }
    }
  }
  if (Is_block(Caml_state->env))
  {

    for (int j = 0; j < cpt_obj_mem; j++)
    { /* recherche de la nouvel adresse du bloc pointé */
      if ((((mlvalue *)Caml_state->env) - 1) == (tab_mem_addr)[0][j])
      {
        Caml_state->env = (((tab_mem_addr)[1][j]) + 1);

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
  }
}

/*Fonction de réallocation du tas lorsque celui ci n'est pas assez grand*/
void caml_realloc(int64_t cpt_obj_mem)
{
  mlvalue *old_heap = Caml_state->heap;
  Caml_state->heap_size = Caml_state->heap_size * 2;
  Caml_state->heap = realloc(Caml_state->heap, Caml_state->heap_size);
  if (old_heap == Caml_state->heap)
  {
    return; // Rien à faire
  }
  int64_t decalage = ((void *)Caml_state->heap - (void *)old_heap);
  // printf("old addr : %p, new addr : %p decalage %p\n", old_heap, Caml_state->heap, decalage);

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
#ifdef DEBUG_REALLOC
        printf("\nancienne adresse %p\n", Ptr_val((heap_pointer + 1)[i]));
        printf("Position relative : %d\n", Ptr_val((heap_pointer + 1)[i]) - old_heap);
#endif
        (heap_pointer + 1)[i] += decalage;
#ifdef DEBUG_REALLOC
        printf("nouvelle adresse %p\n", Ptr_val((heap_pointer + 1)[i]));
        printf("Position relative : %d\n", Ptr_val((heap_pointer + 1)[i]) - Caml_state->heap);
#endif
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
      // Recalcul de la nouvelle adresse : new = old + decalage
      stack[i] += decalage;
    }

  } /* fin de la modif de la pile */

  if (Is_block(Caml_state->accu))
  {
    // Recalcul de la nouvelle adresse : new = old + decalage
    Caml_state->accu += decalage;
  }
#ifdef DEBUG_REALLOC
  printf("ancienne adresse de env %p\n", Ptr_val(Caml_state->env));
  printf("Position relative : %d\n", Ptr_val(Caml_state->env) - old_heap);
#endif
  Caml_state->env += decalage;
#ifdef DEBUG_REALLOC

  printf("nouvelle adresse de env %p\n", Ptr_val(Caml_state->env));
  printf("Position relative : %d\n", Ptr_val(Caml_state->env) - Caml_state->heap);
#endif
}

/*Helpers*/

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
      set_color((ptr_block - 1), GRAY);
      if (Color((stack[i])) == WHITE)
      {
        printf("is still white");
      }
      enfile(racine, ptr_block);
      cpt_obj_mem++;
    }
  }
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
