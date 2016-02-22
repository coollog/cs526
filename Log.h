#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

class Log {
  const size_t BLOCK_SIZE = 0x1000;

  // Holds the data in the superblock.
  typedef struct {
    unsigned int start;
    unsigned int size;
    unsigned int head;
  } Metadata;

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
  bool init();
  bool readMetadata();

private:
  Metadata metadata;
  int diskFd;

  bool diskOpen();
  bool isOpen();
  bool diskClose();
  bool diskSeek(off_t offset);

  void readBlock(unsigned int blockId);

  bool reset();
};