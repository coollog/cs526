#include <stdio.h>
#include "utilities.h"
#include "Log.h"

bool Log::init() {
  return diskOpen();
}
bool Log::finish() {
  if (!isOpen()) return true;
  return diskClose();
}

bool Log::diskOpen() {
  if (isOpen()) return true;

  diskFd = open("/dev/sdc", O_RDWR | O_SYNC);
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

  if (off != offset) {
    setErrno(errno);
    return false;
  }
  return true;
}

bool Log::readMetadata() {
  if (!diskOpen()) return false;

  if (!diskSeek(0)) return false;

  size_t metadataSize = sizeof(metadata);
  ssize_t size = read(diskFd, &metadata, metadataSize);
  if (size != (ssize_t)metadataSize) {
    setErrno(-1);
    return false;
  }

  return true;
}

bool Log::reset(unsigned int size) {
  return reset(size, 0);
}

bool Log::reset(unsigned int size, int flags) {
  if (!diskOpen()) return false;

  if (!resetMetadata(size)) return false;

  for (unsigned int id = 1; id <= size; id ++) {
    if (flags | RESET_VERBOSE) {
      printf("resetting block %u\n", id);
    }
    if (!resetBlock(id)) return false;
  }

  return true;
}

bool Log::resetMetadata(unsigned int size) {
  if (!diskOpen()) return false;

  if (!diskSeek(0)) return false;

  metadata.start = 1;
  metadata.size = size;
  metadata.head = 1;

  size_t metadataSize = sizeof(metadata);
  ssize_t writeSize = write(diskFd, &metadata, metadataSize);
  if (writeSize != (ssize_t)metadataSize) {
    setErrno(errno);
    return false;
  }

  return true;
}

bool Log::resetBlock(unsigned int id) {
  if (!diskOpen()) return false;

  off_t offset = id * BLOCK_SIZE;
  if (!diskSeek(offset)) return false;

  ssize_t writeSize = write(diskFd, EMPTY_BLOCK, BLOCK_SIZE);
  if (writeSize != BLOCK_SIZE) {
    setErrno(errno);
    return false;
  }

  return true;
}