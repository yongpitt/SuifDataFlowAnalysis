/* file "fg.h": this file defines the flow graph 
   and data structures for pointer analysis. */

/* Define necessary data structures and their assistant functions*/

//#pragma interface

class fg;
class fg_node;
class fg_instr;
class fg_label;
class fg_loop;
class fg_for_loop;
class fg_pre_test;
class fg_if;
class fg_block;

inline int pointerass_hash(instruction *i) { return ((int)i)>>2;}
inline int label_hash(label_sym *s) { return ((int)s)>>2;}
inline int tree_hash(tree_node *t) { return ((int)t)>>2;}
inline int var_sym_hash(var_sym *v) { return ((int)v)>>2; }

inline boolean compare_vars(var_sym **a,var_sym **b) { return *a == *b; }
inline boolean compare_pointerass(instruction **a, instruction **b) { return *a == *b;}
inline boolean compare_labels(label_sym **a, label_sym **b) { return *a == *b;}
inline boolean compare_trees(tree_node **a, tree_node **b) { return *a == *b;}


enum Pure_pointer_kind
{
  PURE_HEAP,
  PURE_STATIC,
  PURE_PARAM,
  PURE_STACK
};

DECLARE_ASSOC_TABLE(Pointerass_bit_index, instruction*, int, pointerass_hash,
  	    compare_pointerass);
DECLARE_X_ARRAY0(Pointerass_list, instruction*, 256);




DECLARE_ASSOC_TABLE(Tree_to_fg_node, tree_node *, fg_node *, tree_hash, compare_trees);
DECLARE_X_ARRAY0(fg_instr_xarray,fg_node*,0);

DECLARE_ASSOC_TABLE(Variable_bit_index, var_sym*, int, var_sym_hash,
		    compare_vars);
DECLARE_X_ARRAY0(Variable_list, var_sym*, 256);



DECLARE_ASSOC_TABLE(Label_to_fg_node, label_sym*,
		    fg_node*,label_hash,compare_labels);
/*the "int" in the following definition indicate the dimention of the pointer*/
DECLARE_ASSOC_TABLE(Pure_pointer_to_kind, var_sym*,
		    Pure_pointer_kind,var_sym_hash,compare_vars);



/* Begin the fg class definitions. These classes are based on
   parellel suif and the sharlit package. This enable the defined 
   classes to be processed by sharlit. Various classes correspond  
   to the different suif structure, and provide interface to suif*/

class fg
{ 
  Tree_to_fg_node tree_map;
  Label_to_fg_node label_map;
  fg_instr_xarray patch_list;
  Variable_bit_index var_index_map;
  Variable_list bit_index_map;
  Variable_list ineligible_vars;
  bit_set ineligible_var_set;

  Pointerass_bit_index pointerass_map;
  Pointerass_list pointerass_list;
  Pure_pointer_to_kind purepointer_kind_map;
  
  void collect_all_pure(proc_sym *p);
  void collect_pointerass_info(instruction *);
  void assign_pointerass_index(instruction *);
  void walk_for_purepointer_and_kind(tree_node *);
  void walk_for_purepointer_and_kind(tree_node_list *);
  
  void collect_vars(instruction *in);  //called by enter 
  void collect_vars(operand o);  //call assign_bit_index when meet symbol; called by the above function
  void walk_symtabs(block_symtab *);
  void assign_bit_index(var_sym *); //var_index_map.associate(var,bit_index_map.hi)

  void add_var(operand o,bit_set *);//bit_set *vars->add(get_var_index((var_sym *)symbol));
  void collect_referenced_vars(instruction *i, bit_set *, bit_set *);//calls the add_var function;called by walk_for_used..
  void walk_for_used_var_info(tree_node *, bit_set *, bit_set *);
  void walk_for_used_var_info(tree_node_list *, bit_set *, bit_set *);

public:
  int            no_control_flow_analysis;
  CFG_1          *const forward_graph;
  CFG_1_reversed *const backward_graph;
  proc_sym       *const procedure;

  fg(proc_sym *);
  virtual ~fg() { 
    if (backward_graph) delete backward_graph;
    if (forward_graph) delete forward_graph;
  }


  int is_subgraph(CFGnode *)
  {
    assert(0);
    return 0;
  }

  CFG_0 *get_subgraph(CFGnode *)
  {
    assert(0);
    return NULL;
  }

  /* These methods are for creating flow graphs from SUIF trees */
  int enter(fg_node *u) { return forward_graph->enter((CFGnode *)u); }
  fg_node *enter(fg_node *pred, fg_node *u);
  fg_node *enter(fg_node *pred, tree_node *, block_symtab *);
  fg_node *enter(fg_node *pred, tree_node_list *,
			 block_symtab *);
  void analyze();
  inline void link(CFGnode *u, CFGnode *v)
    { if(u && v) forward_graph->link(u->unique, v->unique); }

  /*
  associating labels with nodes for use in resolving forward
  branches
  */
  void associate_label(label_sym *l,fg_node *u)
    {
      if(label_map.associate(l,u)==0)
	error_line(1,NULL,"failed to associate label");
    }

  fg_node *lookup_label(label_sym *l)
    {
      fg_node *u;
      return label_map.lookup(l,&u) ? u : 0;
    }

  void print(FILE *);

      /* Methods to access information about variables used in
	this procedures */
  inline int set_size() { return bit_index_map.hi; }
  int get_var_index(var_sym *);//var_index_map.lookup(var_sym *v, &index)
  int get_var_index(operand);

  boolean is_pure(var_sym *);
  inline int pointerass_num() { return pointerass_list.hi; }
  int get_pointerass_index(instruction *);
  instruction *get_pointerass(int i) { return pointerass_list[i]; }

  bit_set &get_ineligible_vars() { return ineligible_var_set; }
  var_sym *get_var(int i) { return bit_index_map[i]; }
};

enum fg_node_kinds
{
  NODE_SOURCE,
  NODE_SINK,
  NODE_INSTR,
  NODE_LABEL,
  NODE_LOOP,
  NODE_FOR_LOOP,
  NODE_PRE_TEST,
  NODE_IF,
  NODE_BLOCK
};
    
/*

fg_node wraps up a tree node as a CFGnode, so that they can be
manipulated by Sharlit.  Each kind of tree nodes has a corresponding
derivative of fg_node.  The derivative keep track of fake
nodes introduced to build the control-flow graph.  See comments above
each derivative for an explanation.

*/

class fg_node: public CFGnode
{
protected:
  tree_node *tnode;
  fg *cfg;

public:

  fg_node(tree_node *tn, fg *g)
  {
    tnode = tn;
    cfg = g;
  }
  virtual ~fg_node() { }; 

  virtual fg_node_kinds kind() = 0;
  virtual instruction *instr() { return NULL; }
  tree_node *tree() { return tnode; }
};

class fg_source: public fg_node
{
public:
  fg_source(fg *g):fg_node(0,g) { }
  fg_node_kinds kind() { return NODE_SOURCE; }
};

class fg_sink: public fg_node
{
public:
  fg_sink(fg *g):fg_node(0,g) { }
  fg_node_kinds kind() { return NODE_SINK; }
};

/*

fg_instr wraps up tree_instr node.  fg_label is an
empty node used to indicate specific points within flow graphs of
structures like loops, for-loops, and ifs.

*/
class fg_instr: public fg_node
{
public:
  fg_instr(tree_instr *t, fg *g):fg_node(t,g) { }
  fg_node_kinds kind() { return NODE_INSTR; }
  instruction *instr()
  {
    return tnode->is_instr() ? ((tree_instr *)tnode)->instr() : NULL;
  }
  
  


};

class fg_label: public fg_node
{
public:
  fg_label(fg *g):fg_node(0,g) { }
  fg_node_kinds kind() { return NODE_LABEL; }
};

/*

fg_loop wraps up a tree_loop node.  Each loop has two extra
nodes: a node associated with the continue label and a node associated
with the break label.  The ``continue_node'' heads the flow graph that
corresponds to the test part of the loop.

*/
class fg_loop: public fg_node
{
public:
  fg_node *continue_node;
  fg_node *end_node;

  fg_loop(tree_loop *, fg *g);

  fg_node_kinds kind() { return NODE_LOOP; }
};

/*

fg_for_loop wraps up a tree_for node. The flow graph of a
for-loop starts with the instructions that comprise the lb, ub and the
step computations, followed by a pre-test node, then the computations
in the landing pad, then the fg_for_loop node itself, then the
body, then the continue node, and finally the end_node.

*/
class fg_for_loop: public fg_node
{
public:
  block_symtab *scope;
  bit_set read_vars;
  bit_set write_vars;

  fg_pre_test *pre_test;  /* the pre-test preceding the landing pad */
  fg_node *continue_node; /* the end, where the step and test occurs */
  fg_node *end_node;

  fg_for_loop(tree_for *, fg *, block_symtab *);
  ~fg_for_loop() {};

  fg_node_kinds kind() { return NODE_FOR_LOOP; }

#ifdef HAWG4
  int in_scope(var_sym *v)
  { return NULL!=scope->lookup_symid(v->symid()); }
#else
      /* cosmetic change for hawg 5.0 */
  int in_scope(var_sym *v)
  { return NULL!=scope->lookup_sym_id(v->sym_id()); }
#endif
};

class fg_pre_test: public fg_node
{
public:
  fg_for_loop *loop;

  fg_pre_test(tree_for *, fg *, fg_for_loop *);
  fg_node_kinds kind() { return NODE_PRE_TEST; }
};

/*

fg_if wraps up an if node.  The if has only one extra node, an
end node.

*/

class fg_if: public fg_node
{
public:
  fg_node *else_start;
  fg_node *end_node;

  fg_if(tree_if *, fg *);
  fg_node_kinds kind() { return NODE_IF; }
};

/*

fg_block heads the flow graph of a tree_block.

*/

class fg_block: public fg_node
{
public:
  fg_block(tree_block *b, fg *g):fg_node(b,g) { }
  fg_node_kinds kind() { return NODE_BLOCK; }
};
