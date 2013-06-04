#ifndef _POINTER_ANALYSIS
#define _POINTER_ANALYSIS
#pragma interface
class Pointer_analysis_S;
class Pointer_analysis_L;
class Pointer_analysis_V;
enum Pointer_analysis_kinds{Sk_Pa_source,Sk_Pa_sink,Sk_Pa_rrr,Sk_Pa_lod,Sk_Pa_cpy,Sk_Pa_bj,Sk_Pa_cvt,Sk_Pa_ldc,Sk_Pa_cal,Sk_Pa_gen,Sk_Pa_array,Sk_Pa_mbr,Sk_Pa_identity,Sk_Pa_for_loop,Sk_Pa_pre_test};
struct Pointer_analysis: public Sprob_switches {
Svar * new_var();
int copy_var(Svar *,Svar *);
void meet(Svar *,Snode *);
void _init_tables();
Strans * _FLOW_MAP(Snode *,CFGnode_kinds);


private:
  fg *cfg;

public:

  Pointer_analysis(fg *g);

  inline int pointerass_num() { return cfg->pointerass_num(); }
  inline int set_size()               { return cfg->pointerass_num(); }
  inline int get_pointerass_index(instruction *i) { return cfg->get_pointerass_index(i); }
  inline instruction *get_pointerass(int i)           { return cfg->get_pointerass(i); }
  boolean is_pure(var_sym *v)   { return cfg->is_pure(v); }
  
} ;
struct Pointer_analysis_S: public fg_node {

} ;
struct Pointer_analysis_V: public Svar {

private:
  Pointer_analysis *P;
  bit_set pointerass;
  bit_set_iter iterator;

public:
  Pointer_analysis_V(Pointer_analysis *p);
  void mk_empty()          { pointerass.clear(); }
  instruction* get_pointerass(int i)      { return P->get_pointerass(i); }
  void operator+= (Pointer_analysis_V &r) { pointerass += r.pointerass; }

  void add(instruction *in)
    {
       operand op = in->dst_op(); 
       if(op.kind()==OPER_SYM)
         {
           sym_node *sn = op.symbol();
           if(sn->kind()==SYM_VAR)
              {
                var_sym *vs = (var_sym *)sn;
                type_node *tn = vs->type();
                if(tn->is_ptr())
                  {
                    int i = P->get_pointerass_index(in);
                    if(i>=0)
              pointerass.add(i);
                  }
              }
          }
    }


  void kill(instruction *the_instr) 
    {
      operand o = the_instr->dst_op();
      if(o.kind()==OPER_SYM)
       {
       sym_node *sn = o.symbol();
       if(sn->kind()==SYM_VAR)
        {
        var_sym *vs = (var_sym *)sn;
        for(int i=0;i<(P->pointerass_num());i++)
           {
            instruction *in = P->get_pointerass(i);
            operand op = in->dst_op();
            var_sym *vsi = (var_sym *)(op.symbol());  /*The structure have been fixed by the collect_pointerass*/
            if(vsi==vs)
              {
                int j = P->get_pointerass_index(in);
                if(j>=0)
	        pointerass.remove(j);
              }
           }
         } 
       }
     }

  int test_and_copy(Pointer_analysis_V *r)
    {
      if(pointerass==r->pointerass)
	return 0;
      else
	{
	  pointerass = r->pointerass;
	  return 1;
	}
    }

  void kill_pointerass(bit_set *iset) { *iset -= pointerass; }

  /* iterate through the set of pointer assignments */
  void reset_iter()  { iterator.reset(&pointerass); }
  int is_empty()     { return iterator.is_empty(); }
  instruction *next_pointerass(){ return P->get_pointerass(iterator.step()); }

} ;
struct Pointer_analysis_L: public Strans {
Pointer_analysis *P;

    void out_action(Snode *,Svar *);

} ;
struct Pa_source: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_source();
  ~Pa_source();
  void operator delete(void *);
};
struct Pa_sink: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_sink();
  ~Pa_sink();
  void operator delete(void *);
};
struct Pa_rrr: public Pointer_analysis_L{
void flow(Snode*,Svar *);
void out_action(Snode *,Svar *);
  void *operator new(size_t);
  Pa_rrr();
  ~Pa_rrr();
  void operator delete(void *);
};
struct Pa_lod: public Pointer_analysis_L{
void flow(Snode*,Svar *);
void out_action(Snode *,Svar *);
  void *operator new(size_t);
  Pa_lod();
  ~Pa_lod();
  void operator delete(void *);
};
struct Pa_cpy: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_cpy();
  ~Pa_cpy();
  void operator delete(void *);
};
struct Pa_bj: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_bj();
  ~Pa_bj();
  void operator delete(void *);
};
struct Pa_cvt: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_cvt();
  ~Pa_cvt();
  void operator delete(void *);
};
struct Pa_ldc: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_ldc();
  ~Pa_ldc();
  void operator delete(void *);
};
struct Pa_cal: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_cal();
  ~Pa_cal();
  void operator delete(void *);
};
struct Pa_gen: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_gen();
  ~Pa_gen();
  void operator delete(void *);
};
struct Pa_array: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_array();
  ~Pa_array();
  void operator delete(void *);
};
struct Pa_mbr: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_mbr();
  ~Pa_mbr();
  void operator delete(void *);
};
struct Pa_identity: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_identity();
  ~Pa_identity();
  void operator delete(void *);
};
struct Pa_for_loop: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_for_loop();
  ~Pa_for_loop();
  void operator delete(void *);
};
struct Pa_pre_test: public Pointer_analysis_L{
void flow(Snode*,Svar *);
  void *operator new(size_t);
  Pa_pre_test();
  ~Pa_pre_test();
  void operator delete(void *);
};
#endif
