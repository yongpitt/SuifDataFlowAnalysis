/* file "main.cc" */

//#pragma implementation "variables.h"

#include <stdio.h>
#include <string.h>
#include <suif1.h>
#include <nsharlit/cfg.h>
#include <nsharlit/sharlit.h>
#include <nsharlit/sharlit_test.h>
#include "main.h"
#include "assoc.h"
//#include "variables.h"
#include "fg.h"
#include "pointer_analysis.h"


void do_alias(proc_sym *);

/*
This table matches up driver programs with pass names.
*/

typedef void (*Pass_driver)(proc_sym *);

struct New_passes
{
  char *pass_name;
  Pass_driver driver;
} new_passes[]
= {
  {"alias", &do_alias },
  {NULL, NULL }
};

/*

Keywords

*/
char *k_alias;

/*

Flags

*/

int print_graph = 0;
int apply_do_loops = 0;
int verbose = 0;

void usage()
{
  fprintf(stderr,"usage: pcompile [flags] src dst\n");
  exit(1);
}


main(int argc, char **argv)
{
  char *last_pass;
    
  /* initialize */
  start_suif(argc, argv);
  ANNOTE(k_alias, "alias", TRUE);

  for(argv++,argc--;argc;argv++,argc--)
    {
      if(argv[0][0]=='-')
  switch(argv[0][1])
	  {

	  case 'P':
	    last_pass=&argv[0][2];
	    break;

	  case 'g':
	    print_graph = 1;
	    break;

	  case 'd':
	    apply_do_loops = 1;
	    break;

	  case 'V':
	    verbose = 1;
	    break;

	  default:
	    usage();
	  }
      else
	break;
    }

  if(argc!=2)
    usage();

  return new_suif_phases(last_pass,argv[0],argv[1]);
}


/*

Look up the requested phase and process them.

*/

int new_suif_phases(const char *pass,
		    char *in_suif, char *out_suif)
{
#ifdef HAWG4
  file_set *fs = new file_set;
#else
  file_set *fs = fileset;
#endif

  file_set_entry *stream;
  proc_sym *procedure;
  int i;

  /* open the input and output */
  stream = fs->add_file(in_suif,out_suif);

  /* find out what pass we should use */
  i = 0;
  while(new_passes[i].pass_name)
    {
      if(strcmp(pass,new_passes[i].pass_name)==0)
        break;
      ++i;
    }
  if(new_passes[i].pass_name==0)
    error_line(1, NULL, "No such phase: %s", pass);

  /* process each procedure */
  stream->reset_proc_iter();
  while((procedure = stream->next_proc()) != NULL)
    {
#ifdef HAWG4
	stream->read_proc(procedure,FALSE); /* read in as list of instrs */
	new_passes[i].driver(procedure);
	stream->write_proc(procedure);
	stream->flush_proc(procedure);
#else
	procedure->read_proc(TRUE, FALSE); //modified to read in as expression
	new_passes[i].driver(procedure);
	procedure->write_proc(stream);
	procedure->flush_proc();
#endif
    }

  delete fs;
#ifndef HAWG4
  fileset = 0;
#endif
  return 0;
}

/*

Pointer analysis routine

*/
void do_alias(proc_sym *procedure)
{
  fg *cfg;
  Pointer_analysis *problem;

  /* Build a Flow graph from the procedure */
  printf("------procedure %s is in processing------- \n", procedure->name());

  cfg = new fg(procedure);
  
  if(verbose)
    fprintf(stderr, "procedure %s\n", procedure->name());
  if(print_graph)
    {

	cfg->no_control_flow_analysis = 1;
	cfg->analyze();
	cfg->forward_graph->debug_level = 1;
	cfg->forward_graph->no_control_flow_analysis = 1;
	cfg->forward_graph->CFG_1::analyze();
	cfg->print(stderr);
	cfg->forward_graph->CFG_0::analyze();
	return;
    }
  else
    cfg->analyze();

  /* set up the data-flow problem */
  problem = new Pointer_analysis(cfg);
  problem->n_solve(100);
  
  /* cleanup */
  delete problem;
  delete cfg;

  /* that's all, folks */
}
