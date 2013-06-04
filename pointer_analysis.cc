#pragma implementation 
#include <nsharlit/sharlit.h>
#include <stdio.h>
#include <nsharlit/cfg.h>
#include <nsharlit/sharlit_test.h>

/* file "pointer_analysis.dflow" */


#include <suif1.h>
#include "main.h"
#include "assoc.h"
#include "pointerinfo.h"
#include "fg.h"

#include "pointer_analysis.h"

static Pa_source static_Pa_source;
void *Pa_source::operator new(size_t)
{
  return &static_Pa_source;
}
Pa_source::Pa_source()
{
  kind= Sk_Pa_source;
}
Pa_source::~Pa_source()
{
  assert(this==&static_Pa_source);
}
void Pa_source::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_source);
}
static Pa_sink static_Pa_sink;
void *Pa_sink::operator new(size_t)
{
  return &static_Pa_sink;
}
Pa_sink::Pa_sink()
{
  kind= Sk_Pa_sink;
}
Pa_sink::~Pa_sink()
{
  assert(this==&static_Pa_sink);
}
void Pa_sink::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_sink);
}
static Pa_rrr static_Pa_rrr;
void *Pa_rrr::operator new(size_t)
{
  return &static_Pa_rrr;
}
Pa_rrr::Pa_rrr()
{
  kind= Sk_Pa_rrr;
}
Pa_rrr::~Pa_rrr()
{
  assert(this==&static_Pa_rrr);
}
void Pa_rrr::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_rrr);
}
static Pa_lod static_Pa_lod;
void *Pa_lod::operator new(size_t)
{
  return &static_Pa_lod;
}
Pa_lod::Pa_lod()
{
  kind= Sk_Pa_lod;
}
Pa_lod::~Pa_lod()
{
  assert(this==&static_Pa_lod);
}
void Pa_lod::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_lod);
}
static Pa_cpy static_Pa_cpy;
void *Pa_cpy::operator new(size_t)
{
  return &static_Pa_cpy;
}
Pa_cpy::Pa_cpy()
{
  kind= Sk_Pa_cpy;
}
Pa_cpy::~Pa_cpy()
{
  assert(this==&static_Pa_cpy);
}
void Pa_cpy::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_cpy);
}
static Pa_bj static_Pa_bj;
void *Pa_bj::operator new(size_t)
{
  return &static_Pa_bj;
}
Pa_bj::Pa_bj()
{
  kind= Sk_Pa_bj;
}
Pa_bj::~Pa_bj()
{
  assert(this==&static_Pa_bj);
}
void Pa_bj::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_bj);
}
static Pa_cvt static_Pa_cvt;
void *Pa_cvt::operator new(size_t)
{
  return &static_Pa_cvt;
}
Pa_cvt::Pa_cvt()
{
  kind= Sk_Pa_cvt;
}
Pa_cvt::~Pa_cvt()
{
  assert(this==&static_Pa_cvt);
}
void Pa_cvt::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_cvt);
}
static Pa_ldc static_Pa_ldc;
void *Pa_ldc::operator new(size_t)
{
  return &static_Pa_ldc;
}
Pa_ldc::Pa_ldc()
{
  kind= Sk_Pa_ldc;
}
Pa_ldc::~Pa_ldc()
{
  assert(this==&static_Pa_ldc);
}
void Pa_ldc::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_ldc);
}
static Pa_cal static_Pa_cal;
void *Pa_cal::operator new(size_t)
{
  return &static_Pa_cal;
}
Pa_cal::Pa_cal()
{
  kind= Sk_Pa_cal;
}
Pa_cal::~Pa_cal()
{
  assert(this==&static_Pa_cal);
}
void Pa_cal::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_cal);
}
static Pa_gen static_Pa_gen;
void *Pa_gen::operator new(size_t)
{
  return &static_Pa_gen;
}
Pa_gen::Pa_gen()
{
  kind= Sk_Pa_gen;
}
Pa_gen::~Pa_gen()
{
  assert(this==&static_Pa_gen);
}
void Pa_gen::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_gen);
}
static Pa_array static_Pa_array;
void *Pa_array::operator new(size_t)
{
  return &static_Pa_array;
}
Pa_array::Pa_array()
{
  kind= Sk_Pa_array;
}
Pa_array::~Pa_array()
{
  assert(this==&static_Pa_array);
}
void Pa_array::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_array);
}
static Pa_mbr static_Pa_mbr;
void *Pa_mbr::operator new(size_t)
{
  return &static_Pa_mbr;
}
Pa_mbr::Pa_mbr()
{
  kind= Sk_Pa_mbr;
}
Pa_mbr::~Pa_mbr()
{
  assert(this==&static_Pa_mbr);
}
void Pa_mbr::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_mbr);
}
static Pa_identity static_Pa_identity;
void *Pa_identity::operator new(size_t)
{
  return &static_Pa_identity;
}
Pa_identity::Pa_identity()
{
  kind= Sk_Pa_identity;
}
Pa_identity::~Pa_identity()
{
  assert(this==&static_Pa_identity);
}
void Pa_identity::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_identity);
}
static Pa_for_loop static_Pa_for_loop;
void *Pa_for_loop::operator new(size_t)
{
  return &static_Pa_for_loop;
}
Pa_for_loop::Pa_for_loop()
{
  kind= Sk_Pa_for_loop;
}
Pa_for_loop::~Pa_for_loop()
{
  assert(this==&static_Pa_for_loop);
}
void Pa_for_loop::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_for_loop);
}
static Pa_pre_test static_Pa_pre_test;
void *Pa_pre_test::operator new(size_t)
{
  return &static_Pa_pre_test;
}
Pa_pre_test::Pa_pre_test()
{
  kind= Sk_Pa_pre_test;
}
Pa_pre_test::~Pa_pre_test()
{
  assert(this==&static_Pa_pre_test);
}
void Pa_pre_test::operator delete(void *ptr)
{
  assert(ptr==&static_Pa_pre_test);
}
void
Pa_source::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;


}
void
Pa_sink::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;


}
void
Pa_rrr::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;



}
void
Pa_rrr::out_action(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;

    in_rrr *in = (in_rrr *) (_N->instr());
    //immed_list *list = new immed_list;
    array_instr_list *ail = new array_instr_list;
    if(!in)
    return;
    find_array_instr(in,ail);
    //in->print();
    array_instr_list_iter aili(ail);
      while(!aili.is_empty())
       {
        int i=0;
        //debug_pa();
        in_array *ia_to_be_processed = aili.step();
 
        operand itbp = ia_to_be_processed->base_op();
       
       
        //ia_to_be_processed->print();
        var_sym *vs_to_be_processed = get_sym_of_array_modified(ia_to_be_processed);
        var_sym *vs_ass_n;
        instruction *pointer_ass_n;
        if(P->is_pure(vs_to_be_processed))
        {
         //debug_pa();//now reach hear
         continue;
        }
        _V->reset_iter();
        while(!_V->is_empty())
           {             
             //debug_pa();
             instruction *pointer_ass = _V->next_pointerass();
             operand op_ass = pointer_ass->dst_op();
             var_sym *vs_ass = (var_sym *)(op_ass.symbol());
             if(vs_ass==vs_to_be_processed)
             {i++; vs_ass_n = vs_ass;pointer_ass_n = pointer_ass;}
           } 
        if(i==1)
        {
          //debug_pa();//now reach hear
          in_array *alias_array = trace_to_upper_pointer(ia_to_be_processed,pointer_ass_n);
          if((alias_array)&&(ia_to_be_processed))
            {
              immed_list *list = new immed_list;
              list->append(immed(alias_array));
             // instruction *test_in = pointer_ass_n.clone(); //debug code
             // operand test_op = operand(alias_array); //debug_code
              //list->append(immed(test_op));   //debug code
              ia_to_be_processed->append_annote(k_alias,list);
              //alias_array->print();//debug code
              list->clear();
            }

         }
       
       }  

}
void
Pa_lod::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;

  in_rrr *in = (in_rrr *) _N->instr();
  _V->kill(in);
  _V->add(in);

}
void
Pa_lod::out_action(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;



}
void
Pa_cpy::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;

  in_rrr *in = (in_rrr *) _N->instr();
  _V->kill(in);
  _V->add(in);

}
void
Pa_bj::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;



}
void
Pa_cvt::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;



}
void
Pa_ldc::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;



}
void
Pa_cal::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;



}
void
Pa_gen::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;



}
void
Pa_array::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;

  int i;
  in_array *in = (in_array *) _N->instr();

  _V->kill(in);
  _V->add(in);


}
void
Pa_mbr::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;



}
void
Pa_identity::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;


}
void
Pa_for_loop::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;


}
void
Pa_pre_test::flow(Snode *__N,Svar *__V){
Pointer_analysis_V * _V = (Pointer_analysis_V *) __V;
Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;



}
Svar *Pointer_analysis::new_var(){

  return new Pointer_analysis_V(this);

}
int Pointer_analysis::copy_var(Svar *__d,Svar *__s){
Pointer_analysis_V * _DST = (Pointer_analysis_V *) __d;
Pointer_analysis_V * _SRC = (Pointer_analysis_V *) __s;

  return _DST->test_and_copy(_SRC);

}
void Pointer_analysis::meet(Svar *__d,Snode *__n){
Pointer_analysis_V * _DST = (Pointer_analysis_V *) __d;
Pointer_analysis_V **_SRCS = 0;
int _NSRCS=get_srcs(*(Svar ***)&_SRCS,__n);

  int i;
  _DST->mk_empty();
  for(i=0; i<_NSRCS; i++)
      *_DST += *_SRCS[i];

}
Strans *Pointer_analysis::_FLOW_MAP(Snode *__N, CFGnode_kinds _Kind)
{
  Pointer_analysis_S * _N = (Pointer_analysis_S *) __N;

  Pointer_analysis_L *l;
  /* map each node to a flow function */
  switch(_N->kind())
    {
    case NODE_SOURCE:
      l=new Pa_source;
      break;

    case NODE_SINK:
      l=new Pa_sink;
      break;

    case NODE_INSTR:
      {
	instruction *inst = _N->instr();
	switch(inst->format())
	  {
	  case inf_rrr:
	    {
	      switch(inst->opcode())
		{
                case io_cpy:
		  l=new Pa_cpy;
		  break;

		case io_lod:
		  l=new Pa_lod;
		  break;

		default:
		  l=new Pa_rrr;
		  break;
		}
	    }
	    break;

#ifndef HAWG4
	  case inf_bj:
	    l=new Pa_bj;
	    break;
#endif

#ifdef HAWG4
	  case inf_cvt:
	    l=new Pa_cvt;
	    break;
#endif

	  case inf_ldc:
	    l=new Pa_ldc;
	    break;

	  case inf_cal:
	    l=new Pa_cal;
	    break;

	  case inf_array:
	    l=new Pa_array;
	    break;

	  case inf_mbr:
	    l=new Pa_mbr;
	    break;

	  case inf_none:
	  case inf_lab:
	    l=new Pa_identity;
	    break;

	  case inf_gen:
	    l=new Pa_gen;
	    break;
	  }
      }
      break;

    case NODE_FOR_LOOP:
      l=new Pa_for_loop;
      break;

    case NODE_PRE_TEST:
      l=new Pa_pre_test;
      break;

    case NODE_LABEL:
    case NODE_LOOP:
    case NODE_IF:
    case NODE_BLOCK:
      l=new Pa_identity;
      break;
    }
  l->P = this;
  return l;

}
void Pointer_analysis::_init_tables()
{
assert(-3==Sk_top); /* check */
assert(-1==Sk_identity); /* check */
n_states=0;
}


Pointer_analysis::Pointer_analysis(fg *g)
   :Sprob_switches(g->forward_graph,1)

{
  cfg = g;
}

Pointer_analysis_V::Pointer_analysis_V(Pointer_analysis *p):pointerass(0,p->set_size())
{
  P=p;
}

void Pointer_analysis_L::out_action(Snode *__N, Svar *__V)
{
#if 0
    instruction *in = ((Pointer_analysis_S *)__N)->instr();
    Pointer_analysis_V *_V = (Pointer_analysis_V *)__V;
    immed_list *list = new immed_list;

    _V->reset_iter();
    while(!_V->is_empty())
      list->append(immed(_V->next_var(),0));
      
    if(in)
      in->append_annote(k_live,list);
    else
      delete list;
#endif
}



