#ifndef OBJECTS_H
#define OBJECTS_H
#include "utils.h"
#include <stdbool.h>
#ifndef __cplusplus
typedef enum ObjectClass ObjectClass;
typedef struct Object Object;
typedef struct Type Type;
#endif

enum ObjectClass {
    OC_HEAD,
    OC_TYPE,      // Type
    OC_RECORD,    // Record
    OC_FIELD,     // Field
    OC_ARRAY,     // Array
    OC_CONST,     // const
    OC_VAR,       // variable
    OC_PROCEDURE,
    OC_PARAMETER,         // procedure reference, marked with 'var' keyword
    OC_BUILTIN_PROCEDURE, // can be used in an expression
    OC_COUNT
};

struct Object {
    ObjectClass klass;
    Object      *next;
    Object      *parent;
    Object      *child;
    char        name[MAX_STRLEN];
    int         level;
    bool        is_param;
    Type        *type;

    union {
        struct { // OC_CONST for bool and integer
            int value;
        } konst;
        struct { // OC_VAR
            int address_offset;
        } var;
        struct { // OC_PROCEDURE
            int entry_point_offset; // -1 if not known
        } procedure;
        struct { // OC_BUILTIN_PROCEDURE
            int function_number; // index into table
        } builtin_procedure;
        struct { // OC_PARAMETER
            int address_offset;
        } parameter;
        struct { // OC_FIELD
            int offset;
        } field;
    }/*as*/;
};

Object *object_insert(Object **list);
Object *object_append(Object **list);
Object *object_find(Object **list, const char *name);

#endif
