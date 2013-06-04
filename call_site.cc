/* call_site.cc*/


#include <stdlib.h>
#include <suif1.h>
#include <useful.h>
#include <string.h>
#include <cg.h>
     
     /* Declare a new class which is a list of "in_cal"'s */
      
     DECLARE_DLIST_CLASS(cal_instr_list, in_cal *);
   
     /* Declare functions */
 
     proc_sym *proc_for_call(in_cal *the_call);
     void find_cal_instr(tree_node_list * tnl, cal_instr_list * cil);
     void find_cal_instr(tree_node * tn, cal_instr_list * cil);
     void find_cal_instr(instruction * i, cal_instr_list * cil);  
     
     /* Declare annotation name*/

     char *k_call_site;



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

     /***************************************************************************
         find a call instruction and add it to cil
      ***************************************************************************/ 


void find_cal_instr(tree_node_list * tnl, cal_instr_list * cil)
     {
         tree_node_list_iter iter(tnl);
         while(!iter.is_empty()) {
             tree_node * tn = iter.step();
     
             switch(tn->kind()) {
             case TREE_FOR:{
                 tree_for * tnf = (tree_for *)tn;
                 find_cal_instr(tnf->lb_list(), cil);
                 find_cal_instr(tnf->ub_list(), cil);
                 find_cal_instr(tnf->step_list(), cil);
                 find_cal_instr(tnf->landing_pad(), cil);
                 find_cal_instr(tnf->body(), cil);
                 break;}
     
             case TREE_IF:{
                 tree_if * tni = (tree_if *)tn;
                 find_cal_instr(tni->header(), cil);
                 find_cal_instr(tni->then_part(), cil);
                 find_cal_instr(tni->else_part(), cil);
                 break;}
     
             case TREE_LOOP:{
                 tree_loop * tnl = (tree_loop *)tn;
                 find_cal_instr(tnl->body(), cil);
                 find_cal_instr(tnl->test(), cil);
                 break;}
     
             case TREE_BLOCK:{
                 tree_block * tnb = (tree_block *)tn;
                 find_cal_instr(tnb->body(), cil);
                 break;}
     
             case TREE_INSTR:{
                 tree_instr * tnin = (tree_instr *)tn;
                 if(cil)
                     find_cal_instr(tnin->instr(), cil);
                 break;}
     
             default:{
                 assert(0);
                 break;}
             }
         }
     }
     
     
     /***************************************************************************
      * Iterate over all the instructions of expression trees, add cal        *
      * instructions to the list.                                               *
      ***************************************************************************/
void find_cal_instr(instruction * ins, cal_instr_list * cil)
     {
         if(ins->opcode() == io_cal) {
             assert(cil);
             in_cal *ia = (in_cal *)ins;
          //   if(!ia->are_annotations())
          //   proc_sym *pfc = proc_for_call(ia);
          //   if(pfc->is_readable()) 
             cil->append(ia);
             
         }
     
         for(int i=0; i<ins->num_srcs(); i++) {
             operand op(ins->src_op(i));
             if(op.is_instr())
                 find_cal_instr(op.instr(), cil);
         }
     }
     
 
void do_proc(tree_proc *tp)
     {   
         int call_site_number;
         proc_sym *pfc;
         proc_sym *curr_pfc;
         proc_sym *psym = tp->proc();
         cal_instr_list *cil = new cal_instr_list;
         cal_instr_list *cil_backup = new cal_instr_list;
         
         find_cal_instr(tp->body(),cil);
         find_cal_instr(tp->body(),cil_backup);
         cal_instr_list_iter iterc(cil);
         cal_instr_list_iter iterc_backup(cil_backup);

         printf("\n =======%s======= \n", psym->name());
         printf("CALL SITES in %s: \n",psym->name());     
         while(!iterc_backup.is_empty()) {
           call_site_number = 1;
           in_cal *curr_ci = iterc_backup.step();
           curr_pfc = proc_for_call(curr_ci);
           if(curr_ci->are_annotations())
           continue;
           immed_list *il = new immed_list;
           il->append(immed(call_site_number));
           curr_ci->append_annote(k_call_site,il);
             
             iterc.reset(cil); 
             while(!iterc.is_empty()) {
             in_cal *ci = iterc.step();
             pfc = proc_for_call(ci);
             if(pfc != curr_pfc)
             continue;
             if(!ci->are_annotations()){
             call_site_number++;
             immed_list *il = new immed_list;
             il->append(immed(call_site_number));
             ci->append_annote(k_call_site,il);
             }
             }    
                         
           }   
     } 

int main(int argc, char *argv[])
     {
         start_suif(argc, argv);
         ANNOTE(k_call_site,  "call_site",  TRUE);
         suif_proc_iter(argc, argv, do_proc,TRUE,TRUE,TRUE);
     }

