#include <stdio.h>
#include "utilities.h"
#include "Log.h"

const char *Log::DEV_FILE;

Log::Metadata Log::metadata;
Log::BlockBuffer Log::blockBuffer;

bool Log::verbose = false;

uint32_t Log::currentHead = 1;
uint32_t Log::currentEntry = 0;

int Log::diskFd = -1;
int Log::lastError = 0;

bool Log::init(const char *devFile) {
  invariant(sizeof(BlockHeader) == 8);
  invariant(sizeof(Block) == 4088);

  DEV_FILE = devFile;

  blockBuffer.block = (Block *)aligned_alloc(0x1000, sizeof(Block));

  return readMetadata();
}
bool Log::playback(Entry *entry) {
  if (outOfSpace()) return false;

  if (currentHead == 0) {
    if (!moveToNextBlock()) return false;
  }

  BlockHeader header;
  if (!readBlockHeader(currentHead, &header)) return false;

  if (!readBlockEntry(header, currentHead, currentEntry, entry)) {
    // If reached end of block, move to next block.
    if (!isValidEntryId(currentEntry)) {
      if (!moveToNextBlock()) return false;
      return playback(entry);
    }
    return false;
  }

  return moveToNextEntry();
}
bool Log::add(uint32_t opCode, uint64_t id1, uint64_t id2) {
  if (outOfSpace()) return false;

  Entry entry = { opCode, id1, id2 };

  // Write entry to current space.
  if (!writeBlockEntry(currentHead, currentEntry, &entry)) return false;

  // Update the block header.
  BlockHeader header = { metadata.generation, currentEntry + 1 };
  if (!writeBlockHeader(currentHead, &header)) return false;

  return moveToNextEntry();
}
bool Log::finish() {
  if (!isOpen()) return true;
  return diskClose();
}

bool Log::diskOpen() {
  if (isOpen()) return true;

  diskFd = open(DEV_FILE, O_RDWR | O_SYNC | O_DIRECT);
  if (diskFd == -1) {
    setErrno(errno);
    return false;
  }

  return true;
}
bool Log::isOpen() {
  if (diskFd == -1) {
    setErrno(-2);
    return false;
  }

  int fdOpen = fcntl(diskFd, F_GETFD);
  if (fdOpen == -1 && errno == EBADF) {
    setErrno(errno);
    return false;
  }

  return true;
}
bool Log::diskClose() {
  diskFd = -1;
  return close(diskFd) != -1;
}

bool Log::diskSeek(off_t offset) {
  off_t off = lseek(diskFd, offset, SEEK_SET);

  if (off == -1) {
    setErrno(errno);
    return false;
  }
  return true;
}

bool Log::diskRead(off_t offset, void *buf, size_t size) {
  if (!diskOpen()) return false;
  if (!diskSeek(offset)) return false;

  ssize_t readSize = read(diskFd, buf, size);
  if (readSize != (ssize_t)size) {
    setErrno(errno);
    return false;
  }

  return true;
}

bool Log::diskWrite(off_t offset, const void *data, size_t size) {
  if (!diskOpen()) return false;
  if (!diskSeek(offset)) return false;

  ssize_t writeSize = write(diskFd , data, size);
  if (writeSize != (ssize_t)size) {
    setErrno(errno);
    return false;
  }
  return true;
}

bool Log::bufferBlock(uint32_t blockId) {
  if (!isValidBlockId(blockId)) return false;

  if (blockBuffer.blockId == blockId &&
      blockBuffer.ready &&
      !blockBuffer.dirty) return true;
  if (!diskRead(getBlockOffset(blockId), blockBuffer.block, BLOCK_SIZE))
    return false;

  blockBuffer.blockId = blockId;
  blockBuffer.ready = true;

  return true;
}
bool Log::bufferBlockFromMem(uint32_t blockId,
                             const void *src,
                             off_t offset,
                             size_t size) {
  if (!isValidBlockId(blockId)) return false;

  memcpy(blockBuffer.block + offset, src, size);

  blockBuffer.blockId = blockId;
  blockBuffer.ready = true;
  blockBuffer.dirty = true;

  return true;
}
bool Log::bufferBlockToMem(uint32_t blockId,
                           void *dest,
                           off_t offset,
                           size_t size) {
  if (!isValidBlockId(blockId)) return false;

  if (!bufferBlock(blockId)) return false;
  memcpy(dest, blockBuffer.block + offset, sizeof(Metadata));
  return true;
}
bool Log::bufferBlockWriteBack() {
  if (!blockBuffer.ready) return false;

  uint32_t blockId = blockBuffer.blockId;
  if (!diskWrite(getBlockOffset(blockId), blockBuffer.block, sizeof(Block)))
    return false;

  blockBuffer.dirty = false;
  blockBuffer.ready = true;

  return true;
}

bool Log::readMetadata() {
  return bufferBlockToMem(0, &metadata, 0, sizeof(Metadata));
}
bool Log::writeMetadata() {
  if (!bufferBlockFromMem(0, &metadata, 0, sizeof(Metadata))) return false;

  if (!bufferBlockWriteBack()) return false;

  return true;
}

bool Log::moveToNextBlock() {
  currentHead ++;
  currentEntry = 0;

  if (outOfSpace()) return true;

  BlockHeader header;
  if (!readBlockHeader(currentHead, &header)) return false;

  // Check generation. Update the header if generation is out of date.
  if (metadata.generation > header.generation) {
    header.generation = metadata.generation;
    header.entryCount = 0;
    if (!writeBlockHeader(currentHead, &header)) return false;
  }

  return true;
}

bool Log::moveToNextEntry() {
  currentEntry ++;
  if (!isValidEntryId(currentEntry)) {
    return moveToNextBlock();
  }
  return true;
}

bool Log::readBlockHeader(uint32_t blockId, BlockHeader *header) {
  return bufferBlockToMem(blockId, header, 0, sizeof(BlockHeader));
}

bool Log::writeBlockHeader(uint32_t blockId, const BlockHeader *header) {
  if (!bufferBlockFromMem(blockId, header, 0, sizeof(BlockHeader)))
    return false;

  return bufferBlockWriteBack();
}

bool Log::readBlockEntry(const BlockHeader& header,
                         uint32_t blockId,
                         uint32_t entryId,
                         Entry *entry) {
  // Make sure entryId is valid.
  if (entryId >= header.entryCount) return false;

  return bufferBlockToMem(
    blockId, entry, getEntryOffset(entryId), sizeof(Entry));
}

bool Log::writeBlockEntry(uint32_t blockId,
                          uint32_t entryId,
                          const Entry *entry) {
  // Make sure the block is not full.
  if (!isValidEntryId(entryId)) return false;

  if (!bufferBlockFromMem(
    blockId, entry, getEntryOffset(entryId), sizeof(Entry))) return false;

  return bufferBlockWriteBack();
}

bool Log::reset() {
  if (!diskOpen()) return false;

  if (!resetMetadata()) return false;

  return true;
}

bool Log::resetMetadata() {
  metadata.generation ++;

  return writeMetadata();
}

bool Log::readCheckpoint(void *buf) {
  return diskRead(getCheckpointOffset(), buf, metadata.checkpointSize);
}

bool Log::writeCheckpoint(const void *data, size_t size) {
  metadata.checkpointSize = size;
  if (!writeMetadata()) return false;

  // Write checkpoint data.
  if (!diskWrite(getCheckpointOffset(), data, size)) return false;

  // Reset the entire log (aka garbage collect).
  if (!reset()) return false;

  return true;
}

bool Log::erase() {
  metadata.generation = 0;
  metadata.checkpointSize = 0;
  if (!writeMetadata()) return false;

  currentHead = 1;
  currentEntry = 0;

  return true;
}