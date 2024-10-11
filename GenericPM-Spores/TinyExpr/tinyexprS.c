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

/* COMPILE TIME OPTIONS */

/* Exponentiation associativity:
For a^b^c = (a^b)^c and -a^b = (-a)^b do nothing.
For a^b^c = a^(b^c) and -a^b = -(a^b) uncomment the next line.*/
/* #define TE_POW_FROM_RIGHT */

/* Logarithms
For log = base 10 log do nothing
For log = natural log uncomment the next line. */
/* #define TE_NAT_LOG */

#include "tinyexpr.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifndef NAN
#define NAN (0.0/0.0)
#endif

typedef double (*te_fun2)(double, double);

enum {
    TOK_NULL = TE_CLOSURE7S+1, TOK_ERROR, TOK_END, TOK_SEP,
    TOK_OPEN, TOK_CLOSE, TOK_NUMBER, TOK_VARIABLE, TOK_INFIX
};


enum {TE_CONSTANT = 1};


typedef struct state {
    const char *start;
    const char *next;
    int type;
    union {double value; const double *bound; const void *function;};
    void *context;

    const te_variableS *lookup;
    int lookup_len;
} state;


#define TYPE_MASK(TYPE) ((TYPE)&0x0000001F)

#define IS_PURE(TYPE) (((TYPE) & TE_FLAG_PURES) != 0)
#define IS_FUNCTION(TYPE) (((TYPE) & TE_FUNCTION0S) != 0)
#define IS_CLOSURE(TYPE) (((TYPE) & TE_CLOSURE0S) != 0)
#define ARITY(TYPE) ( ((TYPE) & (TE_FUNCTION0S | TE_CLOSURE0S)) ? ((TYPE) & 0x00000007) : 0 )
#define NEW_EXPR(type, ...) new_expr((type), (const te_exprS*[]){__VA_ARGS__})

static te_exprS *new_expr(const int type, const te_exprS *parameters[]) {
    const int arity = ARITY(type);
    const int psize = sizeof(void*) * arity;
    const int size = (sizeof(te_exprS) - sizeof(void*)) + psize + (IS_CLOSURE(type) ? sizeof(void*) : 0);
    te_exprS *ret = malloc(size);
    memset(ret, 0, size);
    if (arity && parameters) {
        memcpy(ret->parameters, parameters, psize);
    }
    ret->type = type;
    ret->bound = 0;
    return ret;
}


void te_freeS_parametersS(te_exprS *n) {
    if (!n) return;
    switch (TYPE_MASK(n->type)) {
        case TE_FUNCTION7S: case TE_CLOSURE7S: te_freeS(n->parameters[6]);
        case TE_FUNCTION6S: case TE_CLOSURE6S: te_freeS(n->parameters[5]);
        case TE_FUNCTION5S: case TE_CLOSURE5S: te_freeS(n->parameters[4]);
        case TE_FUNCTION4S: case TE_CLOSURE4S: te_freeS(n->parameters[3]);
        case TE_FUNCTION3S: case TE_CLOSURE3S: te_freeS(n->parameters[2]);
        case TE_FUNCTION2S: case TE_CLOSURE2S: te_freeS(n->parameters[1]);
        case TE_FUNCTION1S: case TE_CLOSURE1S: te_freeS(n->parameters[0]);
    }
}


void te_freeS(te_exprS *n) {
    if (!n) return;
    te_freeS_parametersS(n);
    free(n);
}


static double pi() {return 3.14159265358979323846;}
static double e() {return 2.71828182845904523536;}

static const te_variableS functions[] = {
    /* must be in alphabetical order */
    {"abs", fabs,     TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"acos", acos,    TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"asin", asin,    TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"atan", atan,    TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"atan2", atan2,  TE_FUNCTION2S | TE_FLAG_PURES, 0},
    {"ceil", ceil,    TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"cos", cos,      TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"cosh", cosh,    TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"e", e,          TE_FUNCTION0S | TE_FLAG_PURES, 0},
    {"exp", exp,      TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"floor", floor,  TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"ln", log,       TE_FUNCTION1S | TE_FLAG_PURES, 0},
#ifdef TE_NAT_LOG
    {"log", log,      TE_FUNCTION1S | TE_FLAG_PURES, 0},
#else
    {"log", log10,    TE_FUNCTION1S | TE_FLAG_PURES, 0},
#endif
    {"log10", log10,  TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"pi", pi,        TE_FUNCTION0S | TE_FLAG_PURES, 0},
    {"pow", pow,      TE_FUNCTION2S | TE_FLAG_PURES, 0},
    {"sin", sin,      TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"sinh", sinh,    TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"sqrt", sqrt,    TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"tan", tan,      TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {"tanh", tanh,    TE_FUNCTION1S | TE_FLAG_PURES, 0},
    {0, 0, 0, 0}
};

static const te_variableS *find_builtin(const char *name, int len) {
    int imin = 0;
    int imax = sizeof(functions) / sizeof(te_variableS) - 2;

    /*Binary search.*/
    while (imax >= imin) {
        const int i = (imin + ((imax-imin)/2));
        int c = strncmp(name, functions[i].name, len);
        if (!c) c = '\0' - functions[i].name[len];
        if (c == 0) {
            return functions + i;
        } else if (c > 0) {
            imin = i + 1;
        } else {
            imax = i - 1;
        }
    }

    return 0;
}

static const te_variableS *find_lookup(const state *s, const char *name, int len) {
    int iters;
    const te_variableS *var;
    if (!s->lookup) return 0;

    for (var = s->lookup, iters = s->lookup_len; iters; ++var, --iters) {
        if (strncmp(name, var->name, len) == 0 && var->name[len] == '\0') {
            return var;
        }
    }
    return 0;
}



static double add(double a, double b) {return a + b;}
static double sub(double a, double b) {return a - b;}
static double mul(double a, double b) {return a * b;}
static double divide(double a, double b) {return a / b;}
static double negate(double a) {return -a;}
static double comma(double a, double b) {return b;}


void next_tokenS(state *s) {
    s->type = TOK_NULL;

    do {

        if (!*s->next){
            s->type = TOK_END;
            return;
        }

        /* Try reading a number. */
        if ((s->next[0] >= '0' && s->next[0] <= '9') || s->next[0] == '.') {
            s->value = strtod(s->next, (char**)&s->next);
            s->type = TOK_NUMBER;
        } else {
            /* Look for a variable or builtin function call. */
            if (s->next[0] >= 'a' && s->next[0] <= 'z') {
                const char *start;
                start = s->next;
                while ((s->next[0] >= 'a' && s->next[0] <= 'z') || (s->next[0] >= '0' && s->next[0] <= '9') || (s->next[0] == '_')) s->next++;

                const te_variableS *var = find_lookup(s, start, s->next - start);
                if (!var) var = find_builtin(start, s->next - start);

                if (!var) {
                    s->type = TOK_ERROR;
                } else {
                    switch(TYPE_MASK(var->type))
                    {
                        case TE_VARIABLES:
                            s->type = TOK_VARIABLE;
                            s->bound = var->address;
                            break;

                        case TE_CLOSURE0S: case TE_CLOSURE1S: case TE_CLOSURE2S: case TE_CLOSURE3S:
                        case TE_CLOSURE4S: case TE_CLOSURE5S: case TE_CLOSURE6S: case TE_CLOSURE7S:
                            s->context = var->context;

                        case TE_FUNCTION0S: case TE_FUNCTION1S: case TE_FUNCTION2S: case TE_FUNCTION3S:
                        case TE_FUNCTION4S: case TE_FUNCTION5S: case TE_FUNCTION6S: case TE_FUNCTION7S:
                            s->type = var->type;
                            s->function = var->address;
                            break;
                    }
                }

            } else {
                /* Look for an operator or special character. */
                switch (s->next++[0]) {
                    case '+': s->type = TOK_INFIX; s->function = add; break;
                    case '-': s->type = TOK_INFIX; s->function = sub; break;
                    case '*': s->type = TOK_INFIX; s->function = mul; break;
                    case '/': s->type = TOK_INFIX; s->function = divide; break;
                    case '^': s->type = TOK_INFIX; s->function = pow; break;
                    case '%': s->type = TOK_INFIX; s->function = fmod; break;
                    case '(': s->type = TOK_OPEN; break;
                    case ')': s->type = TOK_CLOSE; break;
                    case ',': s->type = TOK_SEP; break;
                    case ' ': case '\t': case '\n': case '\r': break;
                    default: s->type = TOK_ERROR; break;
                }
            }
        }
    } while (s->type == TOK_NULL);
}


static te_exprS *list(state *s);
static te_exprS *expr(state *s);
static te_exprS *power(state *s);

static te_exprS *base(state *s) {
    /* <base>      =    <constant> | <variable> | <function-0> {"(" ")"} | <function-1> <power> | <function-X> "(" <expr> {"," <expr>} ")" | "(" <list> ")" */
    te_exprS *ret;
    int arity;

    switch (TYPE_MASK(s->type)) {
        case TOK_NUMBER:
            ret = new_expr(TE_CONSTANT, 0);
            ret->value = s->value;
            next_tokenS(s);
            break;

        case TOK_VARIABLE:
            ret = new_expr(TE_VARIABLES, 0);
            ret->bound = s->bound;
            next_tokenS(s);
            break;

        case TE_FUNCTION0S:
        case TE_CLOSURE0S:
            ret = new_expr(s->type, 0);
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[0] = s->context;
            next_tokenS(s);
            if (s->type == TOK_OPEN) {
                next_tokenS(s);
                if (s->type != TOK_CLOSE) {
                    s->type = TOK_ERROR;
                } else {
                    next_tokenS(s);
                }
            }
            break;

        case TE_FUNCTION1S:
        case TE_CLOSURE1S:
            ret = new_expr(s->type, 0);
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[1] = s->context;
            next_tokenS(s);
            ret->parameters[0] = power(s);
            break;

        case TE_FUNCTION2S: case TE_FUNCTION3S: case TE_FUNCTION4S:
        case TE_FUNCTION5S: case TE_FUNCTION6S: case TE_FUNCTION7S:
        case TE_CLOSURE2S: case TE_CLOSURE3S: case TE_CLOSURE4S:
        case TE_CLOSURE5S: case TE_CLOSURE6S: case TE_CLOSURE7S:
            arity = ARITY(s->type);

            ret = new_expr(s->type, 0);
            ret->function = s->function;
            if (IS_CLOSURE(s->type)) ret->parameters[arity] = s->context;
            next_tokenS(s);

            if (s->type != TOK_OPEN) {
                s->type = TOK_ERROR;
            } else {
                int i;
                for(i = 0; i < arity; i++) {
                    next_tokenS(s);
                    ret->parameters[i] = expr(s);
                    if(s->type != TOK_SEP) {
                        break;
                    }
                }
                if(s->type != TOK_CLOSE || i != arity - 1) {
                    s->type = TOK_ERROR;
                } else {
                    next_tokenS(s);
                }
            }

            break;

        case TOK_OPEN:
            next_tokenS(s);
            ret = list(s);
            if (s->type != TOK_CLOSE) {
                s->type = TOK_ERROR;
            } else {
                next_tokenS(s);
            }
            break;

        default:
            ret = new_expr(0, 0);
            s->type = TOK_ERROR;
            ret->value = NAN;
            break;
    }

    return ret;
}


static te_exprS *power(state *s) {
    /* <power>     =    {("-" | "+")} <base> */
    int sign = 1;
    while (s->type == TOK_INFIX && (s->function == add || s->function == sub)) {
        if (s->function == sub) sign = -sign;
        next_tokenS(s);
    }

    te_exprS *ret;

    if (sign == 1) {
        ret = base(s);
    } else {
        ret = NEW_EXPR(TE_FUNCTION1S | TE_FLAG_PURES, base(s));
        ret->function = negate;
    }

    return ret;
}

#ifdef TE_POW_FROM_RIGHT
static te_exprS *factor(state *s) {
    /* <factor>    =    <power> {"^" <power>} */
    te_exprS *ret = power(s);

    int neg = 0;
    te_exprS *insertion = 0;

    if (ret->type == (TE_FUNCTION1S | TE_FLAG_PURES) && ret->function == negate) {
        te_exprS *se = ret->parameters[0];
        free(ret);
        ret = se;
        neg = 1;
    }

    while (s->type == TOK_INFIX && (s->function == pow)) {
        te_fun2 t = s->function;
        next_tokenS(s);

        if (insertion) {
            /* Make exponentiation go right-to-left. */
            te_exprS *insert = NEW_EXPR(TE_FUNCTION2S | TE_FLAG_PURES, insertion->parameters[1], power(s));
            insert->function = t;
            insertion->parameters[1] = insert;
            insertion = insert;
        } else {
            ret = NEW_EXPR(TE_FUNCTION2S | TE_FLAG_PURES, ret, power(s));
            ret->function = t;
            insertion = ret;
        }
    }

    if (neg) {
        ret = NEW_EXPR(TE_FUNCTION1S | TE_FLAG_PURES, ret);
        ret->function = negate;
    }

    return ret;
}
#else
static te_exprS *factor(state *s) {
    /* <factor>    =    <power> {"^" <power>} */
    te_exprS *ret = power(s);

    while (s->type == TOK_INFIX && (s->function == pow)) {
        te_fun2 t = s->function;
        next_tokenS(s);
        ret = NEW_EXPR(TE_FUNCTION2S | TE_FLAG_PURES, ret, power(s));
        ret->function = t;
    }

    return ret;
}
#endif



static te_exprS *term(state *s) {
    /* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
    te_exprS *ret = factor(s);

    while (s->type == TOK_INFIX && (s->function == mul || s->function == divide || s->function == fmod)) {
        te_fun2 t = s->function;
        next_tokenS(s);
        ret = NEW_EXPR(TE_FUNCTION2S | TE_FLAG_PURES, ret, factor(s));
        ret->function = t;
    }

    return ret;
}


static te_exprS *expr(state *s) {
    /* <expr>      =    <term> {("+" | "-") <term>} */
    te_exprS *ret = term(s);

    while (s->type == TOK_INFIX && (s->function == add || s->function == sub)) {
        te_fun2 t = s->function;
        next_tokenS(s);
        ret = NEW_EXPR(TE_FUNCTION2S | TE_FLAG_PURES, ret, term(s));
        ret->function = t;
    }

    return ret;
}


static te_exprS *list(state *s) {
    /* <list>      =    <expr> {"," <expr>} */
    te_exprS *ret = expr(s);

    while (s->type == TOK_SEP) {
        next_tokenS(s);
        ret = NEW_EXPR(TE_FUNCTION2S | TE_FLAG_PURES, ret, expr(s));
        ret->function = comma;
    }

    return ret;
}


#define TE_FUN(...) ((double(*)(__VA_ARGS__))n->function)
#define M(e) te_evalS(n->parameters[e])


double te_evalS(const te_exprS *n) {
    if (!n) return NAN;

    switch(TYPE_MASK(n->type)) {
        case TE_CONSTANT: return n->value;
        case TE_VARIABLES: return *n->bound;

        case TE_FUNCTION0S: case TE_FUNCTION1S: case TE_FUNCTION2S: case TE_FUNCTION3S:
        case TE_FUNCTION4S: case TE_FUNCTION5S: case TE_FUNCTION6S: case TE_FUNCTION7S:
            switch(ARITY(n->type)) {
                case 0: return TE_FUN(void)();
                case 1: return TE_FUN(double)(M(0));
                case 2: return TE_FUN(double, double)(M(0), M(1));
                case 3: return TE_FUN(double, double, double)(M(0), M(1), M(2));
                case 4: return TE_FUN(double, double, double, double)(M(0), M(1), M(2), M(3));
                case 5: return TE_FUN(double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4));
                case 6: return TE_FUN(double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5));
                case 7: return TE_FUN(double, double, double, double, double, double, double)(M(0), M(1), M(2), M(3), M(4), M(5), M(6));
                default: return NAN;
            }

        case TE_CLOSURE0S: case TE_CLOSURE1S: case TE_CLOSURE2S: case TE_CLOSURE3S:
        case TE_CLOSURE4S: case TE_CLOSURE5S: case TE_CLOSURE6S: case TE_CLOSURE7S:
            switch(ARITY(n->type)) {
                case 0: return TE_FUN(void*)(n->parameters[0]);
                case 1: return TE_FUN(void*, double)(n->parameters[1], M(0));
                case 2: return TE_FUN(void*, double, double)(n->parameters[2], M(0), M(1));
                case 3: return TE_FUN(void*, double, double, double)(n->parameters[3], M(0), M(1), M(2));
                case 4: return TE_FUN(void*, double, double, double, double)(n->parameters[4], M(0), M(1), M(2), M(3));
                case 5: return TE_FUN(void*, double, double, double, double, double)(n->parameters[5], M(0), M(1), M(2), M(3), M(4));
                case 6: return TE_FUN(void*, double, double, double, double, double, double)(n->parameters[6], M(0), M(1), M(2), M(3), M(4), M(5));
                case 7: return TE_FUN(void*, double, double, double, double, double, double, double)(n->parameters[7], M(0), M(1), M(2), M(3), M(4), M(5), M(6));
                default: return NAN;
            }

        default: return NAN;
    }

}

#undef TE_FUN
#undef M

static void optimize(te_exprS *n) {
    /* Evaluates as much as possible. */
    if (n->type == TE_CONSTANT) return;
    if (n->type == TE_VARIABLES) return;

    /* Only optimize out functions flagged as pure. */
    if (IS_PURE(n->type)) {
        const int arity = ARITY(n->type);
        int known = 1;
        int i;
        for (i = 0; i < arity; ++i) {
            optimize(n->parameters[i]);
            if (((te_exprS*)(n->parameters[i]))->type != TE_CONSTANT) {
                known = 0;
            }
        }
        if (known) {
            const double value = te_evalS(n);
            te_freeS_parametersS(n);
            n->type = TE_CONSTANT;
            n->value = value;
        }
    }
}


te_exprS *te_compileS(const char *expression, const te_variableS *variables, int var_count, int *error) {
    state s;
    s.start = s.next = expression;
    s.lookup = variables;
    s.lookup_len = var_count;

    next_tokenS(&s);
    te_exprS *root = list(&s);

    if (s.type != TOK_END) {
        te_freeS(root);
        if (error) {
            *error = (s.next - s.start);
            if (*error == 0) *error = 1;
        }
        return 0;
    } else {
        optimize(root);
        if (error) *error = 0;
        return root;
    }
}


double te_interpS(const char *expression, int *error) {
    te_exprS *n = te_compileS(expression, 0, 0, error);
    double ret;
    if (n) {
        ret = te_evalS(n);
        te_freeS(n);
    } else {
        ret = NAN;
    }
    return ret;
}

static void pn (const te_exprS *n, int depth) {
    int i, arity;
    printf("%*s", depth, "");

    switch(TYPE_MASK(n->type)) {
    case TE_CONSTANT: printf("%f\n", n->value); break;
    case TE_VARIABLES: printf("bound %p\n", n->bound); break;

    case TE_FUNCTION0S: case TE_FUNCTION1S: case TE_FUNCTION2S: case TE_FUNCTION3S:
    case TE_FUNCTION4S: case TE_FUNCTION5S: case TE_FUNCTION6S: case TE_FUNCTION7S:
    case TE_CLOSURE0S: case TE_CLOSURE1S: case TE_CLOSURE2S: case TE_CLOSURE3S:
    case TE_CLOSURE4S: case TE_CLOSURE5S: case TE_CLOSURE6S: case TE_CLOSURE7S:
         arity = ARITY(n->type);
         printf("f%d", arity);
         for(i = 0; i < arity; i++) {
             printf(" %p", n->parameters[i]);
         }
         printf("\n");
         for(i = 0; i < arity; i++) {
             pn(n->parameters[i], depth + 1);
         }
         break;
    }
}


void te_printS(const te_exprS *n) {
    pn(n, 0);
}
