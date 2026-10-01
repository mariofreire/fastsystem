#include "errno.h"

int * __errno_location (void)
{
  return (int *) ERRNO_ADDRESS;
}
