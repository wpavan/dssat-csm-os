/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 *
 * Copyright (c) 2015, 2016 Lewis Van Winkle
 *
 * http://CodePlea.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgement in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef __TINYEXPRS_H__
#define __TINYEXPRS_H__


#ifdef __cplusplus
extern "C" {
#endif



typedef struct te_exprS {
    int type;
    union {double value; const double *bound; const void *function;};
    void *parameters[1];
} te_exprS;


enum {
    TE_VARIABLES = 0,

    TE_FUNCTION0S = 8, TE_FUNCTION1S, TE_FUNCTION2S, TE_FUNCTION3S,
    TE_FUNCTION4S, TE_FUNCTION5S, TE_FUNCTION6S, TE_FUNCTION7S,

    TE_CLOSURE0S = 16, TE_CLOSURE1S, TE_CLOSURE2S, TE_CLOSURE3S,
    TE_CLOSURE4S, TE_CLOSURE5S, TE_CLOSURE6S, TE_CLOSURE7S,

    TE_FLAG_PURES = 32
};

typedef struct te_variableS {
    const char *name;
    const void *address;
    int type;
    void *context;
} te_variableS;



/* Parses the input expression, evaluates it, and frees it. */
/* Returns NaN on error. */
double te_interp(const char *expression, int *error);

/* Parses the input expression and binds variables. */
/* Returns NULL on error. */
te_exprS *te_compileS(const char *expression, const te_variableS *variables, int var_count, int *error);

/* Evaluates the expression. */
double te_evalS(const te_exprS *n);

/* Prints debugging information on the syntax tree. */
void te_printS(const te_exprS *n);

/* Frees the expression. */
/* This is safe to call on NULL pointers. */
void te_freeS(te_exprS *n);


#ifdef __cplusplus
}
#endif

#endif /*__TINYEXPR_H__*/
