#include "Log.h"

bool Log::diskOpen() {
  if (isOpen()) return true;

#ifdef O_DIRECT
  diskFd = open(DEV_FILE, O_RDWR | O_SYNC | O_DIRECT);
#else
  diskFd = open(DEV_FILE, O_RDWR | O_SYNC);
  fcntl(diskFd, F_NOCACHE, 1);
#endif

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

  if (off == -1) {
    setErrno(errno);
    return false;
  }
  return true;
}

bool Log::diskRead(off_t offset, void *buf, size_t size) {
  if (!diskOpen()) return false;
  if (!diskSeek(offset)) return false;

  ssize_t readSize = read(diskFd, buf, size);
  if (readSize != (ssize_t)size) {
    setErrno(errno);
    return false;
  }

  return true;
}

bool Log::diskWrite(off_t offset, const void *data, size_t size) {
  if (!diskOpen()) return false;
  if (!diskSeek(offset)) return false;

  ssize_t writeSize = write(diskFd , data, size);
  if (writeSize != (ssize_t)size) {
    setErrno(errno);
    return false;
  }
  return true;
}