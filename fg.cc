/* file "fg.h": this file defines the flow graph 
   and various functions for pointer analysis. */

//#pragma implementation "fg.h"

#include <stdio.h>
#include <suif1.h>
#include <nsharlit/sharlit.h>
#include <nsharlit/cfg.h>
#include "assoc.h"
//#include "variables.h"
#include "fg.h"
#include "pointerinfo.h"

fg::fg(proc_sym *p)
   :tree_map(16),
    label_map(16),
    var_index_map(256),
    pointerass_map(256),
    purepointer_kind_map(256),
    forward_graph(new CFG_1),
    backward_graph(new CFG_1_reversed(forward_graph)),
    procedure(p)
{
  bit_set all_read_vars, all_write_vars;
  fg_node *source = new fg_source(this),
    *sink = new fg_sink(this);

  enter(source);
  enter(sink);
  forward_graph->set(source,sink);
  backward_graph->set(sink,source);
  enter(source, p->block(), p->block()->symtab());

  /* walk the tree again to determine used variables */
  all_read_vars.expand(0, set_size());
  all_write_vars.expand(0, set_size());
  walk_for_used_var_info(p->block(), &all_read_vars, &all_write_vars);
  collect_all_pure(p);

  no_control_flow_analysis = 0;
}

/*

The following routines adds a tree node to the flow graph.  It
basically walks the tree, entering nodes into the graph.  Control-flow
edges are computed in two steps.  First, the enter routines puts in
edges where it can, such as those for successive instructions in a
block and edges between components of control-flow structures like
if's, for's, and loop's.  Edges emanating from branches are handled
later in fg::analyze.  To help fg::analyze, the enter
routines creates a lists of branches and associates labels with nodes
so that analyze can link up branches with their targets.

*/
boolean fg::is_pure(var_sym *v)
{
  Pure_pointer_kind ppk;
  if(purepointer_kind_map.lookup(v, &ppk))
  return TRUE;
  else
  return FALSE;

}
void fg::collect_all_pure(proc_sym *p)
{
  sym_node_list *snl = p->block()->proc_syms()->params();
  sym_node_list_iter snli(snl);
  while(!snli.is_empty())
    {
      sym_node *the_sn = snli.step();
      if(the_sn->kind() == SYM_VAR)
        {
          var_sym *the_var = (var_sym *)the_sn;
          if (the_var->type()->is_ptr())
            {
             //debug_pa();
             purepointer_kind_map.associate(the_var,PURE_PARAM);
             //the_var->print();
             //printf("\n");
            }
        }
    }
  walk_for_purepointer_and_kind(p->block());

}
fg_node *fg::enter(fg_node *pred, fg_node *u)
{
  enter(u);
  link(pred,u);
  return u;
}

fg_node *fg::enter(fg_node *u,tree_node *t,
  			   block_symtab *scope)
{
  switch(t->kind())
    {
    case TREE_INSTR:
      {
	instruction *i;
	fg_node *v;
	enter(v = new fg_instr((tree_instr *)t,this));
	link(u, v);

	/* Scan for control-flow transfers and labels */
	i = ((tree_instr *)t)->instr();
	collect_vars(i);

        collect_pointerass_info(i);  //collect pointer assignment infor wherever the node is TREE_INSTR

	switch(i->opcode())
	  {
	  case io_jmp:
	  case io_mbr:
	  case io_ret:
	    /* return NULL to disable connection with ``next'' node */
	    patch_list.extend(v);
	    return NULL;

	  case io_btrue:
	  case io_bfalse:
	    patch_list.extend(v);
	    return v;

	  case io_lab:
	    associate_label(((in_lab *)i)->label(),v);
	    return v;

	  default:
	    return v;
	  }
      }

    case TREE_LOOP:
      {
	fg_loop *v;
	tree_loop *loop = (tree_loop *)t;
	enter(v = new fg_loop(loop,this));
	link(u, v);

	enter(enter(v,
		    loop->body(), scope),   
	      v->continue_node);

	enter(enter(
		    v->continue_node,
		    loop->test(), scope),
	      v->end_node);
	return v->end_node;
      }

    case TREE_FOR:
      {
	/*
	There are two ways of building the flow graph for a for loop.
	  We do only the one with the test at the bottom.
	  */

	fg_for_loop *v;
	tree_for *loop = (tree_for *)t;
	v = new fg_for_loop(loop, this, scope);
	collect_vars(operand(loop->index()));
	assert(tree_map.associate(loop, v));

	enter(
	      enter(
		    enter(
			  enter(u,
				loop->lb_list(), scope),
			  loop->ub_list(), scope),
		    loop->step_list(), scope),
	      v->pre_test);
	
	enter(
	      enter(v->pre_test,
		    loop->landing_pad(), scope),
	      v);

	enter(
	      enter(v,
		    loop->body(), scope),
	      v->continue_node);

	enter(v->continue_node, v->end_node);
	link(v->pre_test, v->end_node);                  /* pre-test edge */
	link(v->continue_node, v);             /* back-edge */

	return v->end_node;
      }

    case TREE_IF:
      {
	fg_if *v;
	fg_node *w, *x;
	tree_if *if_node = (tree_if *)t;
	enter(v = new fg_if(if_node,this));
	link(u, v);

	/* the test and the then part */
	w = enter(enter(v,
			if_node->header(), scope),
		  if_node->then_part(), scope);

	/* enter the else part */
	enter(v->else_start);
	x = enter(v->else_start, if_node->else_part(), scope);

	/* put in the convergence node */
	enter(v->end_node);
	link(x, v->end_node);
	link(w, v->end_node);

	return v->end_node;
      }

    case TREE_BLOCK:
      {
	fg_block *v;
	tree_block *block = (tree_block *)t;
	enter(v = new fg_block(block,this));
	link(u, v);

	return enter(v, block->body(), block->symtab());
      }
    }
  assert(0);
  return NULL;
}

fg_node *fg::enter(fg_node *u,tree_node_list *t,
				   block_symtab *scope)
{
  tree_node_list_iter tnli(t);
  while(!tnli.is_empty())
    {
      tree_node *t = tnli.step();
      u = enter(u, t, scope);
    }
  return u;
}

/*

Analyze scans the patch_list, a list of control-transfer instructions
collected in the enter methods.  Analyze then links these control
transfers with their targets.  Finally, analyze calls CFG_1::analyze
to compute a more efficient edge representation and to do control-flow
analysis.

*/

void fg::analyze()
{
  int i;
  fg_instr *u;
  instruction *inst;

  for(i = 0; i < patch_list.hi; i++)
    {
      u = (fg_instr *)patch_list[i];
      inst = u->instr();
      switch(inst->opcode())
	{
	case io_jmp:
	case io_btrue:
	case io_bfalse:
#ifdef HAWG4
	  {
	    in_rrr *bj = (in_rrr *)inst;
	    operand o = bj->bj_target_op();
	    sym_node *label = o.is_symbol() ? o.symbol() : NULL;
	    fg_node *v;

	    if(label!=NULL && label->is_label())
	      assert(label_map.lookup((label_sym *)label, &v));
	    else
	      assert(0);
	    link(u,v);
	  }
#else
	  {
	      in_bj *bj = (in_bj *) inst;
	      label_sym *label = bj->target();
	      fg_node *v;
	      assert(label_map.lookup((label_sym *)label, &v));
	      link(u,v);
	  }
#endif
	  break;

	case io_mbr:
	  {
	    in_mbr *mbr = (in_mbr *)inst;
	    int j;
	    fg_node *v;

	    assert(label_map.lookup(mbr->default_lab(), &v));
	    link(u, v);
	    for(j = 0; j<(int)mbr->num_labs(); j++)
	      {
		assert(label_map.lookup(mbr->label(j), &v));
		link(u, v);
	      }
	  }
	  break;

	case io_ret:
	  forward_graph->link(u->unique, forward_graph->sink);
	  break;

	default:
	  /* Patch_list should have nothing but control-transfers */
	  assert(0);
	}
    }

  /* compute edge vectors and do control-flow analysis */
  if(!no_control_flow_analysis)
    {
	forward_graph->analyze();
	backward_graph->analyze();
    }

      /* compute a bit set of memory variables */
  ineligible_var_set.expand(0,set_size());
  for(i=0; i < ineligible_vars.hi; i++)
    ineligible_var_set.add(get_var_index(ineligible_vars[i]));
}




/*

Constructors for Flow graph node derivatives.

*/
fg_loop::fg_loop(tree_loop *l, fg *g)
    :fg_node(l,g)
{
  /* create extra nodes and map them to the label */
  g->associate_label(l->toplab(), this);
  g->associate_label(l->contlab(), continue_node = new fg_label(g));
  g->associate_label(l->brklab(), end_node = new fg_label(g));
}

fg_for_loop::fg_for_loop(tree_for *l, fg *g,
					 block_symtab *syms)
    :fg_node(l,g)
{
  scope = syms;
  pre_test = new fg_pre_test(l, g, this);
  g->associate_label(l->contlab(), continue_node = new fg_label(g));
  g->associate_label(l->brklab(), end_node = new fg_label(g));
}

fg_pre_test::fg_pre_test(tree_for *l, fg *g,
					 fg_for_loop *loop_top)
    :fg_node(l,g)
{
  loop = loop_top;
}

fg_if::fg_if(tree_if *i, fg *g)
    :fg_node(i,g)
{
  g->associate_label(i->jumpto(), else_start = new fg_label(g));
  end_node = new fg_label(g);
}




/* print out the graph */
static char *kind_map[] = {
  "source", "sink", "instr", "label", "loop", "for_loop", "pre_test",
  "if", "block" };

void fg::print(FILE *fp)
{
  int i, j, n, n_succ, n_pred;
  fg_node *u, **succ, **pred;

  n = forward_graph->number_of_nodes();
  for(i=0; i < n; i++)
    {
      u = (fg_node *) forward_graph->get_node(i);
      succ = (fg_node **) forward_graph->successors(u->unique,
							    &n_succ);
      pred = (fg_node **) forward_graph->predecessors(u->unique,
							      &n_pred);
      fprintf(fp, "%d %s >", u->unique, kind_map[u->kind()]);
      for(j = 0; j<n_succ; j++)
	fprintf(fp,"%d ",succ[j]->unique);
      fputc('<',fp);
      for(j = 0; j<n_pred; j++)
	fprintf(fp,"%d ",pred[j]->unique);
      fputc('\n',fp);
    }
}

/*
This function collect pointer assignments 
*/

void fg::collect_pointerass_info(instruction *in)
{
    int i,j;
    instruction *the_instr = in;
    switch (the_instr->opcode())
      {
            
        case io_cpy: // pointer assignment a = b; a is a pointer type
          {
            in_rrr *the_cpy = (in_rrr *)the_instr;
            operand srcop = the_cpy->src_op();
            operand dstop = the_cpy->dst_op(); 
            if((srcop.kind()==OPER_SYM) && (dstop.kind()==OPER_SYM))
              {
                sym_node *dstsn = dstop.symbol();
                if(dstsn->kind()==SYM_VAR)
                   {
                     var_sym *dstvs = (var_sym *)dstsn;
                     type_node *dsttn = dstvs->type();
                     if(dsttn->is_ptr())
                       { 
                         //debug_pa();
                         assign_pointerass_index(in);
                         //in->print();
                       }
                   }
               }
            break;
          }

        case io_lod: // pointer assignment a = b[i]; a is a pointer type
          {
            in_rrr *the_lod = (in_rrr *)the_instr;
            operand srcop = the_lod->src_op();
            operand dstop = the_lod->dst_op(); 
            if((srcop.kind()==OPER_INSTR) && (dstop.kind()==OPER_SYM))
              {
                sym_node *dstsn = dstop.symbol();
                if(dstsn->kind()==SYM_VAR)
                   {
                     var_sym *dstvs = (var_sym *)dstsn;
                     type_node *dsttn = dstvs->type();
                     if(dsttn->is_ptr())
                       {
                         //debug_pa();
                         assign_pointerass_index(in);
                         //in->print();
                       }
                   }
               }
             break;
          } 

        case io_array: // pointer assignment a = &b[i]; a is a pointer type
          {
            in_array *the_arr = (in_array *)the_instr;
            operand dstop = the_arr->dst_op(); 
            if(dstop.kind()==OPER_SYM)
              {
                sym_node *dstsn = dstop.symbol();
                if(dstsn->kind()==SYM_VAR)
                   {
                     var_sym *dstvs = (var_sym *)dstsn;
                     type_node *dsttn = dstvs->type();
                     if(dsttn->is_ptr())
                       {
                         //debug_pa();
                         assign_pointerass_index(in);
                         //in->print();
                       }
                   }
               }
             break;
          }       

        //case io_str:
        //case io_memcpy:
        //case io_cvt:
        //case io_add:
        //case io_sub:          

        default:
            break;
      } 

}

/*
The following two functions collects the pure pointer 
and its kind information in the instruction in.
*/

//void collect_purepointer_and_kind(instruction *in)

//void collect_purepointer_and_kind(oprand o)


/*

These functions are used to collect information used in a procedure.

*/

void fg::collect_vars(operand o)
{
  sym_node *symbol;

  switch(o.kind())
    {
    case OPER_NULL:
    case OPER_INSTR:
      break;

    case OPER_SYM:
      symbol = o.symbol();
      switch(symbol->kind())
	{
	case SYM_PROC:
	case SYM_LABEL:
	  break;

	case SYM_VAR:
	  assign_bit_index((var_sym *)symbol);
	}
      break;
    default:
	assert(0);
    }
}

void fg::walk_symtabs(block_symtab *)
{
  /*
  We might need to walk the symbol tables if we need to find the
    liveness of all locals and globals, even those that are never
    mentioned.
    */
}


void fg::assign_pointerass_index(instruction *in)
{
  if(!pointerass_map.associate(in,pointerass_list.hi))
    ;
  else
    {
      pointerass_list.extend(in);
    }
}


void fg::assign_bit_index(var_sym *var)
{
  if(!var_index_map.associate(var,bit_index_map.hi))
    ;
  else
    {
      if(!var->is_auto())
	ineligible_vars.extend(var);
      bit_index_map.extend(var);
    }
}


int fg::get_pointerass_index(instruction *i)
{
    int index;
    if(pointerass_map.lookup(i, &index))
      return index;


    return -1;
}


int fg::get_var_index(var_sym *v)
{
    int index;
    if(var_index_map.lookup(v, &index))
      return index;

    /*
      At this point, something is amiss: either we miss this variable when
	walking the symbol tables (See walk_symtab) or var_index_map is
	  corrupted.
	    */
    error_line(1, NULL, "Can't get bit index for variable "
	       "in get_var_index(var_sym *).");
    return -1;
}

int fg::get_var_index(operand o)
{
  int index;
  sym_node *symbol;

  switch(o.kind())
    {
    case OPER_NULL:
    case OPER_INSTR:
      return -1;

    case OPER_SYM:
      symbol = o.symbol();
      switch(symbol->kind())
	{
	case SYM_PROC:
	case SYM_LABEL:
	  return -1;

	case SYM_VAR:
	  if(var_index_map.lookup((var_sym *)symbol, &index))
	    return index;

	  /*
	  At this point, something is amiss: either we miss this variable when
	    walking the symbol tables (See walk_symtab) or var_index_map is
	      corrupted.
		*/
	  error_line(1, NULL, "Can't get bit index for variable.");
	  return -1;
	default:
	    assert(0);
	}
      break;
    default:
	assert(0);
    }
  assert(0);
  return -1;
}

void fg::collect_vars(instruction *i)
{
  switch(i->format())
    {
    case inf_rrr:
      {
	in_rrr *in = (in_rrr *)i;
	collect_vars(in->dst_op());
	collect_vars(in->src1_op());
	collect_vars(in->src2_op());
	break;
      }

#ifndef HAWG4
      case inf_bj:
      {
	  in_bj *in = (in_bj *)i;
	  collect_vars(in->src_op());
	  break;
      }
#endif

    case inf_ldc:
      {
	  immed c = ((in_ldc *)i)->value();
	  sym_node *symbol;

	  collect_vars(((in_ldc *)i)->dst_op());
	  if(c.is_symbol())
	    {
		symbol = c.symbol();
		switch(symbol->kind())
		  {
		    default:
		      break;

		    case SYM_VAR:
		      collect_vars((var_sym *)symbol);
		      break;
		  }
	    }
      }
      break;

    case inf_cal:
      {
	int j;
	in_cal *in = (in_cal *) i;

	collect_vars(in->dst_op());
	collect_vars(in->addr_op());
	for(j = 0; j < (int)in->num_args(); j++)
	  collect_vars(in->argument(j));
	break;
      }

#ifdef HAWG4
    case inf_cvt:
      {
	in_cvt *cvt = (in_cvt *)i;
	collect_vars(cvt->dst_op());
	collect_vars(cvt->src_op());
	break;
      }
#endif

    case inf_array:
      {
	int j;
	in_array *in = (in_array *) i;

	collect_vars(in->dst_op());
	collect_vars(in->base_op());
	for(j = 0; j < (int)in->dims(); j++)
	  {
	    collect_vars(in->index(j));
	    collect_vars(in->bound(j));
	  }
	break;
      }

    case inf_mbr:
      collect_vars(((in_mbr *)i)->src_op());
      break;

    case inf_gen:
      {
	int j;
	in_gen *in = (in_gen *) i;

	collect_vars(in->dst_op());
	for(j = 0; j < (int)in->num_srcs(); j++)
	  collect_vars(in->src_op(j));
	break;
      }

    case inf_none:
    case inf_lab:
      break;
	    
    }
}

void fg::collect_referenced_vars(instruction *i,
					 bit_set *read_vars,
					 bit_set *write_vars)
{
  switch(i->format())
    {
    case inf_rrr:
      {
	in_rrr *in = (in_rrr *)i;
	add_var(in->dst_op(), write_vars);
	add_var(in->src1_op(), read_vars);
	add_var(in->src2_op(), read_vars);
	break;
      }

#ifndef HAWG4
   case inf_bj:
      {
	  in_bj *in = (in_bj *)i;
	  add_var(in->src_op(), read_vars);
	  break;
      }
#endif

    case inf_ldc:
      {
	  immed c = ((in_ldc *)i)->value();
	  sym_node *symbol;

	  add_var(((in_ldc *)i)->dst_op(), write_vars);
	  if(c.is_symbol())
	    {
		symbol = c.symbol();
		switch(symbol->kind())
		  {
		    default:
		      break;

		    case SYM_VAR:
		      read_vars->add(get_var_index((var_sym *)symbol));
		      break;
		  }
	    }
      }
      break;

    case inf_cal:
      {
	int j;
	in_cal *in = (in_cal *) i;

	add_var(in->dst_op(), write_vars);
	add_var(in->addr_op(), read_vars);
	for(j = 0; j < (int)in->num_args(); j++)
	  add_var(in->argument(j), read_vars);
	break;
      }

#ifdef HAWG4
    case inf_cvt:
      {
	in_cvt *cvt = (in_cvt *)i;
	add_var(cvt->dst_op(), write_vars);
	add_var(cvt->src_op(), read_vars);
	break;
      }
#endif

    case inf_array:
      {
	int j;
	in_array *in = (in_array *) i;

	add_var(in->dst_op(), write_vars);
	add_var(in->base_op(), read_vars);
	for(j = 0; j < (int)in->dims(); j++)
	  {
	    add_var(in->index(j), read_vars);
	    add_var(in->bound(j), read_vars);
	  }
	break;
      }

    case inf_mbr:
      add_var(((in_mbr *)i)->src_op(), read_vars);
      break;

    case inf_gen:
      {
	int j;
	in_gen *in = (in_gen *) i;

	add_var(in->dst_op(), write_vars);
	for(j = 0; j < (int)in->num_srcs(); j++)
	  add_var(in->src_op(j), read_vars);
	break;
      }

    case inf_none:
    case inf_lab:
      break;
	    
    }
}

void fg::add_var(operand o, bit_set *vars)
{
  sym_node *symbol;

  switch(o.kind())
    {
    case OPER_NULL:
    case OPER_INSTR:
      break;

    case OPER_SYM:
      symbol = o.symbol();
      switch(symbol->kind())
	{
	case SYM_PROC:
	case SYM_LABEL:
	  break;

	case SYM_VAR:
	  vars->add(get_var_index((var_sym *)symbol));
	  break;
	}
      break;
    default:
	assert(0);
    }
}

void fg::walk_for_used_var_info(tree_node *t, bit_set *read_vars,
					bit_set *write_vars)
{
  switch(t->kind())
    {
    case TREE_INSTR:
      {
	instruction *i;
	i = ((tree_instr *)t)->instr();
	collect_referenced_vars(i, read_vars, write_vars);
	return;
      }

    case TREE_LOOP:
      {
	tree_loop *loop = (tree_loop *)t;
	walk_for_used_var_info(loop->body(),read_vars, write_vars);
	walk_for_used_var_info(loop->test(),read_vars, write_vars);
	return;
      }

    case TREE_FOR:
      {
	  tree_for *loop = (tree_for *)t;
	  fg_node *u;
	  fg_for_loop *v;

	  /* treat these as being outside the loop */
	  walk_for_used_var_info(loop->lb_list(), read_vars, write_vars);
	  walk_for_used_var_info(loop->ub_list(), read_vars, write_vars);
	  walk_for_used_var_info(loop->step_list(), read_vars, write_vars);
	  walk_for_used_var_info(loop->landing_pad(), read_vars, write_vars);

	      /* lookup and initialize the bit set */
	  assert(tree_map.lookup(loop,&u));
	  assert(u->kind()==NODE_FOR_LOOP);
	  v = (fg_for_loop *)u;
	  v->read_vars.expand(0, set_size());
	  v->write_vars.expand(0, set_size());

	  walk_for_used_var_info(loop->body(), &v->read_vars,
				 &v->write_vars);
	  add_var(loop->index(), &v->read_vars);
	  add_var(loop->index(), &v->write_vars);
	  *read_vars += v->read_vars;
	  *write_vars += v->write_vars;
	  return;
      }

    case TREE_IF:
      {
	  tree_if *if_node = (tree_if *)t;
	  walk_for_used_var_info(if_node->header(), read_vars, write_vars);
	  walk_for_used_var_info(if_node->then_part(), read_vars, write_vars);
	  walk_for_used_var_info(if_node->else_part(), read_vars, write_vars);
	  return;
      }

    case TREE_BLOCK:
      {
	tree_block *block = (tree_block *)t;
	walk_for_used_var_info(block->body(), read_vars, write_vars);
	return;
      }
    }
  assert(0);
}

void fg::walk_for_used_var_info(tree_node_list *t,
					bit_set *read_vars,
					bit_set *write_vars)
{
  tree_node_list_iter tnli(t);
  while(!tnli.is_empty())
    {
      tree_node *t = tnli.step();
      walk_for_used_var_info(t, read_vars, write_vars);
    }
}


void fg::walk_for_purepointer_and_kind(tree_node *tn)
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
            proc_sym *ps = proc_for_call(ic);
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
                        purepointer_kind_map.associate(dstvs,PURE_HEAP);
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
	walk_for_purepointer_and_kind(loop->body());
	walk_for_purepointer_and_kind(loop->test());
	return;
      }

    case TREE_FOR:
      {
	  tree_for *loop = (tree_for *)tn;

	  /* treat these as being outside the loop */
	  walk_for_purepointer_and_kind(loop->lb_list());
	  walk_for_purepointer_and_kind(loop->ub_list());
	  walk_for_purepointer_and_kind(loop->step_list());
	  walk_for_purepointer_and_kind(loop->landing_pad());
          walk_for_purepointer_and_kind(loop->body());
	  return;
      }

    case TREE_IF:
      {
	  tree_if *if_node = (tree_if *)tn;
	  walk_for_purepointer_and_kind(if_node->header());
	  walk_for_purepointer_and_kind(if_node->then_part());
	  walk_for_purepointer_and_kind(if_node->else_part());
	  return;
      }

    case TREE_BLOCK:
      {
	tree_block *block = (tree_block *)tn;
	walk_for_purepointer_and_kind(block->body());
	return;
      }
    }
  assert(0);
}

void fg::walk_for_purepointer_and_kind(tree_node_list *tnl)
{
  tree_node_list_iter tnli(tnl);
  while(!tnli.is_empty())
    {
      tree_node *tn = tnli.step();
      walk_for_purepointer_and_kind(tn);
    }
}





