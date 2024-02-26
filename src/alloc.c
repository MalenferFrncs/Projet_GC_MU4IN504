#include <stdlib.h>

#include "alloc.h"
#include "mlvalues.h"

mlvalue* caml_alloc(size_t size) {
  return aligned_alloc(8,size);
}
