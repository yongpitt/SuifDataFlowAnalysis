/* file "process_main.cc" :
   This file contains functions used to analyze the main procedure 
   of the benchmark.(fix the inputs,branch simplification,constant 
   prprogation,constant folding,information collection,pthread 
   detection and multithreaded analysis initailization.
*/

#include <stdio.h>
#include <string.h>
#include <suif1.h>
#include <useful.h>
#include "./propagate_fold/pf.h"

DECLARE_DLIST_CLASS(cal_instr_list, in_cal *);

alist pure_pointer_in_main; 

int num_threads;

void
start_main_proc (proc_sym *mps)
{
  // push_clue(ps->block());
  
  proc_sym *slave_proc;
  
  cal_instr_list *cil = new cal_instr_list;
  
  fix_and_propagate(mps);
  
  find_pthreadcreate(mps->block()->body(),cil);
  cal_instr_list_iter iterc(cil); 
  assert(cil);
  
  while(!iterc.is_empty()) 
    {
      in_cal *ci = iterc.step();
      slave_proc = proc_for_pthreads(ci);
      printf("slave procedure found %s: \n",slave_proc->name());           
    }
    
  // pop_clue(ps->block());
}

void
fix_and_propagate (proc_sym *mps)
{
  propagate_constants(mps->block());
  forward_propagate(mps->block(),FPK_ALL); 
  fold_all_constants(mps->block());
}

     /***************************************************************************
      * find call instructions and add them into the cil list.  later in the   
      * find_slave_proc() function, the list used to search pthread_create calls.                                                
      ***************************************************************************/
      
void find_pthreadcreate(tree_node_list * tnl, cal_instr_list * cil)
{
  tree_node_list_iter iter(tnl);
  while(!iter.is_empty()) {
      tree_node * tn = iter.step();
     
      switch(tn->kind()) {
      case TREE_FOR:{
          tree_for * tnf = (tree_for *)tn;
          find_pthreadcreate(tnf->lb_list(), cil);
          find_pthreadcreate(tnf->ub_list(), cil);
          find_pthreadcreate(tnf->step_list(), cil);

     
          find_pthreadcreate(tnf->landing_pad(), cil);
          find_pthreadcreate(tnf->body(), cil);
          break;}
     
      case TREE_IF:{
          tree_if * tni = (tree_if *)tn;
          find_pthreadcreate(tni->header(), cil);
          find_pthreadcreate(tni->then_part(), cil);
          find_pthreadcreate(tni->else_part(), cil);
          break;}
     
      case TREE_LOOP:{
          tree_loop * tnl = (tree_loop *)tn;
          find_pthreadcreate(tnl->body(), cil);
          find_pthreadcreate(tnl->test(), cil);
          break;}
     
      case TREE_BLOCK:{
          tree_block * tnb = (tree_block *)tn;
          find_pthreadcreate(tnb->body(), cil);
          break;}
     
      case TREE_INSTR:{
          tree_instr * tnin = (tree_instr *)tn;
          if(cil)
              find_pthreadcreate(tnin->instr(), cil);
          break;}
     
      default:{
          assert(0);
          break;}
      }
  }
}
     
     
/*
 Iterate over all the instructions of expression trees, add cal        
 instructions to the list.                                               
*/
void find_pthreadcreate(instruction *ins, cal_instr_list *cil)
{
  if(ins->opcode() == io_cal) {
      assert(cil);
      in_cal *ia = (in_cal*)ins;
      proc_sym *pfc = proc_for_call(ia);  //modified
      if (strcmp(pfc->name(),"pthread_create")==0)  //modified
      cil->append(ia);
  }
     
  for(int i=0; i<ins->num_srcs(); i++) {
      operand op(ins->src_op(i));
      if(op.is_instr())
          find_pthreadcreate(op.instr(), cil);
       }
}


proc_sym *proc_for_call(in_cal *the_call)
{
    operand function_address = the_call->addr_op();
    if (!function_address.is_expr())
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

/* the function that return an proc_sym for an in_cal class*/        
proc_sym *proc_for_pthreads(in_cal *the_call)
{
    operand pfp = the_call->argument(2);
    if (!pfp.is_instr())
        return NULL;
    instruction * addr_instr = pfp.instr();

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

void collect_purepointer_in_main(tree_node *tn)
{
  switch(tn->kind())
    {
    case TREE_INSTR:
      {
	instruction *i;
	i = ((tree_instr *)tn)->instr();
        if(i->opcode() != io_cvt)
        return;
        in_rrr *the_cvt = (in_rrr *)i;
        operand cvtop = the_cvt->src_op();
        if(cvtop.kind()!=OPER_INSTR)
        return;
        //debug_pa();
        instruction *thein = cvtop.instr();
        if(thein->opcode() == io_cal)
          {
            in_cal *ic = (in_cal *)thein;
            proc_sym *ps = proc_for_call_malloc(ic);
            if(!ps)
            return;
            if(strcmp(ps->name(),"malloc")==0)
             {
               operand dstop = ic->dst_op();
               switch(dstop.kind())
                {
                  case OPER_NULL:
                     return;
                  case OPER_INSTR:     
                     /*here we may add new rules to collect pure pointers in future*/                  
                     return;
                  case OPER_SYM:
                    {
                     sym_node *dstsn = dstop.symbol();
                     switch(dstsn->kind())
	                    {
                        case SYM_VAR:{
	                      var_sym *dstvs = (var_sym *)dstsn;
                        pure_pointer_in_main.enter(dstvs,ic);
                        //debug_pa();
                        //printf("malloc pure: ");
                        //dstvs->print();
                        return;
                                     }
	                      case SYM_PROC:
	                      case SYM_LABEL:
	                      return;
	                    }
                     break;
                    }
                  default:
                     return;
	
                }
              }
	    }
         return;
      }
	

    case TREE_LOOP:
      { //void collect_all_pure(proc_sym *p)
	     tree_loop *loop = (tree_loop *)tn;
	     collect_purepointer_in_main(loop->body());
	     collect_purepointer_in_main(loop->test());
	     return;
      }

    case TREE_FOR:
      {
	     tree_for *loop = (tree_for *)tn;

	     /* treat these as being outside the loop */
	     collect_purepointer_in_main(loop->lb_list());
	     collect_purepointer_in_main(loop->ub_list());
	     collect_purepointer_in_main(loop->step_list());
	     collect_purepointer_in_main(loop->landing_pad());
       collect_purepointer_in_main(loop->body());
	     return;
      }

    case TREE_IF:
      {
	     tree_if *if_node = (tree_if *)tn;
	     collect_purepointer_in_main(if_node->header());
	     collect_purepointer_in_main(if_node->then_part());
	     collect_purepointer_in_main(if_node->else_part());
	     return;
      }

    case TREE_BLOCK:
      {
	     tree_block *block = (tree_block *)tn;
	     collect_purepointer_in_main(block->body());
	     return;
      }
    }
  assert(0);
}

void collect_purepointer_in_main(tree_node_list *tnl)
{
  tree_node_list_iter tnli(tnl);
  while(!tnli.is_empty())
    {
      tree_node *tn = tnli.step();
      collect_purepointer_in_main(tn);
    }
}


proc_sym *proc_for_call_malloc(in_cal *the_call)  //This is a modified version of proc_for_call 
  {                                      //used to detect malloc call nested in cvt instruction
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
  
int get_thread_num(in_cal *ci) //notice: the ci should be a pthread_create call 
{
	 tree_node *the_node = ci->parent()->parent()->parent();
	 while((the_node->kind()!=TREE_FOR)&&(the_node))
	 	{
	 		the_node = the_node->parent()->parent();
	  }
	  if(the_node->kind() == TREE_FOR)
	  {
	  	tree_for *the_for = (tree_for *)the_node;
	  	operand thre_num_oper = iteration_count(the_for);
	  	
	  	
	  } 
	  else 
	  return 1;
}
