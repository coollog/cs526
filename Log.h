#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

class Log {
  static constexpr size_t BLOCK_SIZE = 0x1000;
  const char EMPTY_BLOCK[BLOCK_SIZE] = {0};

  // Holds each entry in a block.
  typedef struct {
    uint32_t opCode;
    uint64_t id1;
    uint64_t id2;
  } Entry;

  // Holds the header of a block.
  typedef struct {
    uint32_t generation;
    uint32_t entryCount;
  } BlockHeader;
  // Total number of entries allowed. Entries start at id 0.
  const uint32_t MAX_ENTRY_COUNT = 204;

  // Header for the checkpoint.
  typedef struct {
    size_t size; // How many bytes the graph takes up.
  } CheckpointHeader;

public:
  static void setDevFile(const char *fname);

  // Holds the data in the superblock.
  typedef struct {
    uint32_t size;
    uint32_t generation;
  } Metadata;

  // Opens the disk, reads the metadata, and sets of internal states of the log.
  bool init();
  // Read in the checkpoint to a buffer.
  bool readCheckpoint(void *buf);
  // Write the graph to the checkpoint and reset the log.
  bool writeCheckpoint(const void *data, size_t size);
  // Play back the log for the current generation (from the head).
  // This plays back one entry of the log, and returns true if there are more
  // entries to play back.
  // Make sure the checkpoint is loaded in first.
  bool playback(Entry *entry);
  // Adds an entry to the log.
  bool add(uint32_t opCode, uint64_t id1, uint64_t id2);
  bool outOfSpace() { return currentHead >= metadata.size; }
  // Unlinks the entire log/checkpoint.
  void erase();
  bool finish();

  int getErrno() { return lastError; }
  const Metadata& getMetadata() { return metadata; }

  bool verbose = false;

private:
  static const char *devFile;

  void setErrno(int en) { lastError = en; }

  bool diskOpen();
  bool isOpen();
  bool diskClose();

  // Garbage-collects the log, and resets it.
  bool reset(uint32_t size);

  // Helper functions for reset().
  bool resetMetadata(uint32_t size);

  // Seeking in the disk.
  bool diskSeekCommon(off_t offset, int whence);
  bool diskSeek(off_t offset); // Seek to position offset.
  bool diskSeekMore(off_t offset); // Seek to current position + offset.

  /**
   * Reading from the disk.
   * 1) Seeks 'offset' with flag 'whence'.
   * 2) Read 'size' bytes of disk to 'buf'.
   */
  bool diskReadCommon(off_t offset, void *buf, size_t size, int whence);
  bool diskRead(off_t offset, void *buf, size_t size);
  bool diskReadMore(off_t getCheckpointOffset, void *buf, size_t size);

  /**
   * Writing to the disk.
   * 1) Seeks 'offset' with flag 'whence'.
   * 2) Write 'size' bytes of 'data' to disk.
   */
  bool diskWriteCommon(off_t offset, const void *data, size_t size, int whence);
  bool diskWrite(off_t offset, const void *data, size_t size);
  bool diskWriteMore(off_t offset, const void *data, size_t size);

  // Helpers for the block/entry methods.
  off_t getBlockOffset(uint32_t blockId) { return blockId * BLOCK_SIZE; }
  off_t getEntryOffset(uint32_t entryId) {
    return sizeof(BlockHeader) + entryId * sizeof(Entry);
  }
  bool isValidBlockId(uint32_t blockId) { return blockId < metadata.size; }
  bool isValidEntryId(uint32_t entryId) { return entryId < MAX_ENTRY_COUNT; }

  bool readMetadata();
  bool moveToNextBlock();
  bool moveToNextEntry();
  bool readBlockHeader(uint32_t blockId, BlockHeader *header);
  bool writeBlockHeader(uint32_t blockId, const BlockHeader *header);
  bool readBlockEntry(
    const BlockHeader& header, uint32_t blockId, uint32_t entryId, Entry *entry);
  // Writes to the next empty entry in blockId.
  bool writeBlockEntry(uint32_t blockId, uint32_t entryId, const Entry *entry);

  off_t getCheckpointOffset() { return metadata.size * BLOCK_SIZE; }
  bool readCheckpointHeader(CheckpointHeader *header); // Caller owns pointer.
  bool writeCheckpointHeader(const CheckpointHeader *header);

  Metadata metadata;

  // The current block.
  uint32_t currentHead = 1;
  // The current entry in the current block.
  uint32_t currentEntry = 0;

  int diskFd = -1;
  int lastError = 0;
};