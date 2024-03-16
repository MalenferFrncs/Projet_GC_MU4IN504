#include <stdlib.h>

#include "domain_state.h"
#include "config.h"
#include "mlvalues.h"


caml_domain_state* Caml_state;

void caml_init_domain() {

  Caml_state = malloc(sizeof(caml_domain_state));

  Caml_state->stack = malloc(Stack_size);

  Caml_state->heap = malloc(Heap_size);

  Caml_state->sp = 0;

  Caml_state->env = Make_empty_env();

  Caml_state->accu = Val_long(0);
  
}
