#include <sys/types.h>
#include <unistd.h>

#define SM_FILE_BASE 0x3e000000UL

#define SM_FILE_CNTL (SM_FILE_BASE + 0x00)
#define SM_FILE_OP (SM_FILE_BASE + 0x08)
#define SM_FILE_OUT (SM_FILE_BASE + 0x10)
#define SM_FILE_IN1 (SM_FILE_BASE + 0x18)
#define SM_FILE_IN2 (SM_FILE_BASE + 0x20)
#define SM_FILE_IN3 (SM_FILE_BASE + 0x28)

typedef enum {
    SM_FILE_OP_OPEN = 0,
    SM_FILE_OP_CLOSE = 1,
    SM_FILE_OP_LSEEK = 2,
    SM_FILE_OP_READ = 3,
    SM_FILE_OP_WRITE = 4,
} SM_file_op_t;

int open(const char *pathname, int flags) {
    *(volatile unsigned long *)SM_FILE_OP = SM_FILE_OP_OPEN;
    *(volatile unsigned long *)SM_FILE_IN1 = (unsigned long)pathname;
    *(volatile unsigned long *)SM_FILE_IN2 = (unsigned)flags;
    *(volatile unsigned long *)SM_FILE_CNTL = 1; // Trigger the operation
    return (int)(*(volatile unsigned long *)SM_FILE_OUT);
}

int close(int fd) {
    *(volatile unsigned long *)SM_FILE_OP = SM_FILE_OP_CLOSE;
    *(volatile unsigned long *)SM_FILE_IN1 = (unsigned long)fd;
    *(volatile unsigned long *)SM_FILE_CNTL = 1; // Trigger the operation
    return (int)(*(volatile unsigned long *)SM_FILE_OUT);
}

off_t lseek(int fd, off_t offset, int whence) {
    *(volatile unsigned long *)SM_FILE_OP = SM_FILE_OP_LSEEK;
    *(volatile unsigned long *)SM_FILE_IN1 = (unsigned long)fd;
    *(volatile unsigned long *)SM_FILE_IN2 = (unsigned long)offset;
    *(volatile unsigned long *)SM_FILE_IN3 = (unsigned long)whence;
    *(volatile unsigned long *)SM_FILE_CNTL = 1; // Trigger the operation
    return (off_t)(*(volatile unsigned long *)SM_FILE_OUT);
}

ssize_t read(int fd, void *buf, size_t count) {
    *(volatile unsigned long *)SM_FILE_OP = SM_FILE_OP_READ;
    *(volatile unsigned long *)SM_FILE_IN1 = (unsigned long)fd;
    *(volatile unsigned long *)SM_FILE_IN2 = (unsigned long)buf;
    *(volatile unsigned long *)SM_FILE_IN3 = (unsigned long)count;
    *(volatile unsigned long *)SM_FILE_CNTL = 1; // Trigger the operation
    return (ssize_t)(*(volatile unsigned long *)SM_FILE_OUT);
}

ssize_t write(int fd, const void *buf, size_t count) {
    *(volatile unsigned long *)SM_FILE_OP = SM_FILE_OP_WRITE;
    *(volatile unsigned long *)SM_FILE_IN1 = (unsigned long)fd;
    *(volatile unsigned long *)SM_FILE_IN2 = (unsigned long)buf;
    *(volatile unsigned long *)SM_FILE_IN3 = (unsigned long)count;
    *(volatile unsigned long *)SM_FILE_CNTL = 1; // Trigger the operation
    return (ssize_t)(*(volatile unsigned long *)SM_FILE_OUT);
}