//#include "VirtualMemory.h"
//#include "PhysicalMemory.h"
//
//void clearTable (uint64_t frameIndex)
//{
//  for (uint64_t i = 0; i < PAGE_SIZE; ++i)
//    {
//      PMwrite(frameIndex * PAGE_SIZE + i, 0);
//    }
//}
//
//void translateAddress (uint64_t virtualAdd, word_t *arr,
//                       word_t *offset)
//{
//  uint64_t mask = PAGE_SIZE - 1;
//  *offset = virtualAdd & mask;
//  virtualAdd >>= OFFSET_WIDTH;
//  for (int i = TABLES_DEPTH; i > 0; i--)
//    {
//      arr[i - 1] = virtualAdd & mask;
//      virtualAdd >>= OFFSET_WIDTH;
//    }
//}
//
//struct infoFrame {
//    uint64_t parentAddressEmpty;
//    uint64_t emptyFrameIndex;
//    uint64_t maxIndex;
//    bool foundEmptyFrame;
//};
//
//
//struct evictedFrameInfo{
//    uint64_t parentAddressEmpty;
//    uint64_t evictedFrame;
//    unsigned long maxWeight;
//    uint64_t numPage;
//};
//
//void dfs (infoFrame *info, unsigned long depth,
//          uint64_t currentFrame, uint64_t parentAddressEmpty)
//{
//
//  if (currentFrame > info->emptyFrameIndex && !info->foundEmptyFrame)
//    {
//      info->maxIndex = currentFrame;
//      info->parentAddressEmpty = parentAddressEmpty;
//    }
//
//  if (depth == TABLES_DEPTH) return;
//
//  word_t frame;
//  bool empty = true;
//  for (int i = 0;i < PAGE_SIZE; i++)
//    {
//      uint64_t address = currentFrame * PAGE_SIZE + i;
//      PMread(address, &frame);
//      if (frame != 0)
//        {
//          empty = false;
//          dfs(info, depth+ 1, frame, address);
//        }
//    }
//  if (empty && !info->foundEmptyFrame)
//    {
//      info->emptyFrameIndex = currentFrame;
//      info->parentAddressEmpty = parentAddressEmpty;
//      info->foundEmptyFrame = empty;
//    }
//}
//
//
//void evictedFrame(evictedFrameInfo* info,unsigned long depth,
//                  uint64_t current, uint64_t parentAddressEmpty,
//                  uint64_t pageNum,unsigned long weight){
//
//  if(depth == TABLES_DEPTH){
//    weight = (pageNum%2==0)?weight+=WEIGHT_EVEN:weight+=WEIGHT_ODD;
//    if (weight > info->maxWeight || (weight == info->maxWeight && pageNum < info->numPage)){
//      info->numPage = pageNum;
//      info->maxWeight = weight;
//      info->parentAddressEmpty = parentAddressEmpty;
//      info->evictedFrame = current;
//    }
//    return;
//  }
//  word_t frame;
//  for (int i = 0; i < PAGE_SIZE; ++i)
//    {
//      uint64_t address = current*PAGE_SIZE+i;
//      PMread(address,&frame);
//      if (frame !=0){
//        weight = (frame%2==0)?weight+=WEIGHT_EVEN:weight+=WEIGHT_ODD;
//        evictedFrame(info,depth+1,frame,address,
//                     (pageNum<<OFFSET_WIDTH)+i,weight);
//      }
//    }
//}
//
//
//uint64_t findEmptyFrame (uint64_t address)
//{
//  infoFrame info{0, 0,0, false};
//  dfs(&info, 0, 0, 0);
//  if(info.foundEmptyFrame){
//      PMwrite(info.parentAddressEmpty,0);
//      PMwrite(address,info.emptyFrameIndex);
//      return info.emptyFrameIndex;
//  }
//  else if (info.maxIndex + 1 < NUM_FRAMES)
//  {
//        clearTable(info.maxIndex+ 1);
//        PMwrite(address, info.emptyFrameIndex + 1);
//        return info.maxIndex + 1;
//  }
//    evictedFrameInfo ev{0,0,0,0};
//    evictedFrame(&ev,0,0,0,0,0);
//    PMwrite(ev.parentAddressEmpty,0);
//    PMevict(ev.evictedFrame,ev.numPage);
//    PMwrite(address,ev.evictedFrame);
//    return ev.evictedFrame;
//}
//
//uint64_t restoreFrame (uint64_t currentFrame, word_t arrOff,
//                       word_t offset, uint64_t virtualAddress)
//{
//  word_t frame = 0;
//  uint64_t address = currentFrame * PAGE_SIZE + arrOff;
//  PMread(address, &frame);
//  if (frame == 0)
//    {
//      frame = findEmptyFrame(address);
//      if (frame == -1)return -1;
//      PMrestore(frame, virtualAddress >> (uint64_t) OFFSET_WIDTH);
//    }
//  return frame * PAGE_SIZE + offset;
//}
//
//uint64_t traverseTree (uint64_t virtualAdd)
//{
//  word_t offset;
//  word_t arr[TABLES_DEPTH];
//  translateAddress(virtualAdd, arr, &offset);
//  word_t currentFrame = 0;
//  word_t frame;
//  for (int i = 0; i < TABLES_DEPTH - 1; ++i)
//    {
//      uint64_t address = currentFrame * PAGE_SIZE + arr[i];
//      PMread(address, &frame);
//      if (frame == 0)
//        {
//          frame = findEmptyFrame(address);
//        }
//      if (frame == -1) return -1;
//      currentFrame = frame;
//    }
//  return restoreFrame(currentFrame, arr[TABLES_DEPTH - 1], offset, virtualAdd);
//}
//
///* reads a word from the given virtual address
// * and puts its content in *value.
// */
//int VMread (uint64_t virtualAddress, word_t *value)
//{
//  if (virtualAddress >= VIRTUAL_MEMORY_SIZE) return 0;
//  uint64_t add = traverseTree(virtualAddress);
//  if (add == -1) return 0;
//  PMread(add, value);
//  return 1;
//}
//
///* writes a word to the given virtual address
// */
//int VMwrite (uint64_t virtualAddress, word_t value)
//{
//  if (virtualAddress >= VIRTUAL_MEMORY_SIZE) return 0;
//  uint64_t add = traverseTree(virtualAddress);
//  if (add == -1) return 0;
//  PMwrite(add, value);
//  return 1;
//}
