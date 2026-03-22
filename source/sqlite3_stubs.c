// SQLite3 benötigt fchown und geteuid für sein Unix-VFS.
// FAT32 (SDMC) unterstützt keine POSIX-Ownership, daher leere Stubs.
#include <sys/types.h>

int fchown(int fd, uid_t owner, gid_t group) {
    (void)fd; (void)owner; (void)group;
    return 0;
}

uid_t geteuid(void) {
    return 0;
}
