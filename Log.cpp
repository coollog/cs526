#include "Log.h"

const char *Log::DEV_FILE;

Log::Metadata Log::metadata;

uint32_t Log::currentHead = 1;
uint32_t Log::currentEntry = 0;

int Log::diskFd = -1;
int Log::lastError = 0;

bool Log::init(const char *devFile) {
  DEV_FILE = devFile;

  BlockBuffer::init();

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
      if (VERBOSE) printf("\treached end of block\n");
      if (!moveToNextBlock()) return false; // Error instead?
      if (VERBOSE) printf("\tcurrent block: %u\n", currentHead);
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
  BlockBuffer::writeBack();

  return Disk::diskClose();
}

bool Log::readMetadata() {
  return BlockBuffer::toMem(0, &metadata, sizeof(Metadata));
}
bool Log::writeMetadata() {
  if (!BlockBuffer::fromMem(0, &metadata, sizeof(Metadata))) return false;

  return BlockBuffer::writeBack();
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

  if (VERBOSE) printf(
    "\tmove to next block: block %u, entry %u\n", currentHead, currentEntry);

  return true;
}

bool Log::moveToNextEntry() {
  currentEntry ++;
  if (!isValidEntryId(currentEntry)) {
    return moveToNextBlock();
  }
  if (VERBOSE) printf(
    "\tmove to next entry: block %u, entry %u\n", currentHead, currentEntry);
  return true;
}

bool Log::readBlockHeader(uint32_t blockId, BlockHeader *header) {
  return BlockBuffer::readHeader(blockId, header);
}
bool Log::writeBlockHeader(uint32_t blockId, const BlockHeader& header) {
  if (!BlockBuffer::writeHeader(blockId, header)) return false;

  return BlockBuffer::writeBack();
}
bool Log::readBlockEntry(const BlockHeader& header,
                         uint32_t blockId,
                         uint32_t entryId,
                         Entry *entry) {
  // Make sure entryId is valid.
  if (entryId >= header.entryCount) return false;

  return BlockBuffer::readEntry(blockId, entryId, entry);
}
bool Log::writeBlockEntry(uint32_t blockId,
                          uint32_t entryId,
                          const Entry& entry) {
  if (!BlockBuffer::writeEntry(blockId, entryId, entry)) return false;

  return BlockBuffer::writeBack();
}

bool Log::reset() {
  if (!resetMetadata()) return false;

  return true;
}

bool Log::resetMetadata() {
  metadata.generation ++;

  return writeMetadata();
}

bool Log::readCheckpoint(void *buf) {
  return Disk::diskRead(getCheckpointOffset(), buf, metadata.checkpointSize);
}

bool Log::writeCheckpoint(const void *data, size_t size) {
  metadata.checkpointSize = size;
  if (!writeMetadata()) return false;

  // Write checkpoint data.
  if (!Disk::diskWrite(getCheckpointOffset(), data, size)) return false;

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