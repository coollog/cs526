#include "Log.h"

// This is for OS X.
#ifndef O_DIRECT
  static void *aligned_alloc(int align, size_t size) {
    return malloc(size);
  }
#endif

bool Log::Disk::diskOpen() {
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
bool Log::Disk::isOpen() {
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
bool Log::Disk::diskClose() {
  diskFd = -1;
  return close(diskFd) != -1;
}

bool Log::Disk::diskSeek(off_t offset) {
  off_t off = lseek(diskFd, offset, SEEK_SET);

  if (off == -1) {
    setErrno(errno);
    return false;
  }
  return true;
}

bool Log::Disk::diskRead(off_t offset, void *buf, size_t size) {
  if (!diskOpen()) return false;
  if (!diskSeek(offset)) return false;

  void *bufAligned;
  size_t sizeAligned = roundToPageSize(size);
  if (reinterpret_cast<uintptr_t>(buf) & 0xfff) {
    bufAligned = aligned_alloc(0x1000, sizeAligned);
  } else {
    bufAligned = buf;
  }

  ssize_t readSize = read(diskFd, bufAligned, sizeAligned);
  if (readSize != (ssize_t)sizeAligned) {
    setErrno(errno);
    if (buf != bufAligned) free(bufAligned);
    return false;
  }

  if (buf != bufAligned) {
    memcpy(buf, bufAligned, size);
    free(bufAligned);
  }

  return true;
}

bool Log::Disk::diskWrite(off_t offset, const void *data, size_t size) {
  if (!diskOpen()) return false;
  if (!diskSeek(offset)) return false;

  const void *dataAligned;
  size_t sizeAligned = roundToPageSize(size);
  if (reinterpret_cast<uintptr_t>(data) & 0xfff) {
    dataAligned = aligned_alloc(0x1000, sizeAligned);
  } else {
    dataAligned = data;
  }

  ssize_t writeSize = write(diskFd, dataAligned, sizeAligned);
  if (writeSize != (ssize_t)sizeAligned) {
    setErrno(errno);
    if (data != dataAligned) free(dataAligned);
    return false;
  }

  if (data != dataAligned) free(dataAligned);

  return true;
}
