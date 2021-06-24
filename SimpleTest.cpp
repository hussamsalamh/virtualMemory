#include "VirtualMemory.h"
#include "tests/PhysicalMemory.h"
#include <cstdio>
#include <cassert>

int main(int argc, char **argv)
{

  for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i)
    {
      printf("writing to %llu\n", (long long int) i); // 46 to enter case 3
      if (i == 46){
          VMwrite(5 * i * PAGE_SIZE, i);
          printRAM();
          continue;
        }
      VMwrite(5 * i * PAGE_SIZE, i);
      printRAM();
    }


  for (uint64_t i = 0; i < (2 * NUM_FRAMES); ++i) {
      word_t value;
      VMread(5 * i * PAGE_SIZE, &value);
      printf("reading from %llu %d\n", (long long int) i, value);
      printRAM();
      assert(uint64_t(value) == i);

    }

  printf("\nsuccess\n");

  return 0;
}