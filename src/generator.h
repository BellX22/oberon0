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
enum ItemMode {
	IM_CONST = OC_CONST,
	IM_VAR = OC_VAR,
	IM_PROCEDURE_CALL = OC_PROCEDURE,
	IM_PARAMETER = OC_PARAMETER, // Actually a reference...
	IM_BUILTIN_PROCEDURE_CALL = OC_BUILTIN_PROCEDURE,
	IM_REGISTER = OC_COUNT,
	IM_REGISTER_INDIRECT,
	IM_CONDITION
};

struct Item {
	ItemMode mode;
	Type    *type;
	int      level;

	//int a, r; // all beneath could be reduced to this

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
			int function_number;
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
int generator_get_word_size();
int generator_get_program_counter();
int generator_get_current_level();
void generator_open();
void generator_header(int size);
void generator_close();
void generator_enter(int parblksize, int locblksize);  // procedure entry
void generator_return(int size);                       // procedure exit
void generator_increase_level(int delta);
Item generator_parameter(Item x, ObjectClass klass);   // push params of procedure call
void generator_call(Item x);                           // call procedure
void generator_store(Item x, Item y);                  // x := y;
Item generator_array_index(Item array, Item index);    // x := x[y]
Item generator_field(Item record, Object *field);      // x := x.y
Item generator_op1(int op_token_kind, Item x);         // x := op x
Item generator_op2(int op_token_kind, Item x, Item y); // x := x op y
Item generator_relation(int op, Item x, Item y);       // x := x ? y
Item generator_cf_jump(Item x);               // conditional forward jump
int  generator_f_jump(int relative_location); // unconditional forward jump
Item generator_cb_jump(Item x, int location); // conditional backward jump
void generator_b_jump(int location);          // unconditional backward jump
void generator_fix_links(int location);
Item generator_make_item(Object *obj);
Item generator_make_const_item(TypeForm form, int value);
void generator_check_registers();

#endif
