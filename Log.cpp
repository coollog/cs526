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

bool Log::reset() {
  if (!diskOpen()) return false;

  if (!diskSeek(0)) return false;

  metadata.start = 1;
  metadata.size = 0;
  metadata.head = 1;

  size_t metadataSize = sizeof(metadata);
  ssize_t size = write(diskFd, &metadata, metadataSize);
  if (size != (ssize_t)metadataSize) {
    setErrno(-1);
    return false;
  }

  return true;
}