#include <sys/types.h>
#include <unistd.h>

#define MDL_CP_FILE_BASE 0x3f000

#define MDL_CP_FILE_CNTL (MDL_CP_FILE_BASE + 0x00)
#define MDL_CP_FILE_OP (MDL_CP_FILE_BASE + 0x04)
#define MDL_CP_FILE_OUT (MDL_CP_FILE_BASE + 0x08)
#define MDL_CP_FILE_IN1 (MDL_CP_FILE_BASE + 0x10)
#define MDL_CP_FILE_IN2 (MDL_CP_FILE_BASE + 0x14)
#define MDL_CP_FILE_IN3 (MDL_CP_FILE_BASE + 0x18)

typedef enum {
    MDL_CP_FILE_OP_OPEN = 0,
    MDL_CP_FILE_OP_CLOSE = 1,
    MDL_CP_FILE_OP_LSEEK = 2,
    MDL_CP_FILE_OP_READ = 3,
    MDL_CP_FILE_OP_WRITE = 4,
} mdl_cp_file_op_t;

int open(const char *pathname, int flags) {
    *(volatile unsigned long *)MDL_CP_FILE_OP = MDL_CP_FILE_OP_OPEN;
    *(volatile unsigned long *)MDL_CP_FILE_IN1 = (unsigned long)pathname;
    *(volatile unsigned long *)MDL_CP_FILE_IN2 = (unsigned)flags;
    *(volatile unsigned long *)MDL_CP_FILE_CNTL = 1; // Trigger the operation
    return (int)(*(volatile unsigned long *)MDL_CP_FILE_OUT);
}

int close(int fd) {
    *(volatile unsigned long *)MDL_CP_FILE_OP = MDL_CP_FILE_OP_CLOSE;
    *(volatile unsigned long *)MDL_CP_FILE_IN1 = (unsigned long)fd;
    *(volatile unsigned long *)MDL_CP_FILE_CNTL = 1; // Trigger the operation
    return (int)(*(volatile unsigned long *)MDL_CP_FILE_OUT);
}

off_t lseek(int fd, off_t offset, int whence) {
    *(volatile unsigned long *)MDL_CP_FILE_OP = MDL_CP_FILE_OP_LSEEK;
    *(volatile unsigned long *)MDL_CP_FILE_IN1 = (unsigned long)fd;
    *(volatile unsigned long *)MDL_CP_FILE_IN2 = (unsigned long)offset;
    *(volatile unsigned long *)MDL_CP_FILE_IN3 = (unsigned long)whence;
    *(volatile unsigned long *)MDL_CP_FILE_CNTL = 1; // Trigger the operation
    return (off_t)(*(volatile unsigned long *)MDL_CP_FILE_OUT);
}

ssize_t read(int fd, void *buf, size_t count) {
    *(volatile unsigned long *)MDL_CP_FILE_OP = MDL_CP_FILE_OP_READ;
    *(volatile unsigned long *)MDL_CP_FILE_IN1 = (unsigned long)fd;
    *(volatile unsigned long *)MDL_CP_FILE_IN2 = (unsigned long)buf;
    *(volatile unsigned long *)MDL_CP_FILE_IN3 = (unsigned long)count;
    *(volatile unsigned long *)MDL_CP_FILE_CNTL = 1; // Trigger the operation
    return (ssize_t)(*(volatile unsigned long *)MDL_CP_FILE_OUT);
}

ssize_t write(int fd, const void *buf, size_t count) {
    *(volatile unsigned long *)MDL_CP_FILE_OP = MDL_CP_FILE_OP_WRITE;
    *(volatile unsigned long *)MDL_CP_FILE_IN1 = (unsigned long)fd;
    *(volatile unsigned long *)MDL_CP_FILE_IN2 = (unsigned long)buf;
    *(volatile unsigned long *)MDL_CP_FILE_IN3 = (unsigned long)count;
    *(volatile unsigned long *)MDL_CP_FILE_CNTL = 1; // Trigger the operation
    return (ssize_t)(*(volatile unsigned long *)MDL_CP_FILE_OUT);
}