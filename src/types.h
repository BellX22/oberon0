#ifndef TYPES_H
#define TYPES_H
#ifndef __cplusplus
typedef enum TypeForm TypeForm;
typedef struct Type Type;
typedef struct Object Object;
#endif

enum TypeForm
{
    TF_BOOL,
    TF_INT,
    TF_ARRAY,
    TF_RECORD,
    TF_COUNT,
};

struct Type
{
    TypeForm form;
    int      size;

    union 
    {
        struct {
            Object* fields;
        } record;

        struct {
            int   len;
            Type* base;
        } array;
    };
};

extern Type BoolType;
extern Type IntType;

#endif
