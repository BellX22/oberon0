#include "generator.h"
#include "objects.h"
#include "types.h"
#include "scanner.h"
#include "abstract_machine.h"
#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <stdarg.h>

static int g_entry = 0;
static int g_current_level = 0;

int generator_get_current_level() { return g_current_level; }
int generator_get_program_counter() { return am_get_pc(); }
int generator_get_word_size() { return 4; }
void generator_increase_level(int delta) { g_current_level += delta; }

static int R = 0;          // Current Register Index (Stack Machine)
static const int GB = 13;  // Global Base Register
static const int SP = 14;  // Stack Pointer Register
static const int LNK = 15; // Link Register/Frame pointer
static const int StackBase = 0xffffffc0; // initialize stack pointer

static Item
load(Item item)
{
    if(IM_REGISTER == item.mode)
        return item;
    if(IM_CONST == item.mode) {
        am_emit_mov_im(R, item.konst.value);
        item.mode = IM_REGISTER;
        item.reg = R;
        R += 1;
    } else if(IM_VAR == item.mode) {
        am_emit_load(R, item.var.reg, item.var.offset);
        item.mode = IM_REGISTER;
        item.reg = R;
        R += 1;
    } else if(IM_PARAMETER == item.mode) {
        am_emit_load(R, item.parameter.reg, item.parameter.offset);
        am_emit_load(R, R, 0);
        item.mode = IM_REGISTER;
        item.reg = R;
        R += 1;
    } else if(IM_REGISTER_INDIRECT == item.mode) {
        int reg = item.reg_indirect.reg;
        int offset = item.reg_indirect.offset;
        am_emit_load(reg, reg, offset);
        item.mode = IM_REGISTER;
        item.reg = reg;
    } else if(IM_CONDITION == item.mode) {
        am_emit_c_jump_im(negate_condition(item.condition.cond_code), 2);
        generator_fix_links(item.condition.true_jump);
        am_emit_mov_im(R, 1);
        am_emit_jump_im(1);
        generator_fix_links(item.condition.false_jump);
        am_emit_mov_im(R, 0);
        item.mode = IM_REGISTER;
        item.reg = R;
        R += 1;
    }
    return item;
}

static Item
load_address(Item item)
{
    if(item.mode == IM_VAR) {
        am_emit_add_im(R, item.var.reg, item.var.offset);
        R += 1;
    } else if(item.mode == IM_PARAMETER) {
        am_emit_load(R, item.parameter.reg, item.parameter.offset);
        R += 1;
    } else if(item.mode == IM_REGISTER_INDIRECT) {
        am_emit_add_im(item.reg_indirect.reg, item.reg_indirect.reg, item.reg_indirect.offset);
    } else {
        scanner_mark_error("address error");
    }
    item.mode = IM_REGISTER;
    item.reg = R - 1;
    return item;
}

static Item
load_condition(Item x)
{
    if(x.type->form == TF_BOOL) {
        if(x.mode == IM_CONST) {
            if(x.konst.value)
                x.condition.cond_code = CC_TRUE;
            else
                x.condition.cond_code = CC_FALSE;
            x.mode = IM_CONDITION;
        } else {
            x = load(x); // R += 1
            am_emit_cmp_im(x.reg, 0);
            R -= 1;
            x.mode = IM_CONDITION;
            x.condition.cond_code = CC_NOT_EQUAL;
            x.condition.false_jump = 0;
        }
    } else {
        scanner_mark_error("bool?");
    }
    return x;
}

void
generator_store(Item x, Item y) // x := y
{
    if(y.mode != IM_REGISTER)
        y = load(y);
    if(x.mode == IM_VAR) {
        am_emit_store(y.reg, x.var.reg, x.var.offset);
        R -= 1;
    } else if(x.mode == IM_PARAMETER) {
        am_emit_load(R, x.parameter.reg, x.parameter.offset);
        am_emit_store(y.reg, R, 0);
        R -= 1;
    } else if(x.mode == IM_REGISTER_INDIRECT) {
        am_emit_store(y.reg, x.reg_indirect.reg, x.reg_indirect.offset);
        R -= 2;
    } else {
        scanner_mark_error("illegal assignment");
    }
}

static Item
put_operation(Operation op, Item x, Item y)
{
    if(x.mode == IM_CONST) {// y.mode != IM_CONST
        // test range(x.a)
        y = load(y);
        if(op == OP_SUB || op == OP_CMP || op == OP_DIV || op == OP_MOD) {
            x = load(x);
            R -= 1;
            am_emit_operation(op, R-1, x.reg, y.reg); // for OP_CMP R-1 arg is ignored
        } else {
            am_emit_operation_im(op, R-1, y.reg, x.konst.value);
        }
    } else { // x.mode != IM_CONST
        x = load(x);
        if(y.mode == IM_CONST) {
            // test range(y.a)
            am_emit_operation_im(op, R-1, x.reg, y.konst.value);
        } else {
            y = load(y);
            am_emit_operation(op, R-2, x.reg, y.reg);
            R -= 1;
        }
    }
    x.mode = IM_REGISTER;
    x.reg = R - 1;
    return x;
}

void
generator_open()
{
    assert(false);
    // TODO@Andreas: is this the entry point for a code module??
    g_current_level = 0;
    //g_program_counter = 0;
    R = 0;
    //put3(2, 7, 0); // jump
    //put3(2, 7, 0); // jump here, used to reserve bytes
    //g_program_counter = 8;
    //first 16 words (32 bytes) in SRam reserved for boot loader
}

void
generator_header(int size)
{
    //TODO@Andreas: Module begin?
    //g_entry = am_get_pc();
    //am_fix_jump(0, am_get_pc() - 1);
    //am_emit_mov_im(GB, 0);
    //am_emit_mov_im(SP, StackBase);
}

void
generator_close()
{
    //TODO@Andreas: Module end?
    //am_emit_mov_im(0, 0);
    //am_emit_jump(0);
    //am_fix_jump(g_entry, (am_get_pc() + 7) / (8 * 32));
}

void
generator_enter(int parblksize, int locblksize)
{
    // TODO@Andreas: a = word size?
    int a = 4;
    int r = 0;
    am_emit_label("ProcedureStart");
    am_emit_sub_im(SP, SP, locblksize);
    am_emit_store(LNK, SP, 0);
    while(a < parblksize) {
        am_emit_store(r,SP,a);
        r += 1;
        a += 4;
    }
}

void
generator_return(int size)
{
    am_emit_load(LNK, SP, 0);
    am_emit_add_im(SP, SP, size);
    am_emit_jump(LNK);
    am_emit_label("ProcedureEnd");
}


Item
generator_field(Item record, Object* field) // x := x.y
{
    assert(record.type->form == TF_RECORD);
    if(IM_VAR == record.mode) {
        record.var.offset += field->field.offset;
    } else if (IM_REGISTER_INDIRECT == record.mode) {
        record.reg_indirect.offset += field->field.offset;
    } else if(IM_PARAMETER == record.mode) {
        assert(false);
        am_emit_load(R, record.r, record.a);
        record.mode = IM_REGISTER_INDIRECT;
        record.r = R;
        record.a = field->field.offset;
        R += 1;
    }
    return record;
}

Item
generator_array_index(Item array, Item index) // x := x[y]
{
    assert(array.type->form == TF_ARRAY);
    int base_size = array.type->array.base->size;
    if(index.mode == IM_CONST) {
        if(index.konst.value < 0
            || index.konst.value >= array.type->array.len)
        {
            scanner_mark_error("bad index");
        }
        if(array.mode == IM_PARAMETER) {
            assert(false);
            am_emit_load(R, array.parameter.reg, array.parameter.offset);
            array.mode = IM_REGISTER_INDIRECT;
            array.reg_indirect.offset = 0;
        }
        array.reg_indirect.offset += index.konst.value * base_size;
    } else {
        if(index.mode != IM_REGISTER)
            index = load(index);
        am_emit_mul_im(index.reg, index.reg, base_size);
        if(array.mode == IM_VAR) {
            am_emit_add(index.reg, array.var.reg, index.reg);
            //int reg = array.var.reg;
            int offset = array.var.offset;
            array.mode = IM_REGISTER_INDIRECT;
            array.reg_indirect.reg = index.reg;
            array.reg_indirect.offset = offset;
        } else if(array.mode == IM_PARAMETER) {
            assert(false);
        } else if(array.mode == IM_REGISTER_INDIRECT) {
            am_emit_add(array.reg_indirect.reg, array.reg_indirect.reg, index.reg);
            R -= 1;
        }
    }
    return array;
}

Item
generator_parameter(Item x, ObjectClass klass)
{
    if(klass == OC_PARAMETER) {
        x = load_address(x);
    } else {
        x = load(x);
    }
    return x;
}

void
generator_call(Item x)
{
    if(x.mode == IM_PROCEDURE_CALL) {
        // save LNK and jump = call
        // put3(3, 7, x.a - g_program_counter - 1)
        // R15 := PC + 1;
        am_emit_mov_im(LNK, am_get_pc() + 1); // must be saved at runtime?
        am_emit_jump_im(x.procedure_call.offset - am_get_pc() - 1);
    } else {
        assert(false); // BUILTIN_PROCEDURE_CALL?
        x = load(x);
        // put3(1, 14, x.reg);
        // R15 := PC + 1;
        am_emit_mov_im(LNK, am_get_pc() + 1); // must be saved at runtime?
        //am_emit_c_jump_reg(CC_GREATER, x.reg);
        R -= 1;
    }
    R = 0;
}

Item
generator_op1(int op, Item x) // x := op x
{
    if(op == TK_MINUS) {
        if(x.mode == IM_CONST) {
            x.konst.value = -x.konst.value;
        } else {
            if(x.mode == IM_VAR)
                x = load(x);
            am_emit_xor_im(x.reg, x.reg, -1);
            am_emit_add_im(x.reg, x.reg, 1);
        }
    } else if(op == TK_LOGIC_NOT) {
        if(x.mode != IM_CONDITION)
            x = load_condition(x);
        x.condition.cond_code = negate_condition(x.condition.cond_code);
        int tmp = x.condition.false_jump;
        x.condition.false_jump = x.condition.true_jump;
        x.condition.true_jump = tmp;
    } else if(op == TK_LOGIC_AND) {
        if(x.mode != IM_CONDITION)
            x = load_condition(x);
        am_emit_c_jump_im(negate_condition(x.condition.cond_code), x.condition.false_jump);
        x.condition.false_jump = generator_get_program_counter() - 1;
        generator_fix_links(x.condition.true_jump);
        x.condition.true_jump = 0;
    } else if(op == TK_LOGIC_OR) {
        if(x.mode != IM_CONDITION)
            x = load_condition(x);
        am_emit_c_jump_im(x.condition.cond_code, x.condition.true_jump);
        x.condition.true_jump = generator_get_program_counter() - 1;
        generator_fix_links(x.condition.false_jump);
        x.condition.false_jump = 0;
    }
    return x;
}

static int
merged(int l0, int l1)
{
    if(l0 != 0) {
        int l2 = l0;
        int l3 = 0;
        while(true) {
            int l3 = am_get_jump_location(l2);
            if(l3 == 0)
                break;
            l2 = l3;
        }
        am_fix_jump(l2, am_get_jump_location(l2) - l3 + l1);
        l1 = l0;
    }
    return l1;
}

Item
generator_op2(int op, Item x, Item y)
{
    if(x.type->form == TF_INT) {
        Operation o = 0;
        switch(op) {
        break;case TK_PLUS:  o = OP_ADD;
        break;case TK_MINUS: o = OP_SUB;
        break;case TK_TIMES: o = OP_MUL;
        break;case TK_DIV:   o = OP_DIV;
        break;case TK_MOD:   o = OP_MOD;
        break;default: assert(false); break;
        }
        if(x.mode == IM_CONST && y.mode == IM_CONST) {
            int result = 0;
            if(o == OP_ADD) result = x.konst.value + y.konst.value;
            else if(o == OP_SUB) result = x.konst.value - y.konst.value;
            else if(o == OP_MUL) result = x.konst.value * y.konst.value;
            else if(o == OP_DIV) result = x.konst.value / y.konst.value;
            else if(o == OP_MOD) result = x.konst.value % y.konst.value;
            else
                assert(false);
            x.konst.value = result;
        } else {
            x = put_operation(o, x, y);
        }
    } else if(x.type->form == TF_BOOL) {
        if(y.mode != IM_CONDITION) {
            y = load_condition(y);
        }
        if(op == TK_LOGIC_OR) {
            x.condition.false_jump = y.condition.false_jump;
            x.condition.true_jump = merged(y.condition.true_jump, x.condition.true_jump);
            x.condition.cond_code = y.condition.cond_code;
        } else if(op == TK_LOGIC_AND) {
            x.condition.false_jump = merged(y.condition.false_jump, x.condition.false_jump);
            x.condition.true_jump = y.condition.true_jump;
            x.condition.cond_code = y.condition.cond_code;
        } else {
            assert(false);
        }
    }
    return x;
}

Item
generator_relation(int op, Item x, Item y) // x := x ? y
{
    ConditionCode cc = 0;
    switch(op){
    break;case TK_NOT_EQUAL:     cc = CC_NOT_EQUAL;
    break;case TK_EQUAL:         cc = CC_EQUAL;
    break;case TK_LESS:          cc = CC_LESS;
    break;case TK_LESS_EQUAL:    cc = CC_LESS_EQUAL;
    break;case TK_GREATER:       cc = CC_GREATER;
    break;case TK_GREATER_EQUAL: cc = CC_GREATER_EQUAL;
    break;default:
    assert(false);
    }
    x = put_operation(OP_CMP, x, y);
    R -= 1;
    x.mode = IM_CONDITION;
    x.condition.cond_code = cc;
    x.condition.false_jump = 0;
    x.condition.true_jump = 0;
    return x;
}

// Fixes the links stored already in the jmp instructions
// with the current program_counter
void
generator_fix_links(int abs_location)
{
    int l0 = abs_location; // the jump destination to fix
    int l1 = 0;            // next jump destination to fix, saved at l0
    while(l0 != 0) {
        l1 = am_get_jump_location(l0);
        am_fix_jump(l0, am_get_pc() - l0 - 1);
        l0 = l1;
    }
}

Item
generator_cf_jump(Item x)
{
    if(x.mode != IM_CONDITION)
        x = load_condition(x);
    am_emit_c_jump_im(negate_condition(x.condition.cond_code), x.condition.false_jump);
    generator_fix_links(x.condition.true_jump);
    x.condition.false_jump = am_get_pc() - 1;
    return x;
}

Item
generator_cb_jump(Item x, int location)
{
    if(x.mode != IM_CONDITION)
        x = load_condition(x);
    am_emit_c_jump_im(negate_condition(x.condition.cond_code), location - am_get_pc() - 1);
    return x;
}

int
generator_f_jump(int location)
{
    am_emit_jump_im(location);
    int abs_location = am_get_pc() - 1;
    return abs_location;
}

void
generator_b_jump(int location)
{
    am_emit_jump_im(location - am_get_pc() - 1); // relative location
}

Item 
generator_make_item(Object* obj)
{
    Item item = {0};
    item.mode = (ItemMode)obj->klass;
    item.type = obj->type;
    item.level = obj->level;
    if(item.mode == IM_CONST) {
        item.konst.value = obj->konst.value;
    } else if(item.mode == IM_VAR) {
        item.var.offset = obj->var.address_offset;
        if(obj->level == 0)
            item.var.reg = GB;
        else if(obj->level == g_current_level)
            item.var.reg = SP;
        else
            scanner_mark_error("level!");
    } else if(item.mode == IM_PARAMETER) {
        item.parameter.offset = obj->parameter.address_offset;
        if(obj->level == 0)
            item.parameter.reg = GB;
        else if(obj->level == g_current_level)
            item.parameter.reg = SP;
        else
            scanner_mark_error("level!");
    } else if(item.mode == IM_PROCEDURE_CALL) {
        item.procedure_call.offset = obj->procedure.entry_point_offset;
        if(obj->level == 0)
            item.procedure_call.reg = GB;
        else if(obj->level == g_current_level)
            item.procedure_call.reg = SP;
        else
            scanner_mark_error("level!");
    } else {
        //disallow builtin procedure calls for now...
        assert(false);
    }
    return item;
}

Item
generator_make_const_item(TypeForm form, int value)
{
    Item item = {0};
    if(form == TF_INT)
        item.type = &IntType;
    else if(form == TF_BOOL)
        item.type = &BoolType;
    else
        assert(false);
    item.mode = IM_CONST;
    item.konst.value = value;
    return item;
}

void
generator_check_registers()
{
    if(R != 0) {
        scanner_mark_error("Expression crashed RegisterStack. Not in sync.");
    }
}
