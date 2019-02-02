#ifndef SYS_SYSCALL_H
#define SYS_SYSCALL_H

#define SYS_restart_syscall         0
#define SYS_exit                    1
#define SYS_fork                    2
#define SYS_read                    3
#define SYS_write                   4
#define SYS_open                    5
#define SYS_close                   6
#define SYS_waitpid                 7
#define SYS_creat                   8
#define SYS_link                    9
#define SYS_unlink                  10
#define SYS_execve                  11
#define SYS_chdir                   12
#define SYS_time                    13
#define SYS_mknod                   14
#define SYS_chmod                   15
#define SYS_lchown                  16
#define SYS_break                   17
#define SYS_oldstat                 18
#define SYS_lseek                   19
#define SYS_getpid                  20
#define SYS_mount                   21
#define SYS_umount                  22
#define SYS_setuid                  23
#define SYS_getuid                  24
#define SYS_stime                   25
#define SYS_ptrace                  26
#define SYS_alarm                   27
#define SYS_oldfstat                28
#define SYS_pause                   29
#define SYS_utime                   30
#define SYS_stty                    31
#define SYS_gtty                    32
#define SYS_access                  33
#define SYS_nice                    34
#define SYS_ftime                   35
#define SYS_sync                    36
#define SYS_kill                    37
#define SYS_rename                  38
#define SYS_mkdir                   39
#define SYS_rmdir                   40
#define SYS_dup                     41
#define SYS_pipe                    42
#define SYS_times                   43
#define SYS_prof                    44
#define SYS_brk                     45
#define SYS_setgid                  46
#define SYS_getgid                  47
#define SYS_signal                  48
#define SYS_geteuid                 49
#define SYS_getegid                 50
#define SYS_acct                    51
#define SYS_umount2                 52
#define SYS_lock                    53
#define SYS_ioctl                   54
#define SYS_fcntl                   55
#define SYS_mpx                     56
#define SYS_setpgid                 57
#define SYS_ulimit                  58
#define SYS_oldolduname             59
#define SYS_umask                   60
#define SYS_chroot                  61
#define SYS_ustat                   62
#define SYS_dup2                    63
#define SYS_getppid                 64
#define SYS_getpgrp                 65
#define SYS_setsid                  66
#define SYS_sigaction               67
#define SYS_sgetmask                68
#define SYS_ssetmask                69
#define SYS_setreuid                70
#define SYS_setregid                71
#define SYS_sigsuspend              72
#define SYS_sigpending              73
#define SYS_sethostname             74
#define SYS_setrlimit               75
#define SYS_getrlimit               76   /* Back compatible 2Gig limited rlimit */
#define SYS_getrusage               77
#define SYS_gettimeofday            78
#define SYS_settimeofday            79
#define SYS_getgroups               80
#define SYS_setgroups               81
#define SYS_select                  82
#define SYS_symlink                 83
#define SYS_oldlstat                84
#define SYS_readlink                85
#define SYS_uselib                  86
#define SYS_swapon                  87
#define SYS_reboot                  88
#define SYS_readdir                 89
#define SYS_mmap                    90
#define SYS_munmap                  91
#define SYS_truncate                92
#define SYS_ftruncate               93
#define SYS_fchmod                  94
#define SYS_fchown                  95
#define SYS_getpriority             96
#define SYS_setpriority             97
#define SYS_profil                  98
#define SYS_statfs                  99
#define SYS_fstatfs                 100
#define SYS_ioperm                  101
#define SYS_socketcall              102
#define SYS_syslog                  103
#define SYS_setitimer               104
#define SYS_getitimer               105
#define SYS_stat                    106
#define SYS_lstat                   107
#define SYS_fstat                   108
#define SYS_olduname                109
#define SYS_iopl                    110
#define SYS_vhangup                 111
#define SYS_idle                    112
#define SYS_vm86old                 113
#define SYS_wait4                   114
#define SYS_swapoff                 115
#define SYS_sysinfo                 116
#define SYS_ipc                     117
#define SYS_fsync                   118
#define SYS_sigreturn               119
#define SYS_clone                   120
#define SYS_setdomainname           121
#define SYS_uname                   122
#define SYS_modify_ldt              123
#define SYS_adjtimex                124
#define SYS_mprotect                125
#define SYS_sigprocmask             126
#define SYS_create_module           127
#define SYS_init_module             128
#define SYS_delete_module           129
#define SYS_get_kernel_syms         130
#define SYS_quotactl                131
#define SYS_getpgid                 132
#define SYS_fchdir                  133
#define SYS_bdflush                 134
#define SYS_sysfs                   135
#define SYS_personality             136
#define SYS_afs_syscall             137
#define SYS_setfsuid                138
#define SYS_setfsgid                139
#define SYS__llseek                 140
#define SYS_getdents                141
#define SYS__newselect              142
#define SYS_flock                   143
#define SYS_msync                   144
#define SYS_readv                   145
#define SYS_writev                  146
#define SYS_getsid                  147
#define SYS_fdatasync               148
#define SYS__sysctl                 149
#define SYS_mlock                   150
#define SYS_munlock                 151
#define SYS_mlockall                152
#define SYS_munlockall              153
#define SYS_sched_setparam          154
#define SYS_sched_getparam          155
#define SYS_sched_setscheduler      156
#define SYS_sched_getscheduler      157
#define SYS_sched_yield             158
#define SYS_sched_get_priority_max  159
#define SYS_sched_get_priority_min  160
#define SYS_sched_rr_get_interval   161
#define SYS_nanosleep               162
#define SYS_mremap                  163
#define SYS_setresuid               164
#define SYS_getresuid               165
#define SYS_vm86                    166
#define SYS_query_module            167
#define SYS_poll                    168
#define SYS_nfsservctl              169
#define SYS_setresgid               170
#define SYS_getresgid               171
#define SYS_prctl                   172
#define SYS_rt_sigreturn            173
#define SYS_rt_sigaction            174
#define SYS_rt_sigprocmask          175
#define SYS_rt_sigpending           176
#define SYS_rt_sigtimedwait         177
#define SYS_rt_sigqueueinfo         178
#define SYS_rt_sigsuspend           179
#define SYS_pread64                 180
#define SYS_pwrite64                181
#define SYS_chown                   182
#define SYS_getcwd                  183
#define SYS_capget                  184
#define SYS_capset                  185
#define SYS_sigaltstack             186
#define SYS_sendfile                187
#define SYS_getpmsg                 188
#define SYS_putpmsg                 189
#define SYS_vfork                   190
#define SYS_ugetrlimit              191
#define SYS_mmap2                   192
#define SYS_truncate64              193
#define SYS_ftruncate64             194
#define SYS_stat64                  195
#define SYS_lstat64                 196
#define SYS_fstat64                 197
#define SYS_lchown32                198
#define SYS_getuid32                199
#define SYS_getgid32                200
#define SYS_geteuid32               201
#define SYS_getegid32               202
#define SYS_setreuid32              203
#define SYS_setregid32              204
#define SYS_getgroups32             205
#define SYS_setgroups32             206
#define SYS_fchown32                207
#define SYS_setresuid32             208
#define SYS_getresuid32             209
#define SYS_setresgid32             210
#define SYS_getresgid32             211
#define SYS_chown32                 212
#define SYS_setuid32                213
#define SYS_setgid32                214
#define SYS_setfsuid32              215
#define SYS_setfsgid32              216
#define SYS_pivot_root              217
#define SYS_mincore                 218
#define SYS_madvise                 219
#define SYS_getdents64              220
#define SYS_fcntl64                 221
#define SYS_gettid                  224
#define SYS_readahead               225
#define SYS_setxattr                226
#define SYS_lsetxattr               227
#define SYS_fsetxattr               228
#define SYS_getxattr                229
#define SYS_lgetxattr               230
#define SYS_fgetxattr               231
#define SYS_listxattr               232
#define SYS_llistxattr              233
#define SYS_flistxattr              234
#define SYS_removexattr             235
#define SYS_lremovexattr            236
#define SYS_fremovexattr            237
#define SYS_tkill                   238
#define SYS_sendfile64              239
#define SYS_futex                   240
#define SYS_sched_setaffinity       241
#define SYS_sched_getaffinity       242
#define SYS_set_thread_area         243
#define SYS_get_thread_area         244
#define SYS_io_setup                245
#define SYS_io_destroy              246
#define SYS_io_getevents            247
#define SYS_io_submit               248
#define SYS_io_cancel               249
#define SYS_fadvise64               250
#define SYS_exit_group              252
#define SYS_lookup_dcookie          253
#define SYS_epoll_create            254
#define SYS_epoll_ctl               255
#define SYS_epoll_wait              256
#define SYS_remap_file_pages        257
#define SYS_set_tid_address         258
#define SYS_timer_create            259
#define SYS_timer_settime           260
#define SYS_timer_gettime           261
#define SYS_timer_getoverrun        262
#define SYS_timer_delete            263
#define SYS_clock_settime           264
#define SYS_clock_gettime           265
#define SYS_clock_getres            266
#define SYS_clock_nanosleep         267
#define SYS_statfs64                268
#define SYS_fstatfs64               269
#define SYS_tgkill                  270
#define SYS_utimes                  271
#define SYS_fadvise64_64            272
#define SYS_vserver                 273
#define SYS_mbind                   274
#define SYS_get_mempolicy           275
#define SYS_set_mempolicy           276
#define SYS_mq_open                 277
#define SYS_mq_unlink               278
#define SYS_mq_timedsend            279
#define SYS_mq_timedreceive         280
#define SYS_mq_notify               281
#define SYS_mq_getsetattr           282
#define SYS_kexec_load              283
#define SYS_waitid                  284
#define SYS_sys_setaltroot          285
#define SYS_add_key                 286
#define SYS_request_key             287
#define SYS_keyctl                  288
#define SYS_ioprio_set              289
#define SYS_ioprio_get              290
#define SYS_inotify_init            291
#define SYS_inotify_add_watch       292
#define SYS_inotify_rm_watch        293
#define SYS_migrate_pages           294
#define SYS_openat                  295
#define SYS_mkdirat                 296
#define SYS_mknodat                 297
#define SYS_fchownat                298
#define SYS_futimesat               299
#define SYS_fstatat64               300
#define SYS_unlinkat                301
#define SYS_renameat                302
#define SYS_linkat                  303
#define SYS_symlinkat               304
#define SYS_readlinkat              305
#define SYS_fchmodat                306
#define SYS_faccessat               307
#define SYS_pselect6                308
#define SYS_ppoll                   309
#define SYS_unshare                 310
#define SYS_set_robust_list         311
#define SYS_get_robust_list         312
#define SYS_splice                  313
#define SYS_sync_file_range         314
#define SYS_tee                     315
#define SYS_vmsplice                316
#define SYS_move_pages              317
#define SYS_getcpu                  318
#define SYS_epoll_pwait             319
#define SYS_utimensat               320
#define SYS_signalfd                321
#define SYS_timerfd_create          322
#define SYS_eventfd                 323
#define SYS_fallocate               324
#define SYS_timerfd_settime         325
#define SYS_timerfd_gettime         326
#define SYS_signalfd4               327
#define SYS_eventfd2                328
#define SYS_epoll_create1           329
#define SYS_dup3                    330
#define SYS_pipe2                   331
#define SYS_inotify_init1           332
#define SYS_preadv                  333
#define SYS_pwritev                 334
#define SYS_rt_tgsigqueueinfo       335
#define SYS_perf_event_open         336
#define SYS_recvmmsg                337
#define SYS_fanotify_init           338
#define SYS_fanotify_mark           339
#define SYS_prlimit64               340
#define SYS_name_to_handle_at       341
#define SYS_open_by_handle_at       342
#define SYS_clock_adjtime           343
#define SYS_syncfs                  344
#define SYS_sendmmsg                345
#define SYS_setns                   346
#define SYS_process_vm_readv        347
#define SYS_process_vm_writev       348
#define SYS_kcmp                    349
#define SYS_finit_module            350
#define SYS_sched_setattr           351
#define SYS_sched_getattr           352
#define SYS_renameat2               353
#define SYS_seccomp                 354
#define SYS_getrandom               355
#define SYS_memfd_create            356
#define SYS_bpf                     357
#define SYS_execveat                358
#define SYS_socket                  359
#define SYS_socketpair              360
#define SYS_bind                    361
#define SYS_connect                 362
#define SYS_listen                  363
#define SYS_accept4                 364
#define SYS_getsockopt              365
#define SYS_setsockopt              366
#define SYS_getsockname             367
#define SYS_getpeername             368
#define SYS_sendto                  369
#define SYS_sendmsg                 370
#define SYS_recvfrom                371
#define SYS_recvmsg                 372
#define SYS_shutdown                373
#define SYS_userfaultfd             374
#define SYS_membarrier              375
#define SYS_mlock2                  376
#define SYS_copy_file_range         377
#define SYS_preadv2                 378
#define SYS_pwritev2                379
#define SYS_pkey_mprotect           380
#define SYS_pkey_alloc              381
#define SYS_pkey_free               382
#define SYS_statx                   383
#define SYS_arch_prctl              384

#define SYS_WOOT_specific           385
#define SYS_create_window           386
#define SYS_show_window             387
#define SYS_hide_window             388
#define SYS_destroy_window          389
#define SYS_draw_rectangle          390
#define SYS_draw_filled_rectangle   391
#define SYS_update_window           392
#define SYS_redraw_window           393
#define SYS_draw_line               394
#define SYS_blit                    395
#define SYS_alpha_blit              396
#define SYS_map_window              397
#define SYS_invalidate_rect         398
#define SYS_get_window_size         399
#define SYS_get_pixel_format        400
#define SYS_set_drag_rect           401
#define SYS_get_event               402
#define SYS_peek_event              403

#define SYS_audio_open              404
#define SYS_audio_close             405
#define SYS_audio_get_frame_size    406
#define SYS_audio_write             407
#define SYS_audio_start_playback    408
#define SYS_audio_stop_playback     409
#define SYS_audio_get_buffer_count  410
#define SYS_audio_get_device_vendor 411
#define SYS_audio_get_device_model  412

#define SYS_redraw_screen           500
#define SYS_sleep_ms                501
#define SYS_get_ticks               502
#define SYS_get_tick_freq           503

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

long syscall0(long number);
long syscall1(long number, long arg1);
long syscall2(long number, long arg1, long arg2);
long syscall3(long number, long arg1, long arg2, long arg3);
long syscall4(long number, long arg1, long arg2, long arg3, long arg4);
long syscall5(long number, long arg1, long arg2, long arg3, long arg4, long arg5);
long syscall6(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SYS_SYSCALL_H
