/* file "assoc.h" */

/*  Copyright (c) 1994 Stanford University

    All rights reserved.

    This software is provided under the terms described in
    the "suif_copyright.h" include file. */

#include <suif_copyright.h>

#define DECLARE_ASSOC_TABLE(NAME,CAR,CDR,SIGF,COMPAREF)  		      \
class NAME ## _hash_e: public hash_e					      \
{									      \
public:									      \
  CAR car;								      \
  CDR cdr;								      \
  NAME ## _hash_e(CAR a): hash_e(SIGF(a)) { car = a; }			      \
};									      \
									      \
inline int NAME ## _compare(hash_e *a, hash_e *b)			      \
{									      \
  return COMPAREF(&((NAME ## _hash_e *)a)->car,				      \
		  &((NAME ## _hash_e *)b)->car);			      \
}									      \
									      \
class NAME: private hash_table						      \
{									      \
public:									      \
  NAME(int sz):hash_table(NAME ## _compare, sz) { }			      \
  boolean lookup(CAR a, CDR *d)						      \
  {									      \
    NAME ## _hash_e elem(a), *ep;					      \
    ep = (NAME ## _hash_e *)hash_table::lookup(&elem);			      \
    if(ep)								      \
      {									      \
	if(d) *d = ep->cdr;						      \
	return 1;							      \
      }									      \
    else								      \
      return 0;								      \
  }									      \
									      \
  boolean associate(CAR a, CDR d)					      \
  {									      \
    NAME ## _hash_e elem(a), *ep;					      \
    ep = (NAME ## _hash_e *)hash_table::lookup(&elem);			      \
    if(ep)								      \
      return 0;								      \
    else								      \
      {									      \
	ep = new NAME ## _hash_e(a);					      \
	ep->cdr = d;							      \
	hash_table::enter(ep);						      \
	return 1;							      \
      }									      \
  }									      \
};
