#include <stdio.h>
#include "utilities.h"
#include "Log.h"

const char *Log::DEV_FILE;

bool Log::init(const char *fname) {
  DEV_FILE = fname;

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

  diskFd = open(DEV_FILE, O_RDWR | O_SYNC);
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
bool Log::diskSeekCommon(off_t offset, int whence) {
  off_t off = lseek(diskFd, offset, whence);

  if (off == -1) {
    setErrno(errno);
    return false;
  }
  return true;
}
bool Log::diskSeekMore(off_t offset) {
  return diskSeekCommon(offset, SEEK_CUR);
}
bool Log::diskSeek(off_t offset) {
  return diskSeekCommon(offset, SEEK_SET);
}

bool Log::diskReadCommon(off_t offset, void *buf, size_t size, int whence) {
  if (!diskOpen()) return false;
  if (!diskSeek(offset)) return false;

  ssize_t readSize = read(diskFd, buf, size);
  if (readSize != (ssize_t)size) {
    setErrno(errno);
    return false;
  }

  return true;
}
bool Log::diskRead(off_t offset, void *buf, size_t size) {
  return diskReadCommon(offset, buf, size, SEEK_SET);
}
bool Log::diskReadMore(off_t offset, void *buf, size_t size) {
  return diskReadCommon(offset, buf, size, SEEK_CUR);
}

bool Log::diskWriteCommon(off_t offset,
                          const void *data,
                          size_t size,
                          int whence) {
  if (!diskOpen()) return false;
  if (!diskSeekCommon(offset, whence)) return false;

  ssize_t writeSize = write(diskFd, data, size);
  if (writeSize != (ssize_t)size) {
    setErrno(errno);
    return false;
  }
  return true;
}
bool Log::diskWrite(off_t offset, const void *data, size_t size) {
  return diskWriteCommon(offset, data, size, SEEK_SET);
}
bool Log::diskWriteMore(off_t offset, const void *data, size_t size) {
  return diskWriteCommon(offset, data, size, SEEK_CUR);
}

bool Log::readMetadata() {
  return diskRead(0, &metadata, sizeof(Metadata));
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
  if (!isValidBlockId(blockId)) return false;

  return diskRead(getBlockOffset(blockId), header, sizeof(BlockHeader));
}

bool Log::writeBlockHeader(uint32_t blockId, const BlockHeader *header) {
  if (!isValidBlockId(blockId)) return false;

  return diskWrite(getBlockOffset(blockId), header, sizeof(BlockHeader));
}

bool Log::readBlockEntry(const BlockHeader& header,
                         uint32_t blockId,
                         uint32_t entryId,
                         Entry *entry) {
  off_t offset = getBlockOffset(blockId);
  off_t entryOffset = getEntryOffset(entryId);

  // Make sure entryId is valid.
  if (entryId >= header.entryCount) return false;

  return diskRead(offset + entryOffset, entry, sizeof(Entry));
}

bool Log::writeBlockEntry(uint32_t blockId,
                          uint32_t entryId,
                          const Entry *entry) {
  // Make sure the block is not full.
  if (!isValidEntryId(entryId)) return false;

  off_t offset = getBlockOffset(blockId);
  off_t entryOffset = getEntryOffset(entryId);

  return diskWrite(offset + entryOffset, entry, sizeof(Entry));
}

bool Log::reset(uint32_t size) {
  if (!diskOpen()) return false;

  if (!resetMetadata(size)) return false;

  return true;
}

bool Log::resetMetadata(uint32_t size) {
  metadata.size = size;
  metadata.generation ++;

  if (!diskWrite(0, &metadata, sizeof(Metadata))) return false;

  return true;
}

bool Log::readCheckpointHeader(CheckpointHeader *header) {
  return diskRead(getCheckpointOffset(), header, sizeof(CheckpointHeader));
}

bool Log::writeCheckpointHeader(const CheckpointHeader *header) {
  return diskWrite(getCheckpointOffset(), header, sizeof(CheckpointHeader));
}

bool Log::readCheckpoint(void *buf) {
  CheckpointHeader header;

  if (!readCheckpointHeader(&header)) return false;

  size_t size = header.size;
  if (!diskReadMore(sizeof(CheckpointHeader), buf, size)) return false;

  return true;
}

bool Log::writeCheckpoint(const void *data, size_t size) {
  if (!diskOpen()) return false;

  uint32_t blockCount = metadata.size;

  CheckpointHeader header = { size };
  if (!writeCheckpointHeader(&header)) return false;

  // Write checkpoint data.
  if (!diskWriteMore(sizeof(CheckpointHeader), data, size)) return false;

  // Reset the entire log (aka garbage collect).
  if (!reset(blockCount)) return false;

  return true;
}

void Log::erase(uint32_t size) {
  CheckpointHeader header = { 0 };
  writeCheckpointHeader(&header);

  metadata.size = size;
  metadata.generation = 0;
  diskWrite(0, &metadata, sizeof(Metadata));

  currentHead = 1;
  currentEntry = 0;
}