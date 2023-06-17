#ifndef GENERATOR_H
#define GENERATOR_H
#include <stdbool.h>
#include "objects.h"
#include "types.h"
#include "abstract_machine.h"
#ifndef __cplusplus
typedef struct Item Item;
typedef enum ItemMode ItemMode;
#endif

// must be in sync with ObjectClass
enum ItemMode
{
    IM_CONST = OC_CONST,
    IM_VAR = OC_VAR,
    IM_PROCEDURE_CALL = OC_PROCEDURE,
    IM_PARAMETER = OC_PARAMETER, // Actually a reference...
    IM_BUILTIN_PROCEDURE_CALL = OC_BUILTIN_PROCEDURE,
    IM_REGISTER = OC_COUNT,
    IM_REGISTER_INDIRECT,
    IM_CONDITION
};

struct Item
{
    ItemMode mode;
    Type*    type;
    int      level;

    int a, r;

    union  {
        struct {
            int value; // a
        } konst;

        struct {
            int reg;    // r - fixed base pointer register
            int offset; // a - address offset from reg
        } var;

        struct {
            int reg;
            int offset;
        } procedure_call;

        struct {
            int reg;     // r
            int offset;  // a
        } parameter;

        struct {

        } builtin_procedure_call;

        int reg;

        struct {
            int reg;    // r - base pointer register (not fixed)
            int offset; // a - address offset from reg
        } reg_indirect;

        struct {
            ConditionCode cond_code; // c - the condition
            int false_jump; // a
            int true_jump;  // b
        } condition;
    };
};

// Size of a pointer on the system
extern int generator_get_word_size();
extern int generator_get_program_counter();
extern int generator_get_current_level();

extern void generator_open();
extern void generator_header(int size);
extern void generator_close();

// generate machine code for procedures
extern void generator_enter(int parblksize, int locblksize);  // procedure entry
extern void generator_return(int size);                       // procedure exit
extern void generator_increase_level(int delta);
extern Item generator_parameter(Item x, ObjectClass klass);   // push params of procedure call
extern void generator_call(Item x);                           // call procedure

extern void generator_store(Item x, Item y);                  // x := y;
extern Item generator_array_index(Item array, Item index);    // x := x[y]
extern Item generator_field(Item record, Object* field);      // x := x.y
extern Item generator_op1(int op_token_kind, Item x);         // x := op x
extern Item generator_op2(int op_token_kind, Item x, Item y); // x := x op y
extern Item generator_relation(int op, Item x, Item y);       // x := x ? y

extern Item generator_cf_jump(Item x);                        // conditional forward jump
extern int  generator_f_jump(int relative_location);          // unconditional forward jump
extern Item generator_cb_jump(Item x, int location);          // conditional backward jump
extern void generator_b_jump(int location);                   // unconditional backward jump
extern void generator_fix_links(int location);

extern Item generator_make_item(Object* obj);
extern Item generator_make_const_item(TypeForm form, int value);

extern void generator_check_registers();

#endif
