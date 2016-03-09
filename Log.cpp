#include "Log.h"

const char *Log::DEV_FILE;

Log::Metadata Log::metadata;
Log::BlockBuffer Log::blockBuffer;

bool Log::verbose = false;

uint32_t Log::currentHead = 1;
uint32_t Log::currentEntry = 0;

int Log::diskFd = -1;
int Log::lastError = 0;

// This is for OS X.
#ifndef aligned_alloc
  void *aligned_alloc(int align, size_t size) {
    return malloc(size);
  }
#endif

bool Log::init(const char *devFile) {
  DEV_FILE = devFile;

  blockBuffer.block = aligned_alloc(0x1000, BLOCK_SIZE);

  return readMetadata();
}
void Log::moveToStart() {
  currentHead = 1;
  currentEntry = 0;
}
bool Log::playback(Entry *entry) {
  if (outOfSpace()) return false;
  if (metadata.generation == 0) return false;

  if (currentHead == 0) {
    if (!moveToNextBlock()) return false; // Error instead?
  }

  BlockHeader header;
  if (!readBlockHeader(currentHead, &header)) return false; // Error instead?

  if (!readBlockEntry(header, currentHead, currentEntry, entry)) {
    // If reached end of block, move to next block.
    if (!isValidEntryId(currentEntry)) {
      if (!moveToNextBlock()) return false; // Error instead?
      return playback(entry);
    }
    return false; // Error instead?
  }

  return moveToNextEntry();
}
bool Log::add(uint32_t opCode, uint64_t id1, uint64_t id2) {
  if (outOfSpace()) return false;

  // If our current generation is 0, we need to initialize it to 1.
  if (metadata.generation == 0) {
    metadata.generation = 1;
    if (!writeMetadata()) return false;
  }

  Entry entry = { opCode, id1, id2 };

  // Write entry to current space.
  if (!writeBlockEntry(currentHead, currentEntry, entry)) return false;

  // Update the block header.
  BlockHeader header = { metadata.generation, currentEntry + 1 };
  if (!writeBlockHeader(currentHead, header)) return false;

  return moveToNextEntry();
}
bool Log::finish() {
  if (!isOpen()) return true;

  bufferBlockWriteBack();

  return diskClose();
}

bool Log::readMetadata() {
  return bufferBlockToMem(0, &metadata, sizeof(Metadata));
}
bool Log::writeMetadata() {
  if (!bufferBlockFromMem(0, &metadata, sizeof(Metadata))) return false;

  return bufferBlockWriteBack();
}

bool Log::moveToNextBlock() {
  currentHead ++;
  currentEntry = 0;

  if (outOfSpace()) return true;

  BlockHeader header;
  if (!readBlockHeader(currentHead, &header)) return false;

  // Check generation. Update the header if generation is out of date.
  if (metadata.generation != header.generation) {
    header.generation = metadata.generation;
    header.entryCount = 0;
    if (!writeBlockHeader(currentHead, header)) return false;
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
  return bufferBlockReadHeader(blockId, header);
}

bool Log::writeBlockHeader(uint32_t blockId, const BlockHeader& header) {
  if (!bufferBlockWriteHeader(blockId, header)) return false;

  return bufferBlockWriteBack();
}

bool Log::readBlockEntry(const BlockHeader& header,
                         uint32_t blockId,
                         uint32_t entryId,
                         Entry *entry) {
  // Make sure entryId is valid.
  if (entryId >= header.entryCount) return false;

  return bufferBlockReadEntry(blockId, entryId, entry);
}

bool Log::writeBlockEntry(uint32_t blockId,
                          uint32_t entryId,
                          const Entry& entry) {
  if (!bufferBlockWriteEntry(blockId, entryId, entry)) return false;

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
  Metadata metadataReset;
  metadata = metadataReset;
  if (!writeMetadata()) return false;

  currentHead = 1;
  currentEntry = 0;

  return true;
}