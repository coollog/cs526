bool Log::diskOpen() {
  if (isOpen()) return true;

  diskFd = open("/dev/sdc", 0);

  return isOpen();
}
bool Log::isOpen() {
  return diskFd != -1;
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

  if (!diskSeek(0)) return false;

  size_t metadataSize = sizeof(metadata);
  ssize_t size = read(fd, &metadata, metadataSize);
  if (size != metadataSize) return false;

  return true;
}

bool Log::reset() {
  invariant(isOpen());

  if (!diskSeek(0)) return false;

  metadata.start = 1;
  metadata.size = 0;
  metadata.head = 1;

  size_t metadataSize = sizeof(metadata);
  ssize_t size = write(diskFd, &metadata, metadataSize);
  if (size != metadataSize) return false;

  return true;
}