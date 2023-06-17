#ifndef ABSTRACT_MACHINE_H
#define ABSTRACT_MACHINE_H
#include <stdbool.h>
typedef int reg_index;
#ifndef __cplusplus
typedef enum Operation Operation;
typedef enum ConditionCode ConditionCode;
#endif

enum Operation
{
    OP_MOV,
    OP_NOT, // Bitwise
    OP_AND, // Bitwise
    OP_OR,  // Bitwise
    OP_XOR, // Bitwise
    OP_LSH, // Bitwise
    OP_RSH, // Bitwise
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_CMP,
};

enum ConditionCode
{
    CC_TRUE,
    CC_FALSE,
    CC_EQUAL,
    CC_NOT_EQUAL,
    CC_LESS,
    CC_LESS_EQUAL,
    CC_GREATER,
    CC_GREATER_EQUAL,
};
// ----------------------------------------------------------------------------------
// Convenience API
// ----------------------------------------------------------------------------------
extern ConditionCode negate_condition(ConditionCode cc);
extern int  am_get_pc(void);
extern int  am_get_jump_location(int loc);
extern void am_fix_jump(int at, int with);
extern void am_emit_operation(Operation op, reg_index a, reg_index b, reg_index c);
extern void am_emit_operation_im(Operation op, reg_index a, reg_index b, int value);
extern void am_emit_c_jump_im(ConditionCode cc, int relative);
extern void am_emit_c_jump(ConditionCode cc, reg_index reg);
// ----------------------------------------------------------------------------------
extern void am_emit_label(const char* name); // just for notes
// ----------------------------------------------------------------------------------
// Specific API
// ----------------------------------------------------------------------------------
extern void am_emit_mov(reg_index dest, reg_index src);                   // format 0
extern void am_emit_cmp(reg_index reg1, reg_index reg2);                  // format 0
extern void am_emit_and(reg_index dest, reg_index lhs, reg_index rhs);    // format 0
extern void am_emit_or (reg_index dest, reg_index lhs, reg_index rhs);    // format 0
extern void am_emit_xor(reg_index dest, reg_index lhs, reg_index rhs);    // format 0
extern void am_emit_add(reg_index dest, reg_index lhs, reg_index rhs);    // format 0
extern void am_emit_sub(reg_index dest, reg_index lhs, reg_index rhs);    // format 0
extern void am_emit_mul(reg_index dest, reg_index lhs, reg_index rhs);    // format 0
extern void am_emit_div(reg_index dest, reg_index lhs, reg_index rhs);    // format 0
extern void am_emit_lsh(reg_index dest, reg_index lhs, reg_index rhs);    // format 0
extern void am_emit_rsh(reg_index dest, reg_index lhs, reg_index rhs);    // format 0
extern void am_emit_mod(reg_index dest, reg_index lhs, reg_index rhs);    // format 0
// ----------------------------------------------------------------------------------
extern void am_emit_mov_im(reg_index dest, int value);                    // format 1
extern void am_emit_cmp_im(reg_index reg, int value);                     // format 1
extern void am_emit_and_im(reg_index dest, reg_index lhs, int rhs_value); // format 1
extern void am_emit_or_im (reg_index dest, reg_index lhs, int rhs_value); // format 1
extern void am_emit_xor_im(reg_index dest, reg_index lhs, int rhs_value); // format 1
extern void am_emit_add_im(reg_index dest, reg_index lhs, int rhs_value); // format 1
extern void am_emit_sub_im(reg_index dest, reg_index lhs, int rhs_value); // format 1
extern void am_emit_mul_im(reg_index dest, reg_index lhs, int rhs_value); // format 1
extern void am_emit_div_im(reg_index dest, reg_index lhs, int rhs_value); // format 1
extern void am_emit_lsh_im(reg_index dest, reg_index lhs, int rhs_value); // format 1
extern void am_emit_rsh_im(reg_index dest, reg_index lhs, int rhs_value); // format 1
extern void am_emit_mod_im(reg_index dest, reg_index lhs, int rhs_value); // format 1
// ----------------------------------------------------------------------------------
extern void am_emit_load(reg_index dest, reg_index base_reg, int offset); // format 2
extern void am_emit_store(reg_index src, reg_index base_reg, int offset); // format 2
// ----------------------------------------------------------------------------------
extern void am_emit_jump(reg_index reg);                                  // format 3
extern void am_emit_jump_equal(reg_index reg);                            // format 3
extern void am_emit_jump_not_equal(reg_index reg);                        // format 3
extern void am_emit_jump_less(reg_index reg);                             // format 3
extern void am_emit_jump_less_equal(reg_index reg);                       // format 3
extern void am_emit_jump_greater(reg_index reg);                          // format 3
extern void am_emit_jump_greater_euqal(reg_index reg);                    // format 3
// ----------------------------------------------------------------------------------
extern void am_emit_jump_im(int relative);                                // format 3
extern void am_emit_jump_equal_im(int relative);                          // format 3
extern void am_emit_jump_not_equal_im(int relative);                      // format 3
extern void am_emit_jump_less_im(int relative);                           // format 3
extern void am_emit_jump_less_equal_im(int relative);                     // format 3
extern void am_emit_jump_greater_im(int relative);                        // format 3
extern void am_emit_jump_greater_euqal_im(int relative);                  // format 3

#endif // ABSTRACT_MACHINE_H
