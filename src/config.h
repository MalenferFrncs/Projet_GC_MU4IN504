#ifndef _CONFIG_H
#define _CONFIG_H

#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

/* The size of the stack. */
/* Note that it doesn't hold 8MB mlvalues, but rather 8MB of bytes. */
/* No boundary-checks are done: stack overflow will silently corrupt
   the heap; probably causing something to go wrong somewhere. */
/* TODO: auto-growing stack, or throw stack overflow when needed. */

/*NOTE : la taille du tas nécessaire pour certains de nos tests est beaucoup plus grande que pour d'autres
afin de constater des exemples où le GC est appelé pour des petits exemples, il faut modifier la valeur dans Heap_size

   * Pour tester avec avec bench/list_4, il faut au minimum 512*KB
   * Pour tester avec bench/list_{1,2,3}, il faut au minimum 256*KB
   * Pour les autres programmes, il faut au minimum 512.
*/
#define Stack_size (8 * MB)
#define Heap_size ( 512*KB)

/*Décommenter pour avoir les affichages à l'exécution des tests, et pour que la réallocation dynamique se mette en marche*/
//#define DEBUG
//#define DEBUG_REALLOC
//#define REALLOC

#endif
