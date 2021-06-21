#include "VirtualMemory.h"
#include "PhysicalMemory.h"

void clearTable(uint64_t frameIndex) {
    for (uint64_t i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}


/*
 * Initialize the virtual memory
 */
void VMinitialize() {
    clearTable(0);
}



// Arr contain where to go
// Contain in each cell which enter must go and find the offset
void translateAddress(uint64_t virtualAddress,uint64_t &offset,uint64_t *arr )
{

  uint64_t cast = PAGE_SIZE - 1;
  offset = virtualAddress & cast;
  virtualAddress >>= cast;
  for (uint64_t i = 0; i < TABLES_DEPTH; ++i)
    { //TODO : Check if the type of i must be uint64_t or word_t
      arr[TABLES_DEPTH -i - 1] = virtualAddress & cast;
      virtualAddress >>= cast;
    }
}

bool usedPage(uint64_t add){}

void dfs(uint64_t *arr,uint64_t offset){








}





int findEmpty(uint64_t *arr,uint64_t offset){






  return 1;
}
/**
 * This function travers over tree to find page
 * and return it
 * @param virtualAddress virtual address
 * @return frame in success , -1 otherwise
 */
uint64_t traversTree(uint64_t virtualAddress){
  uint64_t arr[TABLES_DEPTH];
  uint64_t offset;
  translateAddress(virtualAddress,offset,arr);
  word_t currentFrame = 0;word_t frame;
  for (uint64_t add : arr)
    {
      PMread(currentFrame * PAGE_SIZE + add,&frame);
      if (frame == 0){
          if (findEmpty(arr,offset)) return -1;
      }
      currentFrame = frame;
    }
  return frame * PAGE_SIZE + offset;
}



/* reads a word from the given virtual address
 * and puts its content in *value.
 */
int VMread(uint64_t virtualAddress, word_t* value) {
  uint64_t add = traversTree(virtualAddress);
  if (add == -1 ) return 0;
  PMread(add,value);
  return 1;
}

/* writes a word to the given virtual address
 */
int VMwrite(uint64_t virtualAddress, word_t value) {
  uint64_t add = traversTree(virtualAddress);
  if (add == -1 ) return 0;
  PMwrite(add,value);
  return 1;
}
