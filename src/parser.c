#include "parser.h"
#include "scanner.h"
#include "types.h"
#include "objects.h"
#include "generator.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

TokenKind g_symbol;
Object*   g_module_scope = NULL;
Object*   g_current_scope = NULL;

////////////////////////////////////////////////////////////////////////////
/// Helper Functions
////////////////////////////////////////////////////////////////////////////

static void next(void) { g_symbol = scanner_get(); }

static void
sym_assert_then_next(TokenKind kind, const char* message)
{
    if (g_symbol == kind)
        next();
    else
        scanner_mark_error(message);
}

static void
open_scope(void)
{
    Object* obj = malloc(sizeof(*obj));
    memset(obj, 0, sizeof(*obj));
    if(g_current_scope != NULL)
        g_current_scope->child = obj;
    obj->klass = OC_HEAD;
    obj->parent = g_current_scope;
    obj->next = NULL;
    g_current_scope = obj;
}

static void
close_scope(void)
{
    g_current_scope = g_current_scope->parent;
}

static Object*
create_object(ObjectClass klass, const char* name)
{
    Object* obj = object_find(&g_current_scope, name);
    if(obj == NULL) {
        obj = object_append(&g_current_scope);
        obj->klass = klass;
        string_copy(obj->name, name);
        return obj;
    }
    scanner_mark_error("multiple definitions '%s'", name);
    return obj;
}

static Object*
find_object(const char* name)
{
    for(Object* scope = g_current_scope; scope; scope = scope->parent) {
        Object* obj = object_find(&scope, name);
        if(obj)
            return obj;
    }
    scanner_mark_error("undefined '%s'", name);
    return NULL;
}

static Object*
find_field(Object* fields_list, const char* name)
{
    return object_find(&fields_list, name);
}

static bool
check_int(Item item)
{
    if(item.type == &IntType)
        return true;
    scanner_mark_error("not an int");
    return false;
}

static bool
check_bool(Item item)
{
    if(item.type == &BoolType)
        return true;
    scanner_mark_error("not a bool");
    return false;
}


////////////////////////////////////////////////////////////////////////////
/// Parser Rules
////////////////////////////////////////////////////////////////////////////

static Item parse_expression(void);

static Item
parse_selector(Item x)
{
    while(g_symbol == TK_LEFT_BRACKET || g_symbol == TK_PERIOD) {
        if(g_symbol == TK_LEFT_BRACKET) {
            next();
            Item index = parse_expression();
            if(x.type->form == TF_ARRAY) {
                check_int(index);
                x = generator_array_index(x, index);
                x.type = x.type->array.base;
            } else {
                scanner_mark_error("not an array");
            }
            sym_assert_then_next(TK_RIGHT_BRACKET, "]?");
        } else if(g_symbol == TK_PERIOD) {
            next();
            if(g_symbol == TK_IDENTIFIER) {
                if (x.type->form == TF_RECORD) {
                    Object* record_field = find_field(x.type->record.fields, scanner_get_identifier());
                    next();
                    if(record_field != NULL) {
                        x = generator_field(x, record_field);
                        x.type = record_field->type;
                    }
                } else {
                    scanner_mark_error("undef");
                }
            } else {
                scanner_mark_error("ident?");
            }
        } else {
            scanner_mark_error("not a selector");
        }
    }
    return x;
}

#if 0
static void
parse_builtin_function()
{
    if(g_symbol == TK_LEFT_PAREN)
    {
        g_symbol = scanner_get();
        parse_expression();
        if(fctno == 2)
            ;
        else if(fctno == 3)
        {
            generator_odd(x);
        }
        else if(fctno == 4)
        {
            if(g_symbol == TK_COMMA)
            {
                g_symbol = scanner_get();
                parse_expression(y);
                generator_bit(x, y);
            }
            else
            {
                scanner_mark_error("command expected");
            }
        }
        if(g_symbol == TK_RIGHT_PAREN)
            g_symbol = scanner_get();
        else
            scanner_mark_error("rparen expected");
    }
    else
    {
        scanner_mark_error("param missing");
        generator_make_const_item(x, TF_INT, 0);
    }
}
#endif

static Item
parse_factor()
{
    Item item = {0};
    Object *obj = NULL;
    // sync block
    if(g_symbol < TK_LEFT_PAREN) {
        scanner_mark_error("ident?");
        while(g_symbol < TK_LEFT_PAREN)
            next();
    }
    if(g_symbol == TK_IDENTIFIER) {
        obj = find_object(scanner_get_identifier());
        next();
        if(obj->klass == OC_BUILTIN_PROCEDURE) {
            //parse_builtin_function(x, obj->value);
            //x.type = obj->type;
        } else {
            item = generator_make_item(obj);
            item = parse_selector(item);
        }
    } else if(g_symbol == TK_LITERAL_NUMBER) {
        item = generator_make_const_item(TF_INT, scanner_get_number());
        next();
    } else if(g_symbol == TK_LEFT_PAREN) {
        next();
        if(g_symbol != TK_RIGHT_PAREN) {
            item = parse_expression();
        }
        sym_assert_then_next(TK_RIGHT_PAREN, ")?");
    } else if(g_symbol == TK_LOGIC_NOT) {
        next();
        item = parse_factor();
        check_bool(item);
        item = generator_op1(TK_LOGIC_NOT, item);
    } else {
        scanner_mark_error("factor?");
        item = generator_make_item(NULL); // insert guard?
    }
    return item;
}

// when int -> mul and div
static Item
parse_term()
{
    Item x = parse_factor();
    while(g_symbol >= TK_TIMES && g_symbol <= TK_LOGIC_AND) {
        TokenKind op = g_symbol;
        next();
        if(op == TK_LOGIC_AND) {
            check_bool(x);
            x = generator_op1(op, x);
        } else {
            check_int(x);
        }
        Item y = parse_factor();
        if(x.type == y.type) {
            x = generator_op2(op, x, y);
        } else {
            scanner_mark_error("incompatible types");
        }
    }
    return x;
}

// when int -> plus and minus
static Item
parse_simple_expression()
{
    Item x;
    if(g_symbol == TK_PLUS) {
        next();
        x = parse_term();
        check_int(x);
    } else if(g_symbol == TK_MINUS) {
        next();
        x = parse_term();
        x = generator_op1(TK_MINUS, x);
    } else {
        x = parse_term();
    }
    while(g_symbol >= TK_PLUS && g_symbol <= TK_LOGIC_OR) {
        TokenKind op = g_symbol;
        next();
        if(op == TK_LOGIC_OR) {
            check_bool(x);
            x = generator_op1(op, x);
        } else {
            check_int(x);
        }
        Item y = parse_term();
        if(x.type == y.type) {
            x = generator_op2(op, x, y);
        } else {
            scanner_mark_error("incompatible types");
        }
    }
    return x;
}

// relational only
static Item
parse_expression()
{
    Item x = parse_simple_expression();
    if(g_symbol >= TK_EQUAL && g_symbol <= TK_GREATER_EQUAL) {
        TokenKind op = g_symbol; // save operator
        next();
        Item y = parse_simple_expression();
        if(x.type == y.type)
            x = generator_relation(op, x, y);
        else
            scanner_mark_error("incompatible types");
        x.type = &BoolType;
    }
    return x;
}

static void parse_statement_sequence(void);
static void
parse_statement_if(void)
{
    assert(g_symbol == TK_KEY_IF);
    next();
    Item condition = parse_expression();
    check_bool(condition);
    // execute a jump when the condition is 'false'
    condition = generator_cf_jump(condition); // condition holds the current pc
    sym_assert_then_next(TK_KEY_THEN, "then?");
    parse_statement_sequence();
    int abs_loc = 0;
    while(g_symbol == TK_KEY_ELSEIF) {
        next();
        abs_loc = generator_f_jump(abs_loc); // here we 'mark' the entry of the next elsif
        generator_fix_links(condition.condition.false_jump); // fix jump dest from cf_jump
        condition = parse_expression();
        check_bool(condition);
        condition = generator_cf_jump(condition);
        sym_assert_then_next(TK_KEY_THEN, "then?");
        parse_statement_sequence();
    }
    if(g_symbol == TK_KEY_ELSE) {
        next();
        abs_loc = generator_f_jump(abs_loc); // jump simply to the 'end'
        generator_fix_links(condition.condition.false_jump);
        parse_statement_sequence();
    } else {
        generator_fix_links(condition.condition.false_jump);
    }
    generator_fix_links(abs_loc); // fix all forward jumps at once
    sym_assert_then_next(TK_KEY_END, "end?");
}

static void
parse_statement_while(void)
{
    assert(g_symbol == TK_KEY_WHILE);
    next();
    int location = generator_get_program_counter();
    Item item = parse_expression();
    check_bool(item);
    item = generator_cf_jump(item);
    sym_assert_then_next(TK_KEY_DO, "do?");
    parse_statement_sequence();
    generator_b_jump(location);
    generator_fix_links(item.condition.false_jump);
    sym_assert_then_next(TK_KEY_END, "end?");
}

static void
parse_statement_repeat(void)
{
    assert(g_symbol == TK_KEY_REPEAT);
    next();
    int location = generator_get_program_counter();
    parse_statement_sequence();
    if(g_symbol == TK_KEY_UNTIL) {
        next();
        Item item = parse_expression();
        check_bool(item);
        item = generator_cb_jump(item, location);
    } else {
        scanner_mark_error("missing until");
        next();
    }
}

// assignment and procedure call
static void
parse_statement_identifier(void)
{
    assert(g_symbol == TK_IDENTIFIER);
    Object* obj = find_object(scanner_get_identifier());
    next();
    Item x = generator_make_item(obj);
    x = parse_selector(x);
    if(g_symbol == TK_ASSIGN) {
        next();
        Item y = parse_expression();
        if((x.type->form == TF_BOOL || x.type->form == TF_INT)
            && x.type->form == y.type->form) {
            // simple assignment
            generator_store(x, y); // x := y
        } else {
            scanner_mark_error("incompatible assignment");
        }
    } else if(g_symbol == TK_EQUAL) {
        scanner_mark_error(":= ?");
        next();
        parse_expression(); // silently discard...
    } else if(x.mode == IM_PROCEDURE_CALL) {
        Object* param = obj->parent;
        if(g_symbol == TK_LEFT_PAREN) {
            next();
            if(g_symbol == TK_RIGHT_PAREN) {
                next();
            } else {
                while(true) {
                    Item param_ex = parse_expression();
                    if(param->is_param) {
                        if(param_ex.type == param->type) {
                            // TODO@Andreas: return value not used...?
                            // load the params into registers
                            // or push to the stack? for passing to procedure
                            generator_parameter(param_ex, param->klass);
                        } else {
                            scanner_mark_error("bad param type");
                        }
                        param = param->next;
                    } else {
                        scanner_mark_error("too many parameters");
                    }
                    if(g_symbol == TK_COMMA) {
                        next();
                    } else if(g_symbol == TK_RIGHT_PAREN) {
                        next();
                        break;
                    } else if(g_symbol >= TK_SEMICOLON) {
                        break;
                    } else {
                        scanner_mark_error(") or , ?");
                    }
                }
            }
        }
        // must be OC_PROCEDURE
        if(obj->procedure.entry_point_offset < 0) {
            scanner_mark_error("forward call not allowed");
        } else {
            generator_call(x);
            if(param && param->is_param) {
                scanner_mark_error("too few parameters");
            }
        }
    }
    else if(x.mode == IM_BUILTIN_PROCEDURE_CALL) {
        assert(false);
        int n = x.a;
        if(g_symbol == TK_LEFT_PAREN) {
            next();
            Item x = parse_expression();
            if(g_symbol == TK_COMMA) {
                next();
                Item y = parse_expression();
                if(n == 0) {
                    //generator_get(x, y);
                } else if(n == 1) {
                    //generator_put(x, y);
                } else {
                    scanner_mark_error("comma expected");
                }
            }
            if(g_symbol == TK_RIGHT_PAREN) {
                next();
            } else {
                scanner_mark_error("rparen expected");
            }
        }
        else {
            scanner_mark_error("rparen expected");
        }
    }
    else if(obj->klass == OC_TYPE) {
        scanner_mark_error("illegal assignment");
    } else {
        scanner_mark_error("statement");
    }
}

static void
parse_statement_sequence(void)
{
    while(true) {
        // sync
        if(g_symbol < TK_IDENTIFIER) {
            scanner_mark_error("statement?");
            do {
                next();
            } while (g_symbol < TK_IDENTIFIER);
        }
        if(g_symbol == TK_IDENTIFIER) {
            parse_statement_identifier();
        } else if(g_symbol == TK_KEY_IF) {
            parse_statement_if();
        } else if(g_symbol == TK_KEY_WHILE) {
            parse_statement_while();
        } else if(g_symbol == TK_KEY_REPEAT) {
            parse_statement_repeat();
        }
        if(g_symbol == TK_SEMICOLON) {
            next();
        } else if((g_symbol >= TK_SEMICOLON && g_symbol <= TK_KEY_IF) || g_symbol >= TK_KEY_ARRAY) {
            break;
        } else {
            scanner_mark_error("; ?");
        }
    }
    generator_check_registers();
}

static Object*
parse_identifier_list(ObjectClass klass)
{
    if(g_symbol == TK_IDENTIFIER) {
        Object* first = create_object(klass, scanner_get_identifier());
        next();
        while(g_symbol == TK_COMMA) {
            next();
            if(g_symbol == TK_IDENTIFIER) {
                create_object(klass, scanner_get_identifier());
                next();
            } else {
                scanner_mark_error("ident?");
            }
        }
        sym_assert_then_next(TK_COLON, ":?");
        return first;
    }
    return NULL;
}

static Type*
parse_type_declaration()
{
    // sync
    if((g_symbol != TK_IDENTIFIER) && g_symbol >= TK_KEY_CONST) {
        scanner_mark_error("type?");
        do {
            next();
        } while ((g_symbol != TK_IDENTIFIER) || (g_symbol < TK_KEY_ARRAY));
    }
    Type* type = &IntType; // default type
    if(g_symbol == TK_IDENTIFIER) {
        Object* obj = find_object(scanner_get_identifier());
        next();
        if(obj->klass == OC_TYPE)
            type = obj->type;
        else
            scanner_mark_error("type?");
    } else if(g_symbol == TK_KEY_ARRAY) {
        next();
        Item item = parse_expression();
        if(item.mode != IM_CONST /*|| item.a < 0*/)
            scanner_mark_error("bad index");
        sym_assert_then_next(TK_KEY_OF, "of?");
        Type* base_type = parse_type_declaration();
        type = malloc(sizeof(*type));
        type->form = TF_ARRAY;
        type->array.base = base_type;
        type->array.len = item.konst.value;
        type->size = type->array.len * base_type->size;
    } else if(g_symbol == TK_KEY_RECORD) {
        next();
        type = malloc(sizeof(*type));
        type->form = TF_RECORD;
        type->size = 0;
        open_scope();
        while(true) {
            if(g_symbol == TK_IDENTIFIER) {
                // fields
                Object* first = parse_identifier_list(OC_FIELD);
                Type* field_type = parse_type_declaration();
                // for all field identifiers per type
                for(Object* it = first; it; it = it->next) {
                    it->type = field_type;
                    it->level = generator_get_current_level();
                    it->field.offset = type->size;
                    type->size += it->type->size;
                }
            }
            if(g_symbol == TK_SEMICOLON)
                next();
            else if(g_symbol == TK_IDENTIFIER)
                scanner_mark_error(";?");
            else
                break;
        }
        type->record.fields = g_current_scope->next;
        close_scope();
        sym_assert_then_next(TK_KEY_END, "end?");
    } else {
        scanner_mark_error("ident?");
    }
    return type;
}

static void
parse_formal_parameter_section(int *param_block_size)
{
    int param_size = 0;
    Object* param_first = NULL;
    Type* type = NULL;
    if(g_symbol == TK_KEY_VAR) {
        next();
        param_first = parse_identifier_list(OC_PARAMETER);
    } else {
        param_first = parse_identifier_list(OC_VAR);
    }
    if(g_symbol == TK_IDENTIFIER) {
        Object* obj = find_object(scanner_get_identifier());
        next();
        if(obj->klass == OC_TYPE) {
            type = obj->type;
        } else {
            scanner_mark_error("type?");
            type = &IntType;
        }
    } else {
        scanner_mark_error("ident?");
        type = &IntType;
    }
    if(param_first->klass == OC_VAR) {
        param_size = type->size;
        if(type->form >= TF_ARRAY) {
            scanner_mark_error("no struct parameter!");
        }
    } else {
        // address size (4 bytes)
        param_size = generator_get_word_size();
    }
    for(Object* it = param_first; it; it = it->next) {
        it->type = type;
        it->level = generator_get_current_level();
        it->var.address_offset = *param_block_size;
        it->is_param = true;
        *param_block_size += param_size;
    }
}

static void parse_declarations(int *declarations_bytes_needed);
static void
parse_procedure_declaration()
{
    char procedure_name[MAX_STRLEN];
    const int MarkSize = 4;//TODO@Andreas: Why 4? Maybe generator_get_word_size()? Size of address to jump to
    Object *proc = NULL;
    int local_block_size = 0;
    int param_block_size = 0;
    next();
    if(g_symbol == TK_IDENTIFIER) {
        string_copy(procedure_name, scanner_get_identifier());
        proc = create_object(OC_PROCEDURE, scanner_get_identifier());
        next();
        param_block_size = MarkSize;
        generator_increase_level(1);
        open_scope();
        proc->procedure.entry_point_offset = -1;
        if(g_symbol == TK_LEFT_PAREN) {
            next();
            if(g_symbol == TK_RIGHT_PAREN) {
                next();
            } else {
                parse_formal_parameter_section(&param_block_size);
                while(g_symbol == TK_SEMICOLON) {
                    next();
                    parse_formal_parameter_section(&param_block_size);
                }
                sym_assert_then_next(TK_RIGHT_PAREN, ")?");
            }
        }
        local_block_size = param_block_size;
        proc->parent = g_current_scope->next;
        sym_assert_then_next(TK_SEMICOLON, ";?");
        parse_declarations(&local_block_size);
        while(g_symbol == TK_KEY_PROCEDURE) {
            parse_procedure_declaration();
            sym_assert_then_next(TK_SEMICOLON, ";?");
        }
        proc->procedure.entry_point_offset = generator_get_program_counter();
        generator_enter(param_block_size, local_block_size);
        if(g_symbol == TK_KEY_BEGIN) {
            next();
            parse_statement_sequence();
        }
        sym_assert_then_next(TK_KEY_END, "end?");
        if(g_symbol == TK_IDENTIFIER) {
            if(!string_equal(procedure_name, scanner_get_identifier()))
                scanner_mark_error("no match");
            next();
        }
        generator_return(local_block_size);
        close_scope();
        generator_increase_level(-1);
    }
}

static void
parse_declarations(int *declarations_bytes_needed)
{
    int variables_size = *declarations_bytes_needed;
    // sync
    if(g_symbol < TK_KEY_CONST && g_symbol != TK_KEY_END) {
        scanner_mark_error("declaration?");
        do {
            next();
        } while ((g_symbol < TK_KEY_CONST) && (g_symbol != TK_KEY_END));
    }
    while(true) {
        if(g_symbol == TK_KEY_CONST) {
            next();
            while(g_symbol == TK_IDENTIFIER) {
                Object* obj = create_object(OC_CONST, scanner_get_identifier());
                next();
                sym_assert_then_next(TK_EQUAL, "=?");
                Item item = parse_expression();
                if(item.mode == IM_CONST) {
                    obj->konst.value = item.konst.value;
                    obj->type = item.type;
                } else {
                    scanner_mark_error("expression not constant");
                }
                sym_assert_then_next(TK_SEMICOLON, ";?");
            }
        }
        if(g_symbol == TK_KEY_TYPE) {
            next();
            while(g_symbol == TK_IDENTIFIER) {
                Object* obj = create_object(OC_TYPE, scanner_get_identifier());
                next();
                sym_assert_then_next(TK_EQUAL, "=?");
                obj->type = parse_type_declaration();
                sym_assert_then_next(TK_SEMICOLON, ";?");
            }
        }
        if(g_symbol == TK_KEY_VAR) {
            next();
            while(g_symbol == TK_IDENTIFIER) {
                Object* first = parse_identifier_list(OC_VAR);
                Type* type = parse_type_declaration();
                for(Object* it = first; it; it = it->next) {
                    it->type = type;
                    it->level = generator_get_current_level();
                    // here we use the size as an address/offset...
                    it->var.address_offset = variables_size;
                    variables_size += it->type->size;
                    it->is_param = false;
                }
                sym_assert_then_next(TK_SEMICOLON, ";?");
            }
        }
        if(g_symbol >= TK_KEY_CONST && g_symbol <= TK_KEY_VAR)
            scanner_mark_error("declaration?");
        else
            break;
    }
    *declarations_bytes_needed = variables_size;
}

static void
parse_module()
{
    Object *obj = NULL;
    char module_name[MAX_STRLEN] = {0};
    int declarations_bytes_needed = 0;
    if (g_symbol != TK_KEY_MODULE) {
        scanner_mark_error("module?");
        return;
    }
    open_scope(); // module scope
    g_module_scope = g_current_scope;
    create_object(OC_TYPE, "integer")->type = &IntType;
    create_object(OC_TYPE, "bool")->type = &BoolType;
    obj = create_object(OC_CONST, "true");
    obj->type = &BoolType;
    obj->konst.value = 1;
    obj = create_object(OC_CONST, "false");
    obj->type = &BoolType;
    obj->konst.value = 0;
    next();
    if (g_symbol == TK_IDENTIFIER) {
        string_copy(module_name, scanner_get_identifier());
        next();
    }
    sym_assert_then_next(TK_SEMICOLON, ";?");
    // global declaration
    parse_declarations(&declarations_bytes_needed);
    while (g_symbol == TK_KEY_PROCEDURE) {
        parse_procedure_declaration();
        sym_assert_then_next(TK_SEMICOLON, ";?");
    }
    // allocate space for global declarations
    // module header?
    generator_header(declarations_bytes_needed);
    if (g_symbol == TK_KEY_BEGIN) {
        next();
        parse_statement_sequence();
    }
    sym_assert_then_next(TK_KEY_END, "end?");
    if (g_symbol == TK_IDENTIFIER) {
        if (!string_equal(module_name, scanner_get_identifier()))
            scanner_mark_error("no match");
        next();
    }
    else {
        scanner_mark_error("ident?");
    }
    sym_assert_then_next(TK_PERIOD, ".?");
    close_scope();
    if(!scanner_has_error()) {
        generator_close();
    }
}

void
parse_program(const char* source)
{
    scanner_init(source);
    next();
    parse_module();
}
