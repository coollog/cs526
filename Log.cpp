#include "utilities.h"
#include "Log.h"

bool Log::init() {
  return diskOpen();
}
bool Log::finish() {
  invariant(isOpen());
  return diskClose();
}
int Log::getErrno() {
  return lastError;
}
void Log::setErrno(int en) {
  lastError = en;
}

bool Log::diskOpen() {
  if (isOpen()) return true;

  diskFd = open("/dev/sdc", 0);

  return isOpen();
}
bool Log::isOpen() {
  return fcntl(diskFd, F_GETFD) != -1 || errno != EBADF;
}
bool Log::diskClose() {
  diskFd = -1;
  return close(diskFd) != -1;
}
bool Log::diskSeek(off_t offset) {
  off_t off = lseek(diskFd, offset, SEEK_SET);
  return off == offset;
}

bool Log::readMetadata() {
  invariant(isOpen());

  if (!diskSeek(0)) {
    setErrno(errno);
    return false;
  }

  size_t metadataSize = sizeof(metadata);
  ssize_t size = read(diskFd, &metadata, metadataSize);
  if (size != (ssize_t)metadataSize) {
    setErrno(-1);
    return false;
  }

  return true;
}

bool Log::reset() {
  invariant(isOpen());

  if (!diskSeek(0)) {
    setErrno(errno);
    return false;
  }

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