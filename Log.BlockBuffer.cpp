#include "Log.h"

bool Log::BlockBuffer::ready = false;
bool Log::BlockBuffer::dirty = false;
uint32_t Log::BlockBuffer::id;
void *Log::BlockBuffer::block;

// This is for OS X.
#ifndef O_DIRECT
  void *aligned_alloc(int align, size_t size) {
    return malloc(size);
  }
#endif

void Log::BlockBuffer::init() {
  block = aligned_alloc(0x1000, BLOCK_SIZE);
}

bool Log::BlockBuffer::prepareBlock(uint32_t blockId) {
  // Check valid id.
  if (!isValidBlockId(blockId)) return false;

  // If the block is dirty, write it back.
  if (!writeBack()) return false;

  // If the block is already loaded, just return.
  if (id == blockId && ready) return true;

  // Read in block.
  if (!Disk::diskRead(getBlockOffset(blockId), block, BLOCK_SIZE))
    return false;

  id = blockId;
  ready = true;

  return true;
}
bool Log::BlockBuffer::writeEntry(uint32_t blockId,
                                  uint32_t entryId,
                                  const Entry& entry) {
  if (!isValidEntryId(entryId)) return false;
  if (!prepareBlock(blockId)) return false;

  asBlock()->entries[entryId] = entry;
  dirty = true;

  return true;
}
bool Log::BlockBuffer::readEntry(uint32_t blockId,
                                 uint32_t entryId,
                                 Entry *entry) {
  if (!isValidEntryId(entryId)) return false;
  if (!prepareBlock(blockId)) return false;

  *entry = asBlock()->entries[entryId];

  return true;
}
bool Log::BlockBuffer::readHeader(uint32_t blockId, BlockHeader *header) {
  if (!prepareBlock(blockId)) return false;

  *header = asBlock()->header;

  return true;
}
bool Log::BlockBuffer::writeHeader(uint32_t blockId, const BlockHeader& header) {
  if (!prepareBlock(blockId)) return false;

  asBlock()->header = header;
  dirty = true;

  return true;
}
bool Log::BlockBuffer::fromMem(uint32_t blockId, const void *src, size_t size) {
  if (!prepareBlock(blockId)) return false;

  memset(block, 0, BLOCK_SIZE);
  memcpy(block, src, size);

  dirty = true;

  return true;
}
bool Log::BlockBuffer::toMem(uint32_t blockId, void *dest, size_t size) {
  if (!prepareBlock(blockId)) return false;

  memcpy(dest, block, size);

  return true;
}
bool Log::BlockBuffer::writeBack() {
  if (!ready || !dirty) return true;

  off_t offset = getBlockOffset(id);
  if (!Disk::diskWrite(offset, block, BLOCK_SIZE))
    return false;

  dirty = false;

  return true;
}