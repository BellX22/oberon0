#include "abstract_machine.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

//--------------------------------------------------------------------------

#define MAXLINESIZE 80
#define MAXNUMLINES 4096

typedef struct {
	int  line;
	char out[MAXNUMLINES][MAXLINESIZE];
} AsmFile;

static void print(AsmFile *out, int line, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(out->out[line], format, args);
	va_end(args);
}

AsmFile g_out_file = {0};

#define PRINT(format, ...) print(&g_out_file, g_out_file.line++, format, ## __VA_ARGS__)

void print_file()
{
	for (int i = 0; i < g_out_file.line; i++) {
		printf("%3d: %s", i, g_out_file.out[i]);
	}
}

//--------------------------------------------------------------------------

#define MAX_REGISTERS 16
static int g_registers[MAX_REGISTERS];
static char *Name[MAX_REGISTERS] = {
	"R0", "R1", "R2", "R3",
	"R4", "R5", "R6", "R7",
	"R8", "R9", "R10", "R11",
	"R12", "GB", "SP", "LNK",
};

int am_get_pc(void)
{
	return g_out_file.line;
}
void am_fix_jump(int at, int with)
{
	// 'at'must be a jump instruction!!!
	char *text = g_out_file.out[at];
	sprintf(text + 4, "%3d\n", with);
	// scan to the number and overwrite it with relative jump_address
}

int am_get_jump_location(int abs_location)
{
	const char *text = g_out_file.out[abs_location];
	const char *num = text + 4;

	char buffer[512];
	int d = 0;

	for (int i = 0; i < 3; i += 1) {
		if (is_digit(num[i])) {
			buffer[d++] = num[i];
		}
	}

	buffer[d] = '\0';
	int n = to_number(buffer);
	return n;
}

void am_emit_label(const char *name)
{
	PRINT("%s:\n", name);
}

// Emit Code
void am_emit_mov(reg_index dest, reg_index src)
{
	PRINT("%s := %s\n", Name[dest], Name[src]);
}

void am_emit_cmp(reg_index reg1, reg_index reg2)
{
	PRINT("cmp %s, %s\n", Name[reg1], Name[reg2]);
}

void am_emit_and(reg_index dest, reg_index lhs, reg_index rhs)
{
	PRINT("%s := %s and %s\n", Name[dest], Name[lhs], Name[rhs]);
}

void am_emit_or (reg_index dest, reg_index lhs, reg_index rhs)
{
	PRINT("%s := %s or %s\n", Name[dest], Name[lhs], Name[rhs]);
}

void am_emit_xor(reg_index dest, reg_index lhs, reg_index rhs)
{
	PRINT("%s := %s xor %s\n", Name[dest], Name[lhs], Name[rhs]);
}

void am_emit_add(reg_index dest, reg_index lhs, reg_index rhs)
{
	PRINT("%s := %s + %s\n", Name[dest], Name[lhs], Name[rhs]);
}

void am_emit_sub(reg_index dest, reg_index lhs, reg_index rhs)
{
	PRINT("%s := %s - %s\n", Name[dest], Name[lhs], Name[rhs]);
}

void am_emit_mul(reg_index dest, reg_index lhs, reg_index rhs)
{
	PRINT("%s := %s * %s\n", Name[dest], Name[lhs], Name[rhs]);
}

void am_emit_div(reg_index dest, reg_index lhs, reg_index rhs)
{
	PRINT("%s := %s / %s\n", Name[dest], Name[lhs], Name[rhs]);
}

void am_emit_lsh(reg_index dest, reg_index lhs, reg_index rhs)
{
	PRINT("%s := %s << %s\n", Name[dest], Name[lhs], Name[rhs]);
}

void am_emit_rsh(reg_index dest, reg_index lhs, reg_index rhs)
{
	PRINT("%s := %s >> %s\n", Name[dest], Name[lhs], Name[rhs]);
}

void am_emit_mod(reg_index dest, reg_index lhs, reg_index rhs)
{
	PRINT("%s := %s %% %s\n", Name[dest], Name[lhs], Name[rhs]);
}

void am_emit_mov_im(reg_index dest, int value)
{
	PRINT("%s := %d\n",        Name[dest], value);
}

void am_emit_cmp_im(reg_index reg, int value)
{
	PRINT("cmp %s, %d\n",      Name[reg], value);
}

void am_emit_and_im(reg_index dest, reg_index lhs, int rhs_value)
{
	PRINT("%s := %s and %d\n", Name[dest], Name[lhs], rhs_value);
}

void am_emit_or_im (reg_index dest, reg_index lhs, int rhs_value)
{
	PRINT("%s := %s or %d\n",  Name[dest], Name[lhs], rhs_value);
}

void am_emit_xor_im(reg_index dest, reg_index lhs, int rhs_value)
{
	PRINT("%s := %s xor %d\n", Name[dest], Name[lhs], rhs_value);
}

void am_emit_add_im(reg_index dest, reg_index lhs, int rhs_value)
{
	PRINT("%s := %s + %d\n",   Name[dest], Name[lhs], rhs_value);
}

void am_emit_sub_im(reg_index dest, reg_index lhs, int rhs_value)
{
	PRINT("%s := %s - %d\n",   Name[dest], Name[lhs], rhs_value);
}

void am_emit_mul_im(reg_index dest, reg_index lhs, int rhs_value)
{
	PRINT("%s := %s * %d\n",   Name[dest], Name[lhs], rhs_value);
}

void am_emit_div_im(reg_index dest, reg_index lhs, int rhs_value)
{
	PRINT("%s := %s / %d\n",   Name[dest], Name[lhs], rhs_value);
}

void am_emit_lsh_im(reg_index dest, reg_index lhs, int rhs_value)
{
	PRINT("%s := %s << %d\n",  Name[dest], Name[lhs], rhs_value);
}

void am_emit_rsh_im(reg_index dest, reg_index lhs, int rhs_value)
{
	PRINT("%s := %s >> %d\n",  Name[dest], Name[lhs], rhs_value);
}

void am_emit_mod_im(reg_index dest, reg_index lhs, int rhs_value)
{
	PRINT("%s := %s %% %d\n",  Name[dest], Name[lhs], rhs_value);
}

void am_emit_load(reg_index dest, reg_index base_reg, int offset)
{
	PRINT("%s := mem[%s + %d]\n", Name[dest], Name[base_reg], offset);
}

void am_emit_store(reg_index src, reg_index base_reg, int offset)
{
	PRINT("mem[%s + %d] := %s\n", Name[base_reg], offset, Name[src]);
}

void am_emit_jump_im(int relative)
{
	PRINT("jmp %3d\n", relative);
}

void am_emit_jump_equal_im(int relative)
{
	PRINT("je  %3d\n", relative);
}

void am_emit_jump_not_equal_im(int relative)
{
	PRINT("jne %3d\n", relative);
}

void am_emit_jump_less_im(int relative)
{
	PRINT("jl  %3d\n", relative);
}

void am_emit_jump_less_equal_im(int relative)
{
	PRINT("jle %3d\n", relative);
}

void am_emit_jump_greater_im(int relative)
{
	PRINT("jg  %3d\n", relative);
}

void am_emit_jump_greater_euqal_im(int relative)
{
	PRINT("jge %3d\n", relative);
}

void am_emit_jump(reg_index reg)
{
	PRINT("jmp %s\n", Name[reg]);
}

void am_emit_jump_equal(reg_index reg)
{
	PRINT("je  %s\n", Name[reg]);
}

void am_emit_jump_not_equal(reg_index reg)
{
	PRINT("jne %s\n", Name[reg]);
}

void am_emit_jump_less(reg_index reg)
{
	PRINT("jl  %s\n", Name[reg]);
}

void am_emit_jump_less_equal(reg_index reg)
{
	PRINT("jle %s\n", Name[reg]);
}

void am_emit_jump_greater(reg_index reg)
{
	PRINT("jg  %s\n", Name[reg]);
}

void am_emit_jump_greater_euqal(reg_index reg)
{
	PRINT("jge %s\n", Name[reg]);
}

void am_emit_operation(Operation op, reg_index a, reg_index b, reg_index c)
{
	switch (op) {
		//break;case OP_MOV: am_emit_mov(a,c);
		break;

	case OP_AND:
		am_emit_and(a, b, c);
		break;

	case OP_OR:
		am_emit_or(a, b, c);
		break;

	case OP_XOR:
		am_emit_xor(a, b, c);
		break;

	case OP_LSH:
		am_emit_lsh(a, b, c);
		break;

	case OP_RSH:
		am_emit_rsh(a, b, c);
		break;

	case OP_ADD:
		am_emit_add(a, b, c);
		break;

	case OP_SUB:
		am_emit_sub(a, b, c);
		break;

	case OP_MUL:
		am_emit_mul(a, b, c);
		break;

	case OP_DIV:
		am_emit_div(a, b, c);
		break;

	case OP_MOD:
		am_emit_mod(a, b, c);
		break;

	case OP_CMP:
		am_emit_cmp(b, c);
		break;

	default:
		assert(false);
		break;
	}
}

void am_emit_operation_im(Operation op, reg_index a, reg_index b, int value)
{
	switch (op) {
		//break;case OP_MOV: am_emit_mov_im(a,value);
		break;

	case OP_AND:
		am_emit_and_im(a, b, value);
		break;

	case OP_OR:
		am_emit_or_im(a, b, value);
		break;

	case OP_XOR:
		am_emit_xor_im(a, b, value);
		break;

	case OP_LSH:
		am_emit_lsh_im(a, b, value);
		break;

	case OP_RSH:
		am_emit_rsh_im(a, b, value);
		break;

	case OP_ADD:
		am_emit_add_im(a, b, value);
		break;

	case OP_SUB:
		am_emit_sub_im(a, b, value);
		break;

	case OP_MUL:
		am_emit_mul_im(a, b, value);
		break;

	case OP_DIV:
		am_emit_div_im(a, b, value);
		break;

	case OP_MOD:
		am_emit_mod_im(a, b, value);
		break;

	case OP_CMP: 
		am_emit_cmp_im(a, value); // check if we pass the correct params
		break;

	default:
		assert(false);
		break;
	}
}

ConditionCode negate_condition(ConditionCode cc)
{
	if (cc == CC_TRUE) return CC_FALSE;

	if (cc == CC_FALSE) return CC_TRUE;

	if (cc == CC_EQUAL) return CC_NOT_EQUAL;

	if (cc == CC_NOT_EQUAL) return CC_EQUAL;

	if (cc == CC_LESS) return CC_GREATER_EQUAL;

	if (cc == CC_LESS_EQUAL) return CC_GREATER;

	if (cc == CC_GREATER) return CC_LESS_EQUAL;

	if (cc == CC_GREATER_EQUAL) return CC_LESS;

	assert(false);
	return 0;
}

void am_emit_c_jump_im(ConditionCode cc, int relative)
{
	switch (cc) {
		break;

	case CC_TRUE:
		am_emit_jump_im(relative);
		break;

	case CC_FALSE:         // never
		break;

	case CC_EQUAL:
		am_emit_jump_equal_im(relative);
		break;

	case CC_NOT_EQUAL:
		am_emit_jump_not_equal_im(relative);
		break;

	case CC_LESS:
		am_emit_jump_less_im(relative);
		break;

	case CC_LESS_EQUAL:
		am_emit_jump_less_equal_im(relative);
		break;

	case CC_GREATER:
		am_emit_jump_greater_im(relative);
		break;

	case CC_GREATER_EQUAL:
		am_emit_jump_greater_euqal_im(relative);
		break;

	default:
		assert(false);
		break;
	}
}

void am_emit_c_jump(ConditionCode cc, reg_index reg)
{
	switch (cc) {
		break;

	case CC_TRUE:
		am_emit_jump(reg);
		break;

	case CC_FALSE:         // never
		break;

	case CC_EQUAL:
		am_emit_jump_equal(reg);
		break;

	case CC_NOT_EQUAL:
		am_emit_jump_not_equal(reg);
		break;

	case CC_LESS:
		am_emit_jump_less(reg);
		break;

	case CC_LESS_EQUAL:
		am_emit_jump_less_equal(reg);
		break;

	case CC_GREATER:
		am_emit_jump_greater(reg);
		break;

	case CC_GREATER_EQUAL:
		am_emit_jump_greater_euqal(reg);
		break;

	default:
		assert(false);
		break;
	}
}
