#ifndef invariant
void invariant(bool truth) {
  if (!truth) throw 1337;
}
#endif