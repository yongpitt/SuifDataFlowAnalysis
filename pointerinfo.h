/*pointerinfo.h: this file contains functions and 
  data structure declarations to manipulate pointer
  ailas information */

#include <useful.h>

DECLARE_DLIST_CLASS(array_instr_list, in_array *);

void debug_pa();
var_sym *get_sym_of_array_modified(in_array *ai);
var_sym *get_pass_thru_sym_modified(instruction *i);
operand op_base_addr_op_modified(operand address);
operand base_from_instr_modified(instruction *the_instr);
void find_array_instr(operand op, array_instr_list *ail);
void find_array_instr(instruction *in, array_instr_list *ail);
in_array *trace_to_upper_pointer(in_array *ai, instruction *in);
proc_sym *proc_for_call(in_cal *the_call);
