#include "Log.h"

bool Log::bufferBlock(uint32_t blockId) {
  // Check valid id.
  if (!isValidBlockId(blockId)) return false;

  // If the block is dirty, write it back.
  if (!bufferBlockWriteBack()) return false;

  // If the block is already loaded, just return.
  if (blockBuffer.blockId == blockId && blockBuffer.ready) return true;

  // Read in block.
  if (!diskRead(getBlockOffset(blockId), blockBuffer.block, BLOCK_SIZE))
    return false;

  blockBuffer.blockId = blockId;
  blockBuffer.ready = true;

  return true;
}
bool Log::bufferBlockWriteEntry(uint32_t blockId,
                                uint32_t entryId,
                                const Entry& entry) {
  if (!isValidEntryId(entryId)) return false;
  if (!bufferBlock(blockId)) return false;

  bufferBlockAsBlock()->entries[entryId] = entry;
  blockBuffer.dirty = true;

  return true;
}
bool Log::bufferBlockReadEntry(uint32_t blockId,
                               uint32_t entryId,
                               Entry *entry) {
  if (!isValidEntryId(entryId)) return false;
  if (!bufferBlock(blockId)) return false;

  *entry = bufferBlockAsBlock()->entries[entryId];

  return true;
}
bool Log::bufferBlockReadHeader(uint32_t blockId, BlockHeader *header) {
  if (!bufferBlock(blockId)) return false;

  *header = bufferBlockAsBlock()->header;

  return true;
}
bool Log::bufferBlockWriteHeader(uint32_t blockId, const BlockHeader& header) {
  if (!bufferBlock(blockId)) return false;

  bufferBlockAsBlock()->header = header;
  blockBuffer.dirty = true;

  return true;
}
bool Log::bufferBlockFromMem(uint32_t blockId, const void *src, size_t size) {
  if (!bufferBlock(blockId)) return false;

  memset(blockBuffer.block, 0, BLOCK_SIZE);
  memcpy(blockBuffer.block, src, size);

  blockBuffer.dirty = true;

  return true;
}
bool Log::bufferBlockToMem(uint32_t blockId, void *dest, size_t size) {
  if (!bufferBlock(blockId)) return false;

  memcpy(dest, blockBuffer.block, size);

  return true;
}
bool Log::bufferBlockWriteBack() {
  if (!blockBuffer.ready || !blockBuffer.dirty) return true;

  off_t offset = getBlockOffset(blockBuffer.blockId);
  if (!diskWrite(offset, blockBuffer.block, BLOCK_SIZE))
    return false;

  blockBuffer.dirty = false;

  return true;
}