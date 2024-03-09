#ifndef _INTERP_H
#define _INTERP_H

#include "mlvalues.h"

mlvalue caml_interprete(code_t* prog);
extern unsigned int sp;

#endif /* _INTERP_H */
