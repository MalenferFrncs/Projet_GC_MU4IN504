#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instruct.h"
#include "mlvalues.h"


/* Type for unassembled code */
typedef struct _ua_code_t {
  int instr;
  char* arg1;
  char* arg2; // No instruction have more than 2 arguments
} ua_code_t;

/* A linked list to store labels */
typedef struct _lbl_list {
  char* label;
  int instr;
  struct _lbl_list* next;
} lbl_list;



/* Inserts the label |label| in the linked list of labels |l|. */
void add_label(lbl_list** l, char* label, int instr) {
  lbl_list* new_head = malloc(sizeof(lbl_list));
  new_head->label = label;
  new_head->instr = instr;
  new_head->next  = *l;
  *l = new_head;
}

/* Returns the instruction associated with the label |label| in |l|. */
int label_addr(lbl_list* l, char* label) {
  while (l) {
    if (strcmp(l->label, label) == 0) {
      return l->instr;
    }
    l = l->next;
  }

  fprintf(stderr, "[Fatal error] Label not found: %s.\n", label);
  exit(EXIT_FAILURE);
}

/* Frees a linked list of labels. */
void free_labels(lbl_list* l) {
  while (l) {
    lbl_list* next = l->next;
    free(l);
    l = next;
  }
}


enum primitives { ADD, AND, DIV, EQ, GE, GT, LE, LT, MUL, NE, NOT, OR, PRINT, SUB };

int prim_of_str(char* buff) {
  if (strcmp(buff, "+") == 0)     return ADD;
  if (strcmp(buff, "&") == 0)     return AND;
  if (strcmp(buff, "/") == 0)     return DIV;
  if (strcmp(buff, "=") == 0)     return EQ;
  if (strcmp(buff, ">=") == 0)    return GE;
  if (strcmp(buff, ">") == 0)     return GT;
  if (strcmp(buff, "<=") == 0)    return LE;
  if (strcmp(buff, "<") == 0)     return LT;
  if (strcmp(buff, "*") == 0)     return MUL;
  if (strcmp(buff, "<>") == 0)    return NE;
  if (strcmp(buff, "not") == 0)   return NOT;
  if (strcmp(buff, "or") == 0)    return OR;
  if (strcmp(buff, "print") == 0) return PRINT;
  if (strcmp(buff, "-") == 0)     return SUB;

  fprintf(stderr, "[Fatal error] Unkown primitive: %s.\n", buff);
  exit(EXIT_FAILURE);
}

code_t* assemble(ua_code_t* ua_code, lbl_list* labels,
                 int instr_count, int code_size) {
  code_t* code = malloc(code_size * sizeof(code_t));

  int code_i = 0;
  for (int i = 0; i < instr_count; i++) {
    ua_code_t ua_instr = ua_code[i];

    code[code_i++] = ua_instr.instr;

    if (ua_instr.instr == PRIM) {
      code[code_i++] = prim_of_str(ua_instr.arg1);
    } else if (Arg1_is_label(ua_instr.instr)) {
      code[code_i++] = label_addr(labels, ua_instr.arg1);
    } else if (Has_arg1(ua_instr.instr)) {
      code[code_i++] = atoi(ua_instr.arg1);
    }

    if (Has_arg2(ua_instr.instr)) {
      code[code_i++] = atoi(ua_instr.arg2);
    }
    free(ua_instr.arg1);
    free(ua_instr.arg2);
  }

  free(ua_code);
  free_labels(labels);

  return code;
}



/* Returns the number of lines in |f|. */
unsigned int count_lines(FILE* f) {
  unsigned int count = 0;
  char c;
  while ((c = fgetc(f)) != EOF) {
    if (c == '\n') {
      count++;
    }
  }
  fseek(f, 0, SEEK_SET); // TODO: check fseek return?
  return count;
}

/* Reads characters from |f| until a newline is read */
void read_until_newline(FILE* f) {
  char c = fgetc(f);
  while (c != EOF && c != '\n') {
    c = fgetc(f);
  }
}


int instr_of_str(char* buff) {

#define CASE_INSTR(instr) if (strcmp(buff, #instr) == 0) return instr;

  CASE_INSTR(CONST);
  CASE_INSTR(PRIM);
  CASE_INSTR(BRANCH);
  CASE_INSTR(BRANCHIFNOT);
  CASE_INSTR(PUSH);
  CASE_INSTR(POP);
  CASE_INSTR(ACC);
  CASE_INSTR(ENVACC);
  CASE_INSTR(CLOSURE);
  CASE_INSTR(CLOSUREREC);
  CASE_INSTR(OFFSETCLOSURE);
  CASE_INSTR(APPLY);
  CASE_INSTR(RETURN);
  CASE_INSTR(STOP);
  CASE_INSTR(GRAB);
  CASE_INSTR(RESTART);
  CASE_INSTR(MAKEBLOCK);
  CASE_INSTR(GETFIELD);
  CASE_INSTR(VECTLENGTH);
  CASE_INSTR(GETVECTITEM);
  CASE_INSTR(SETFIELD);
  CASE_INSTR(SETVECTITEM);
  CASE_INSTR(ASSIGN);
  CASE_INSTR(PUSHTRAP);
  CASE_INSTR(POPTRAP);
  CASE_INSTR(RAISE);
  CASE_INSTR(APPTERM);

  fprintf(stderr, "[Fatal error] Unkown instruction: %s.\n", buff);
  exit(EXIT_FAILURE);
}



code_t* parse(char* filename) {

  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "[Fatal error] Cannot open bytecode file '%s'.\n", filename);
    exit(EXIT_FAILURE);
  }

  // Finding an upper bound to the number of instructions
  unsigned int line_count = count_lines(f);

  ua_code_t* code = malloc(line_count * sizeof(ua_code_t));

  int instr_count = 0, code_size = 0;
  lbl_list* labels = NULL;

  /* Reading input file; filling up |code| */
  while (1) {
    char buff[1024];

    char *arg1 = NULL, *arg2 = NULL;

    /* Parsing instruction */
    if (fscanf(f, " %1023s", buff) == EOF) {
      break;
    }
    int len = strlen(buff);
    if (buff[len-1] == ':') {
      buff[len-1] = '\0';
      add_label(&labels, strdup(buff), code_size);
      fscanf(f, " %1023s", buff);
    }

    int instr = instr_of_str(buff);

    /* Reading 1st argument */
    if (Has_arg1(instr)) {
      fscanf(f, " %1023s", buff);
      arg1 = strdup(buff);
      code_size++;
    }

    /* Reading second argument */
    if (Has_arg2(instr)) {
      arg1[strlen(arg1)-1] = '\0'; // removing trailing ',' for |arg1|
      fscanf(f, " %1023s", buff);
      arg2 = strdup(buff);
      code_size++;
    }

    read_until_newline(f);

    code[instr_count++] = (ua_code_t){ .instr = instr, .arg1 = arg1,
                                       .arg2 = arg2 };
    code_size++;
  }

  return assemble(code, labels, instr_count, code_size);
}
