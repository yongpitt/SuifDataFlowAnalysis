/* file pointerinfo.cc: this file defines useful functions
   to acquire array and pointer information */

#include <stdio.h>
#include <suif1.h>
#include <useful.h>
#include "pointerinfo.h"


/*useful functions in useful.h which will be used for pointer analysis*/ 
//extern boolean operands_are_same_expr(operand op_1,operand op_2);
//extern boolean instrs_are_same_expr(instruction *instr_1,instruction *instr_2);
//extern var_sym *get_sym_of_array(in_array *ai);
//extern boolean instr_references_var(instruction *the_instr,var_sym *the_var);
//extern boolean operand_references_var(operand *the_operand,var_sym *the_var);

/* functions defined in this file */
void debug_pa()
{
}
/*
This function collect all the array instructions that need to be annotated
*/
void find_array_instr(operand op, array_instr_list *ail)
{
  switch(op.kind())
    {
    case OPER_INSTR:
      {
      instruction *in = op.instr();
      find_array_instr(in,ail);
      break;
      }
    case OPER_SYM:

 /*     symbol = o.symbol();
      switch(symbol->kind())
  {
	case SYM_PROC:
	case SYM_LABEL:
	  break;

	case SYM_VAR:
	  assign_bit_index((var_sym *)symbol);
	}   
*/
      break;

    case OPER_NULL:

    default:
      break;
    }
}

void find_array_instr(instruction *in, array_instr_list *ail)
{
    switch (in->opcode())
      {
        case io_array:
           {
            in_array *ia = (in_array *)in;
            //debug_pa();
            ail->append(ia);
            break;
           }
         case io_add:
         case io_sub:
         case io_mul:
         case io_div:
           {
           in_rrr *ir = (in_rrr *)in; 
          // find_array_instr(ir->dst_op(),ail);
           find_array_instr(ir->src1_op(),ail);
           find_array_instr(ir->src2_op(),ail);
           break;
           }
         case io_str:
           {
           in_rrr *ir = (in_rrr *)in; 
           find_array_instr(ir->dst_addr_op(),ail);
           break;
           }
         case io_lod:
           {
           in_rrr *ir = (in_rrr *)in; 
           find_array_instr(ir->src_addr_op(),ail);
           break;
           }
         case io_memcpy:
           {
           in_rrr *ir = (in_rrr *)in; 
           find_array_instr(ir->dst_addr_op(),ail);
           find_array_instr(ir->src_addr_op(),ail);
           break;
           }
         default:
          break;
      }

}

/*
Currently this function only deal with three kinds of pointer assignment:
a = b, a = b[i] and a = &b[i].  this should correspond to the pointerass 
_bit_index data structure.
*/

in_array *trace_to_upper_pointer(in_array *ai, instruction *in)
  {                   
    if((!in) || (!ai)) 
    return NULL;

    int i,j;
    in_array *old_array = ai;
    instruction *the_instr = in;
    switch (the_instr->opcode())
      {
            
        case io_cpy: // pointer assignment a = b; a is a pointer type
          {
            in_rrr *the_cpy = (in_rrr *)the_instr;
            operand upperop = the_cpy->src_op();
            if ((upperop.kind()) != OPER_SYM)
            return NULL;
            sym_node *uppersn = upperop.symbol();
            if((uppersn->kind())!=SYM_VAR)
            return NULL;
            var_sym *uppervar = (var_sym *)uppersn;
            in_array *repai = 
                   new in_array(old_array->result_type(),operand(),
                                operand(uppervar),old_array->elem_size(),
                                old_array->dims(),old_array->offset()); 
            for(i=0; i < (old_array->dims());i++)
              {
               repai->set_index(i,(old_array->index(i)).clone());
              }
            return repai;
          }

        case io_lod: // pointer assignment a = b[i]; a is a pointer type
          {
            in_rrr *the_load = (in_rrr *)the_instr;
            operand upperop = the_load->src_op();
            if ((upperop.kind()) != OPER_INSTR)
            return NULL;
            instruction *upperin = upperop.instr();
            if ((upperin->opcode()) != io_array)
            return NULL;
       
            in_array *upperai = (in_array *)upperin;
            var_sym *uppervar = get_sym_of_array(upperai);
            in_array *repai = 
                   new in_array(old_array->result_type(),operand(),
                                operand(uppervar),old_array->elem_size(),
                                (old_array->dims())+(upperai->dims()),
                                                  old_array->offset()); 
            for(i=0; i < (upperai->dims());i++)
              {
               repai->set_index(i,(upperai->index(i)).clone());
              }

             for(i=upperai->dims(),j=0; i < (old_array->dims())+(upperai->dims());i++,j++)
              {
               repai->set_index(i,(old_array->index(j)).clone());
              }
            return repai;
          } 

        case io_array: // pointer assignment a = &b[i]; a is a pointer type
          {
            in_array *upperai = (in_array *)the_instr; 
            var_sym *uppervar = get_sym_of_array_modified(upperai);  //use modified functione
            int upperdim = upperai->dims();
            int lowerdim = old_array->dims();
            
            in_array *repai = 
                   new in_array(old_array->result_type(),operand(),
                                operand(uppervar),old_array->elem_size(),
                                upperdim+lowerdim-1,old_array->offset()); 

            for(i=0; i < upperdim+lowerdim-1;i++)
              {
               if(i < upperdim-1)
                   repai->set_index(i,(upperai->index(i)).clone());

               else if(i == upperdim-1)
                 { 
                   operand indexoperup = upperai->index(i);
                   operand indexoperlo = old_array->index(0);
                   operand lowerindex = indexoperlo.clone();
                   operand upperindex = indexoperup.clone();
                   

                   //operand upperindex = (upperai->index(i)).clone();
                   //operand lowerindex = (old_array->index(0)).clone();
                   type_node *tn = lowerindex.type();
                   operand mixed_index = operand(new in_rrr(io_add,tn,operand(),
                                                upperindex,lowerindex));
                   repai->set_index(i,mixed_index);
                 }

               else 

                   repai->set_index(i,(old_array->index(i-upperdim)).clone());
                 
              }

            return repai;
         }       

        case io_str:
        case io_memcpy:
        case io_cvt:
        case io_add:
        case io_sub:          

        default:
            return NULL;
      }


  }

proc_sym *proc_for_call(in_cal *the_call)  //This is a modified version of proc_for_call 
  {
    operand function_address = the_call->addr_op();
    if (!function_address.is_expr() && !function_address.is_instr())
        return NULL;
    instruction *addr_instr = function_address.instr();

    if (addr_instr->opcode() != io_ldc)
        return NULL;
    in_ldc *addr_ldc = (in_ldc *)addr_instr;

    immed addr_value = addr_ldc->value();
    if (!addr_value.is_symbol())
        return NULL;
    sym_node *addr_sym = addr_value.symbol();

    if (!addr_sym->is_proc())
        return NULL;
    return (proc_sym *)addr_sym;
  }




var_sym *get_sym_of_array_modified(in_array *ai)
  {
    return get_pass_thru_sym_modified(ai);
  }

var_sym *get_pass_thru_sym_modified(instruction *i)
  {
    instruction *base_instr = i;
    operand addr_op = base_from_instr_modified(base_instr);
    if (!addr_op.is_null())
        addr_op = op_base_addr_op_modified(addr_op);
    if (addr_op.is_symbol())
        return addr_op.symbol();
    if (addr_op.is_expr() || addr_op.is_instr()) //modified code
        base_instr = addr_op.instr();
    if (base_instr->opcode() == io_ldc)
      {
        in_ldc *the_ldc = (in_ldc *)base_instr;
        immed value = the_ldc->value();
        if (value.is_symbol())
          {
            sym_node *the_sym = value.symbol();
            if (the_sym->is_var())
                return (var_sym *)the_sym;
               
          }
      }


    return NULL;
  }

operand op_base_addr_op_modified(operand address)
  {
    assert(address.type()->is_ptr());
    if (address.is_expr() || address.is_instr())  //modified
      {
        operand test_op = base_from_instr_modified(address.instr());
        if (!test_op.is_null())
            return op_base_addr_op_modified(test_op);
      }

    return address;
  }

operand base_from_instr_modified(instruction *the_instr)
  {
    switch (the_instr->opcode())
      {
        case io_lod:     //added code
        case io_cpy:
        case io_cvt:
          {
            in_rrr *the_cvt = (in_rrr *)the_instr;
            if (the_cvt->src_op().type()->is_ptr())
                return the_cvt->src_op();
            break;
          }
        case io_add:
        case io_sub:
          {
            in_rrr *the_rrr = (in_rrr *)the_instr;
            if (the_rrr->src1_op().type()->is_ptr())
                return the_rrr->src1_op();
            if ((the_instr->opcode() == io_add) &&
                the_rrr->src2_op().type()->is_ptr())
              {
                return the_rrr->src2_op();
              }
            break;
          }
            
        case io_array:
          {
            in_array *the_array = (in_array *)the_instr;
            return the_array->base_op();
          }
        default:
            break;
      }

    return operand();
  }



