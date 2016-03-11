#include <cstdint>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

class Log {
  static constexpr size_t BLOCK_SIZE = 0x1000;
  static constexpr char EMPTY_BLOCK[BLOCK_SIZE] = {0};
  static const bool VERBOSE = false;

  // Holds the header of a block.
  typedef struct {
    uint32_t generation;
    uint32_t entryCount;
  } __attribute__((packed)) BlockHeader;
  // Total number of entries allowed. Entries start at id 0.
  static const uint32_t MAX_ENTRY_COUNT = 204;

public:
  // Holds the data in the superblock.
  typedef struct {
    uint32_t generation = 0;
    uint32_t logSize = 100;
    uint32_t checkpointSize = 0;
  } __attribute__((packed)) Metadata;

  // Holds each entry in a block.
  typedef struct {
    uint32_t opCode;
    uint64_t id1;
    uint64_t id2;
  } __attribute__((packed)) Entry;

  typedef struct {
    BlockHeader header;
    Entry entries[MAX_ENTRY_COUNT];
    // Pad Block to BLOCK_SIZE;
    char padding[
      BLOCK_SIZE - sizeof(BlockHeader) - MAX_ENTRY_COUNT * sizeof(Entry)];
  } __attribute__((packed)) Block;

  // Opens the disk, reads the metadata, and sets of internal states of the log.
  static bool init(const char *devFile);
  static uint32_t getCheckpointSize() { return metadata.checkpointSize; };
  // Read in the checkpoint to a buffer.
  static bool readCheckpoint(void *buf);
  // Write the graph to the checkpoint and reset the log.
  static bool writeCheckpoint(const void *data, size_t size);
  // Move back to the start of the log.
  static void moveToStart();
  // Play back the log for the current generation (from the head).
  // This plays back one entry of the log, and returns true if there are more
  // entries to play back.
  // Make sure the checkpoint is loaded in first.
  static bool playback(Entry *entry);
  // Adds an entry to the log.
  static bool add(uint32_t opCode, uint64_t id1, uint64_t id2);
  static bool outOfSpace() { return currentHead >= metadata.logSize; }
  // Unlinks the entire log/checkpoint.
  static bool erase();
  static bool finish();

  static int getErrno() { return lastError; }
  static const Metadata& getMetadata() { return metadata; }

private:
  static const char *DEV_FILE;

  static void setErrno(int en) { lastError = en; }

  // Garbage-collects the log, and resets it.
  static bool reset();

  // Helper functions for reset().
  static bool resetMetadata();

  class Disk {
  public:
    static bool diskClose();

    // Seeks 'offset' and reads 'size' bytes of disk to 'buf'.
    static bool diskRead(off_t offset, void *buf, size_t size);

    // Seeks 'offset' and writes 'size' bytes of 'data' to disk.
    static bool diskWrite(off_t offset, const void *data, size_t size);
  private:
    static bool isOpen(); // Is the disk open?
    static bool diskOpen(); // Open the disk.

    static bool diskSeek(off_t offset); // Seek to position offset.
  };

  // Helpers for the block/entry methods.
  static off_t getBlockOffset(uint32_t blockId) { return blockId * BLOCK_SIZE; }
  static off_t getEntryOffset(uint32_t entryId)
    { return sizeof(BlockHeader) + entryId * sizeof(Entry); }
  static bool isValidBlockId(uint32_t blockId)
    { return blockId < metadata.logSize; }
  static bool isValidEntryId(uint32_t entryId)
    { return entryId < MAX_ENTRY_COUNT; }

  class BlockBuffer {
  public:
    static void init();

    static bool writeBack();
    static bool readEntry(uint32_t blockId, uint32_t entryId, Entry *entry);
    static bool writeEntry(uint32_t blockId,
                           uint32_t entryId,
                           const Entry& entry);
    static bool readHeader(uint32_t blockId, BlockHeader *header);
    static bool writeHeader(uint32_t blockId, const BlockHeader& header);
    // Copy 'size' bytes memory from 'src' to the buffer.
    static bool fromMem(uint32_t blockId, const void *src, size_t size);
    // Copy 'size' bytes memory from buffer to 'dest'.
    // Reads block from disk if not in buffer.
    static bool toMem(uint32_t blockId, void *dest, size_t size);
  private:
    static Block *asBlock() { return (Block *)block; }

    static bool prepareBlock(uint32_t blockId);

    static bool ready;
    static bool dirty;
    static uint32_t id;
    static void *block;
  };

  static bool readMetadata();
  static bool writeMetadata();
  static bool moveToNextBlock();
  static bool moveToNextEntry();
  static bool readBlockHeader(uint32_t blockId, BlockHeader *header);
  static bool writeBlockHeader(uint32_t blockId, const BlockHeader& header);
  static bool readBlockEntry(const BlockHeader& header,
                             uint32_t blockId,
                             uint32_t entryId,
                             Entry *entry);
  // Writes to the next empty entry in blockId.
  static bool writeBlockEntry(uint32_t blockId,
                              uint32_t entryId,
                              const Entry& entry);


  static off_t getCheckpointOffset() { return metadata.logSize * BLOCK_SIZE; }

  static alignas(0x1000) Metadata metadata;

  // The current block.
  static uint32_t currentHead;
  // The current entry in the current block.
  static uint32_t currentEntry;

  static int diskFd;
  static int lastError;
};