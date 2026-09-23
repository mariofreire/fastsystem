#define ERRNO_ADDRESS 0x850000

int * __errno_location (void)
{
  return (int *) ERRNO_ADDRESS;
}
