#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

class Log {
  static constexpr size_t BLOCK_SIZE = 0x1000;
  const char EMPTY_BLOCK[BLOCK_SIZE] = {0};

  // Holds the header entry in a block.
  typedef struct {
    unsigned int opCode;
    unsigned int id1;
    unsigned int id2;
  } EntryHeader;

  // Holds each entry in a block.
  typedef struct {
    char byte[12];
  } Entry;

  // Holds the header of a block.
  typedef struct {
    unsigned int generation;
    unsigned int entryCount;
    EntryHeader entryHeader;
  } BlockHeader;

public:
  enum {
    RESET_VERBOSE
  };

  // Holds the data in the superblock.
  typedef struct {
    unsigned int start;
    unsigned int size;
    unsigned int head;
  } Metadata;

  bool init();
  bool reset(unsigned int size);
  bool reset(unsigned int size, int flags);
  bool readMetadata();
  bool finish();

  int getErrno() { return lastError; }
  const Metadata& getMetadata() { return metadata; }

private:
  Metadata metadata;
  int diskFd = -1;
  int lastError = 0;

  void setErrno(int en) { lastError = en; }

  bool diskOpen();
  bool isOpen();
  bool diskClose();
  bool diskSeek(off_t offset);

  void readBlock(unsigned int blockId);

  // Helper functions for reset().
  bool resetMetadata(unsigned int size);
  bool resetBlock(unsigned int id);
};