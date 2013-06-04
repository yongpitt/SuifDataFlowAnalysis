/* malloc_info.cc*/

#include <stdlib.h>
#include <suif1.h>
#include <useful.h>
#include <string.h>
#include "assoc.h"
//#include "variables.h"
#include "fg.h"
#include "pointerinfo.h"
#include "malloc_info.h"

     
malloc_info::malloc_info(in_cal *the_malloc)
  :ic_malloc(the_malloc),
	 patterns(0)
{
	assert(ic_malloc); 
	ti_malloc = ic_malloc->parent();
	
	parallel = false;
	shared = true;
	done = false;
	
	curr_pattern = NULL;
	
	proc_sym *the_ps = ti_malloc->proc();
	ps_malloc = the_ps;
  
  if(strcmp(the_ps->name(),"main")!=0 && parallel_region) //parallel_region should be a global varaible
  parallel = true;
  
  rl = new reference_list; //
  
  operand the_op;
  instruction *the_i;
	the_i = ((tree_instr *)ti_malloc)->instr();
      if(the_i->opcode() == io_cvt)
          the_op = ic_malloc->dst_op();
      else if(the_i->opcode() == io_str) 
        {
          in_rrr *the_str = (in_rrr *)the_i;
          the_op = the_str->src1_op();
        }
      else
      	{
          fprintf(stderr,"malloc detected in an unexpected structure");
          exit(0);
        }
  rl->append(the_op);
   	
}		
	
int malloc_info::add_reference(operand the_op)
{
	rl->append(the_op);
}

int malloc_info::kill_reference(operand the_op)
{
	reference_list_iter iterl(rl);
	while(!iterl.is_empty())
	  {
	  	operand 
	  }
}
