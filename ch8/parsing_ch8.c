#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>

#include "mpc.h"


/* add Windows portability */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif
/**/

/* Lispy Types */
// Declare new lisp value 'lval' struct. Added union type for num and
// error, it will never need both at the same time.
/* typedef union { */
/*   long i; */
/*   double f; */
/* } number; */

typedef struct {
  int type;
  union {
    double num;
    int err;
  };
} lval;

// enumeration of possible lval types.
enum lval_t { LVAL_NUM, LVAL_ERR, LVAL_FLOAT };

// enumeration of possible error types.
enum lval_err_t { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

// Create number type.
lval lval_num(double x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

/* //Create floating point number type. */
/* lval lval_float(double x) { */
/*   lval v; */
/*   v.type = LVAL_FLOAT; */
/*   v.num = x; */
/*   return v; */
/* } */

// Create error type.
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}


/* The Eval */
// Helper function for 'eval()'. Will cary out the operation for the
// given string.
lval eval_op(lval x, char* op, lval y){
  // If there is an error, return it.
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }

  // Do maths with da numbers.
  if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }

  if (strcmp(op, "/") == 0) {
    // return error on division by zero.
    return y.num == 0
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(x.num / y.num);
  }

  if (strcmp(op, "^") == 0) {
    return lval_num(pow(x.num, y.num));
  }
  
  // Type generic '%' remainder op.
  if (strcmp(op, "%") == 0) {
    return lval_num(remainder(x.num, y.num));
  }
  
  
   /* Native Functions */
  // Type generic 'fmod()' function.
  if (strcmp(op, "mod") == 0) {
    return lval_num(fmod(x.num, y.num));
  }

  // 'min' and 'max' use type-generic functions.
    if (strcmp(op, "min") == 0) {
    return lval_num(fmin(x.num, y.num));
  }

  if (strcmp(op, "max") == 0) {
    return lval_num(fmax(x.num, y.num));
  }
  
  // Return error for unmatched operand.
  return lval_err(LERR_BAD_OP);
}


// The main 'eval' function defined here.
lval eval(mpc_ast_t* tree) {

  // If tag is "number", return it.
  if (strstr(tree->tag, "number")) {
    // Check for conversion error.
    errno = 0;
    double x = strtod(tree->contents, NULL);

    return errno != ERANGE
      ? lval_num(x)
      : lval_err(LERR_BAD_NUM);
  }
        
  // Operator always comes after open-parens, ie: 2nd child
  // (index 1).
  char* op = tree->children[1]->contents;
        
  // Store 3rd child (index 2) in 'x'
  lval x = eval(tree->children[2]);
        
  // Iterate the remaining children and combine them, starting
  // with the 4th child (index 3).
  int i = 3;
  while (strstr(tree->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(tree->children[i]));
    i++;
  }

  // Check for '-' with single argument.
  if (strstr(op, "-") && !strstr(tree->children[3]->tag, "expr")) {

    return x = lval_num(-x.num);
  }
  
  return x;
}


/* Print 'lval' types with these two functions */
void lval_print(lval v) {
  switch (v.type){
    // Print if type is 'number'.
  case LVAL_NUM: printf("%f", v.num);
    break;
    // if type is 'err' (error type).
  case LVAL_ERR:
    // determine what kind of error.
    if(v.err == LERR_DIV_ZERO) {
      printf("Error: Can't d1vide by zero, foo!?.");
    }
    if (v.err == LERR_BAD_OP) {
      printf("Error: Youre invuhlyd operator is invuhlyd, foo!?.");
    }
    if (v.err == LERR_BAD_NUM) {
      printf("Error: You heard I like bad numbers, foo!?.");
    }
    break;
  }
}

// 'lval_print()' with trailing newline
void lval_println(lval v) {
  lval_print(v);
  putchar('\n');
}


int main(int argc, char** argv) {

  /* Create Parsers*/
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  
  /* Define grammars */
  mpca_lang(MPCA_LANG_DEFAULT,
            "\
            number : /-?[0-9]+.?[0-9]*/ ;                    \
            operator : '+' | '-' | '*' |                     \
                       '/' | '%' | '^' |                     \
                       /min/ | /max/ | /mod/ ;               \
                                                             \
            expr : <number> | '(' <operator> <expr>+ ')' ;   \
            lispy : /^/ <operator> <expr>+ /$/ ;             \
            "
            , Number, Operator, Expr, Lispy);
  
  puts("Lispy version 0.0.0.0.4");
  puts("Press Ctrl+c to Exit\n");


  /* THE Loop */
  while (1) {
    char* input = readline("pew pew nya~> ");
    add_history(input);

    /* The Read */
    // fAttempt to parse input, and store results in 'r'
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
        
      /* The Print */
    // Call the eval function.
    lval result = eval(r.output);
    lval_println(result);
    mpc_ast_delete(r.output);
    
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  /* Undefine and Delete our Parsers */
  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  
  return 0;
}
