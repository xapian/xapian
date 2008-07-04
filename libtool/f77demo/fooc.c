#include "foo.h"
#include <stdio.h>


int csub(int arg)
{
  return (2*arg);
}


int fwrapper(int arg)
{
  int res;
  printf("Calling the Fortran 77 subroutine from the C wrapper...\n");
  F77_FUNC(fsub,FSUB)(&arg,&res);
  printf("Returned from the Fortran 77 subroutine...\n");
  return res;
}
