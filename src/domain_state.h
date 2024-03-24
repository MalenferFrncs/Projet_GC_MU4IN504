#ifndef _DOMAIN_STATE_H
#define _DOMAIN_STATE_H

#include "mlvalues.h"


typedef struct _caml_domain_state {
  /* Stack */
  mlvalue* stack;
  /* Heap */
  mlvalue* heap;
  /* stack pointer */
  int64_t sp;
  /* environement */
  mlvalue env;
  /* accumulateur */
  mlvalue accu;

  /*Facteur de multiplication de la taille après realloc*/
  size_t heap_size;
} caml_domain_state;

/* The global state */
extern caml_domain_state* Caml_state;

/* Initialisation function for |Caml_state| */
void caml_init_domain();

void caml_free_domain(); //Ajout pour libérer correctement la mémoire du tas et de la pile

#endif
