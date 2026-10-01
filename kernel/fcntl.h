// Fast System File Control
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#ifndef _FCNTL_H
#define _FCNTL_H

#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_ACCMODE       0x0003

#define O_CREAT         0x0040
#define O_EXCL          0x0080
#define O_TRUNC         0x0200
#define O_APPEND        0x0400
#define O_NONBLOCK      0x0800
#define O_DIRECTORY     0x10000
#define O_CLOEXEC       0x80000

#define O_NOFOLLOW      0x20000
#define O_SYNC          0x101000
#define O_DSYNC         0x1000
#define O_RSYNC         O_SYNC

#define O_EXEC          0x400000
#define O_SEARCH        0x400000

#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

#define AT_FDCWD        (-100)

#endif // _FCNTL_H
