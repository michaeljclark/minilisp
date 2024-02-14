#pragma once

//======================================================================
// Lisp objects
//======================================================================

// The Lisp object type
enum {
    // Regular objects visible from the user
    TINT = 1,
    TCELL,
    TSYMBOL,
    TPRIMITIVE,
    TFUNCTION,
    TMACRO,
    TENV,
    // The marker that indicates the object has been moved to other location by GC. The new location
    // can be found at the forwarding pointer. Only the functions to do garbage collection set and
    // handle the object of this type. Other functions will never see the object of this type.
    TMOVED,
    TERROR,
    // Const objects. They are statically allocated and will never be managed by GC.
    TTRUE,
    TNIL,
    TDOT,
    TCPAREN,
};

// Typedef for the primitive function
struct Obj;
typedef struct Obj *Primitive(void *root, struct Obj **env, struct Obj **args);

// The object type
typedef struct Obj {
    // The first word of the object represents the type of the object. Any code that handles object
    // needs to check its type first, then access the following union members.
    int type;

    // The unaligned length of the object excluding "type" and "length" is stored to support
    // opaque octet strings that include '\0'. The total size of the object, including the
    // "type" and "length" field, this field, and the padding at the end of the object is
    // calculated using the obj_size function.
    int length;

    // Object values.
    union {
        // Int
        int value;
        // Cell
        struct {
            struct Obj *car;
            struct Obj *cdr;
        };
        // Symbol
        char name[1];
        // Primitive
        Primitive *fn;
        // Function or Macro
        struct {
            struct Obj *params;
            struct Obj *body;
            struct Obj *env;
        };
        // Environment frame. This is a linked list of association lists
        // containing the mapping from symbols to their value.
        struct {
            struct Obj *vars;
            struct Obj *up;
        };
        // Forwarding pointer
        void *moved;
    };
} Obj;

// Constants
extern Obj *True;
extern Obj *Nil;
extern Obj *Err;
extern Obj *Dot;
extern Obj *Cparen;
extern Obj *Symbols;

//======================================================================
// Logging
//======================================================================

static _Noreturn void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

static Obj* warning(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    return Err;
}

//======================================================================
// Library Interface
//======================================================================

extern void init(void **root, Obj **env);
extern void buffer_expr(const char *str, size_t length);
extern Obj *read_expr(void *root);
extern Obj *eval_expr(void *root, Obj **env, Obj **obj);
extern Obj *read_eval_expr(void *root, Obj **env, Obj **expr);
extern void print(Obj *obj);
