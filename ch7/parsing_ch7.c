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


/* The Eval */
// Helper function for 'eval()'. Will cary out the operation for the
// given string.
long eval_op(long x, char* op, long y){
  if (strcmp(op, "+") == 0) { return x + y; }
  if (strcmp(op, "-") == 0) { return x - y; }
  if (strcmp(op, "*") == 0) { return x * y; }
  if (strcmp(op, "/") == 0) { return x / y; }
  if (strcmp(op, "^") == 0) { return pow(x, y); }
  //Type generic 'fmod()' function.
  if (strcmp(op, "%") == 0) { return fmod(x, y); }
  // Native functions.
  // min and max use type-generic functions.
  if (strcmp(op, "min") == 0) { return fmin(x,y); }
  if (strcmp(op, "max") == 0) { return fmax(x,y); }
  
  return 0;
}

// The main 'eval' function defined here.
long eval(mpc_ast_t* tree) {
  // If tag is "number", return it.
  if (strstr(tree->tag, "number")) {
    return atoi(tree->contents);
  }
        
  // Operator always comes after open-parens, ie: 2nd child
  // (index 1).
  char* op = tree->children[1]->contents;
        
  // Store 3rd child (index 2) in 'x'
  long x = eval(tree->children[2]);
        
  // Iterate the remaining children and combine them, starting
  // with the 4th child (index 3).
  int i = 3;
  while (strstr(tree->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(tree->children[i]));
    i++;
  }

  // Check for '-' with single argument.
  if (strstr(op, "-") && !strstr(tree->children[3]->tag, "expr")) {

    return - x;
  }
  
  return x;
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
            number : /-?[0-9]+/ ;                           \
            operator : '+' | '-' | '*' |                    \
                       '/' | '%' | '^' |                    \
                                                            \
                       /min/ | /max/ ;                      \
                                                            \
            expr : <number> | '(' <operator> <expr>+ ')' ;  \
            lispy : /^/ <operator> <expr>+ /$/ ;            \
            "
            , Number, Operator, Expr, Lispy);
  
  puts("Lispy version 0.0.0.0.3");
  puts("Press Ctrl+c to Exit\n");


  /* THE Loop */
  while (1) {
    char* input = readline("pew pew nya~> ");
    add_history(input);

    /* The Read */
    // Attempt to parse input, and store results in 'r'
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
        
      /* The Print */
    // Call the eval function.
    long result = eval(r.output);
    printf("%li\n", result);
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
