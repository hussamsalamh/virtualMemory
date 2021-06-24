#include "VirtualMemory.h"
#include "PhysicalMemory.h"

void clearTable (uint64_t frameIndex)
{
  for (uint64_t i = 0; i < PAGE_SIZE; ++i)
    {
      PMwrite(frameIndex * PAGE_SIZE + i, 0);
    }
}



void VMinitialize() {
  clearTable(0);
}


/**
 * this function take the part from the address
 * @param virtualAdd virtual address
 * @param arr array
 * @param offset offset
 */
void translateAddress (uint64_t virtualAdd, word_t *arr,
                       word_t *offset)
{
  uint64_t mask = PAGE_SIZE - 1;
  *offset = virtualAdd & mask;
  virtualAdd >>= (uint64_t)OFFSET_WIDTH;
  for (int i = TABLES_DEPTH; i > 0; i--)
    {
      arr[i - 1] = virtualAdd & mask; // set the part in the array in order
      virtualAdd >>= (uint64_t)OFFSET_WIDTH;
    }
}

/**
 * This struct is used to find the
 * empty frame or unused frame
 */
struct infoFrame {
    uint64_t parentAddressEmpty;
    uint64_t emptyFrameIndex;
    uint64_t maxIndex;
    bool foundEmptyFrame;
};


/**
 * This struct for finding the evicted page
 *
 */
struct evictedFrameInfo{
    uint64_t parentAddress;
    uint64_t evictedFrame;
    unsigned long maxWeight;
    uint64_t numPage; // saving the number of the page
};


/**
 * This function apply the dfs in the table to find the
 * unused frame or empty frame
 * @param info information about the frame (holding the data)
 * @param depth depth of the table
 * @param currentFrame current frame
 * @param parentAddress parent address
 */
void dfs (infoFrame *info, unsigned long depth,
          uint64_t currentFrame, uint64_t parentAddress)
{

  if (currentFrame > info->maxIndex)
    {
      // Check if the current is larger than the max index
      info->maxIndex = currentFrame;
    }

  if (depth == TABLES_DEPTH) return;

  word_t frame;
  bool empty = true;
  for (int i = 0;i < PAGE_SIZE; i++)
    {
      uint64_t address = currentFrame * PAGE_SIZE + i;
      PMread(address, &frame);
      if (frame != 0)
        {
          empty = false;
          dfs(info, depth+ 1, frame, address);
        }
    }
  if (empty && !info->foundEmptyFrame)
    {
      // This used for empty frame
      info->emptyFrameIndex = currentFrame;
      info->parentAddressEmpty = parentAddress;
      info->foundEmptyFrame = empty;
    }
}


/**
 * This function get the evicted frame
 * @param info information about the evicted frame
 * @param depth current depth of the table
 * @param current current address
 * @param parentAddress parent address
 * @param pageNum the number of the page
 * @param weight the weight of the path
 */
void evictedFrame(evictedFrameInfo* info,unsigned long depth,
                  uint64_t current, uint64_t parentAddress,
                  uint64_t pageNum,unsigned long weight){

  weight = (current%2==0)?weight+=WEIGHT_EVEN:weight+=WEIGHT_ODD;
  if(depth == TABLES_DEPTH){
      weight = (pageNum%2==0)?weight+=WEIGHT_EVEN:weight+=WEIGHT_ODD;
      // Checking if current weight are larger than the max
      // This hold to take the maximum weight and the smallest number page
      // If there's exist more than one equal with the weight
      if (weight > info->maxWeight ||
      (weight == info->maxWeight && pageNum < info->numPage)){
          info->numPage = pageNum;
          info->maxWeight = weight;
          info->parentAddress = parentAddress;
          info->evictedFrame = current;
        }
      return;
    }
  word_t frame;
  for (int i = 0; i < PAGE_SIZE; ++i)
    {
      uint64_t address = current*PAGE_SIZE+i;
      PMread(address,&frame);
      if (frame !=0){
          evictedFrame(info,depth+1,frame,address,
                       (pageNum<<(uint64_t)OFFSET_WIDTH)+i,weight);
        }
    }
}


/**
 * This function find the empty frame
 * it use the dfs algorithm and eviceted frame function
 * @param address addres
 * @param preIndex number of frame this to avoid taking the the same frame
 * @return new frame
 */
uint64_t findEmptyFrame (uint64_t address,
                         uint64_t preIndex)
{
  infoFrame info{0, 0,0,false};
  dfs(&info, 0, 0, 0);
  // Checking if there's an empty frame
  if(info.foundEmptyFrame && info.emptyFrameIndex != preIndex){
      PMwrite(info.parentAddressEmpty, 0);
      clearTable(info.emptyFrameIndex);
      PMwrite(address,info.emptyFrameIndex);
      return info.emptyFrameIndex;
    }
    // This to check if there's unused frame
  else if (info.maxIndex + 1 < NUM_FRAMES)
    {
      clearTable(info.maxIndex+ 1);
      PMwrite(address, info.maxIndex  + 1);
      return info.maxIndex + 1;
    }
    // This to check the evicted frame
  evictedFrameInfo ev{0,0,0,0};
  evictedFrame(&ev,0,0,0,0,0);
  PMwrite(ev.parentAddress,0);
  PMevict(ev.evictedFrame,ev.numPage);
  clearTable(ev.evictedFrame);
  PMwrite(address,ev.evictedFrame);
  return ev.evictedFrame;
}

/**
 * This function restore the page
 * @param currentFrame current frame
 * @param arrOff array of the indeies
 * @param offset offset
 * @param virtualAddress virtual address
 * @param preFrame the previous index
 * @return the physical address
 */
uint64_t restoreFrame (uint64_t currentFrame, word_t arrOff,
                       word_t offset, uint64_t virtualAddress,
                       uint64_t preFrame)
{
  // This to check if the depth of the table
  // is equal to zero then we need to save it in the first frame
  // Which is the root frame
  if (TABLES_DEPTH ==0 ){
      return virtualAddress;
  }
  word_t frame = 0;
  uint64_t address = currentFrame * PAGE_SIZE + arrOff;
  PMread(address, &frame);
  if (frame == 0)
    {
      frame = findEmptyFrame(address,preFrame);
      if (frame == -1)return -1;
      PMrestore(frame, virtualAddress >> (uint64_t) OFFSET_WIDTH);
    }
  return frame * PAGE_SIZE + offset;
}

/**
 * This function used to travers on the tree
 * @param virtualAdd virtual address
 * @return physical address
 */
uint64_t traverseTree (uint64_t virtualAdd)
{
  word_t offset;
  word_t arr[TABLES_DEPTH];
  translateAddress(virtualAdd, arr, &offset);
  word_t currentFrame = 0;
  word_t frame;
  // This part to mapping the address without making
  // restore for the frame
  for (int i = 0; i < TABLES_DEPTH - 1; ++i)
    {
      uint64_t address = currentFrame * PAGE_SIZE + arr[i];
      PMread(address, &frame);
      if (frame == 0)
        {
          frame = findEmptyFrame(address,currentFrame);
        }
      currentFrame = frame;
    }
    // In this part we restore the page
  return restoreFrame(currentFrame, arr[TABLES_DEPTH - 1], offset, virtualAdd,currentFrame);
}

/* reads a word from the given virtual address
 * and puts its content in *value.
 */
int VMread (uint64_t virtualAddress, word_t *value)
{
  if (virtualAddress >= VIRTUAL_MEMORY_SIZE) return 0;
  uint64_t add = traverseTree(virtualAddress);
  PMread(add, value);
  return 1;
}

/* writes a word to the given virtual address
 */
int VMwrite (uint64_t virtualAddress, word_t value)
{
  if (virtualAddress >= VIRTUAL_MEMORY_SIZE) return 0;
  uint64_t add = traverseTree(virtualAddress);
  PMwrite(add, value);
  return 1;
}
