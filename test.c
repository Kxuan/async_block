//
// Created by xuan on 8/26/15.
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include "libab_thread.h"
#include "libab.h"

static void *thread_echo(void *arg) {
    char buff[1024];
    ssize_t retsize;

    //TODO we need a new function abio_manage() to do this
    int fl;
    fl = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, fl | O_NONBLOCK);
    fl = fcntl(STDOUT_FILENO, F_GETFL);
    fcntl(STDOUT_FILENO, F_SETFL, fl | O_NONBLOCK);

    while ((retsize = ab_read(STDIN_FILENO, buff, 1024)) > 0) {
        ab_write(STDOUT_FILENO, buff, (size_t) retsize);
        if (strcmp(buff, "exit\n") == 0)
            break;
    }
    return arg;
}

static void *thread_copy(void *arg) {
    char buff[1024];
    int fdin, fdout;
    uint32_t copy_count = 0;
    fdin = open("/dev/random", O_RDONLY | O_NONBLOCK);
    if (fdin < 0) {
        perror("/dev/random.");
        goto out_open_fdin;
    }
    fdout = open("/dev/zero", O_WRONLY | O_NONBLOCK);
    if (fdout < 0) {
        perror("/dev/zero");
        goto out_open_fdout;
    }

    ssize_t retsize;
    while ((retsize = ab_read(fdin, buff, 1024)) > 0) {
        ab_write(fdout, buff, (size_t) retsize);
        copy_count += retsize;
        fprintf(stderr, "Copied %u bytes\n", copy_count);
    }

    close(fdout);
    out_open_fdout:
    close(fdin);
    out_open_fdin:
    if (retsize) {
        fprintf(stderr, "thread_copy exited (Error:%s)\n", strerror(-(int) retsize));
    }
    return arg;
}

int test_main() {
    ab_thread_create(thread_copy, (void *) 2);
    ab_thread_create(thread_echo, (void *) 1);
    return 0;
}