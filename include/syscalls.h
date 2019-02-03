#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <ints.h>
#include <types.h>

#define MAX_SYSCALLS 768

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
    static long sys_fstat(long *args); // 108
    static long sys_fsync(long *args); // 118
    static long sys_llseek(long *args); // 140
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
    static long sys_get_event(long *args); // 402
    static long sys_peek_event(long *args); // 403

    static long sys_audio_open(long *args); // 404
    static long sys_audio_close(long *args); // 405
    static long sys_audio_get_frame_size(long *args); // 406
    static long sys_audio_write(long *args); // 407
    static long sys_audio_start_playback(long *args); // 408
    static long sys_audio_stop_playback(long *args); // 409
    static long sys_audio_get_buffer_count(long *args); // 410
    static long sys_audio_get_device_vendor(long *args); // 411
    static long sys_audio_get_device_model(long *args); // 412

    static long sys_redraw_screen(long *args); // 500
    static long sys_sleep_ms(long *args); // 501
    static long sys_get_ticks(long *args); // 502
    static long sys_get_tick_freq(long *args); // 503
    static long sys_thread_create(long *args); // 504
    static long sys_thread_delete(long *args); // 505
    static long sys_thread_suspend(long *args); // 506
    static long sys_thread_resume(long *args); // 507
    static long sys_thread_sleep(long *args); // 508
    static long sys_mutex_create(long *args); // 509
    static long sys_mutex_delete(long *args); // 510
    static long sys_mutex_acquire(long *args); // 511
    static long sys_mutex_release(long *args); // 512
    static long sys_mutex_cancel(long *args); // 513
    static long sys_semaphore_create(long *args); // 514
    static long sys_semaphore_delete(long *args); // 515
    static long sys_semaphore_wait(long *args); // 516
    static long sys_semaphore_signal(long *args); // 517
    static long sys_semaphore_reset(long *args); // 518
    static long sys_semaphore_cancel(long *args); // 519
    static long sys_semaphore_get_count(long *args); // 520

public:
    static void Initialize();
    static void Cleanup();
};

#endif // SYSCALLS_H
