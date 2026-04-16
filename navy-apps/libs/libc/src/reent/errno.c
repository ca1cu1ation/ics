/* Single errno definition for reentrant syscall wrappers.
   Older toolchains tolerated multiple tentative definitions, but modern
   linkers with -fno-common treat them as duplicate symbols.  */

#undef errno
int errno;