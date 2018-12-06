#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <ints.h>
#include <types.h>

#define MAX_SYSCALLS 512

class SysCalls
{
    typedef long (*Callback)(long *args);

    static Ints::Handler handler;
    static Callback callbacks[MAX_SYSCALLS];
    static bool isr(Ints::State *state, void *context);

    static long sys_exit(long *args); // 1
    static long sys_read(long *args); // 3
    static long sys_write(long *args); // 4
    static long sys_open(long *args); // 5
    static long sys_close(long *args); // 6
    static long sys_time(long * args); // 13
    static long sys_lseek(long *args); // 19
    static long sys_getpid(long *args); // 20
    static long sys_mkdir(long *args); // 39
    static long sys_brk(long *args); // 45
    static long sys_readdir(long *args); // 89
    static long sys_munmap(long *args); // 91
    static long sys_stat(long *args); // 106
    static long sys_fsync(long *args); // 118
    static long sys_fdatasync(long *args); // 148
    static long sys_nanosleep(long *args); // 162
    static long sys_getcwd(long *args); // 183
    static long sys_gettid(long *args); // 224

    static long sys_create_window(long *args); // 386
    static long sys_show_window(long *args); // 387
    static long sys_hide_window(long *args); // 388
    static long sys_destroy_window(long *args); // 389
    static long sys_draw_rectangle(long *args); // 390
    static long sys_draw_filled_rectangle(long *args); // 391
    static long sys_update_window(long *args); // 392
    static long sys_redraw_window(long *args); // 393
    static long sys_draw_line(long *args); // 394
    static long sys_blit(long *args); // 395
    static long sys_alpha_blit(long *args); // 396
    static long sys_map_window(long *args); // 397
    static long sys_invalidate_rect(long *args); // 398
    static long sys_get_window_size(long *args); // 399
    static long sys_get_pixel_format(long *args); // 400
    static long sys_set_drag_rect(long *args); // 401
public:
    static void Initialize();
    static void Cleanup();
};

#endif // SYSCALLS_H
