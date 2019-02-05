#ifndef SYS_IOCTL_H
#define SYS_IOCTL_H

#define TCGETS           0x00005401 // struct termios *
#define TCSETS           0x00005402 // const struct termios *
#define TCSETSW          0x00005403 // const struct termios *
#define TCSETSF          0x00005404 // const struct termios *
#define TCGETA           0x00005405 // struct termio *
#define TCSETA           0x00005406 // const struct termio *
#define TCSETAW          0x00005407 // const struct termio *
#define TCSETAF          0x00005408 // const struct termio *
#define TCSBRK           0x00005409 // int
#define TCXONC           0x0000540A // int
#define TCFLSH           0x0000540B // int
#define TIOCEXCL         0x0000540C // void
#define TIOCNXCL         0x0000540D // void
#define TIOCSCTTY        0x0000540E // int
#define TIOCGPGRP        0x0000540F // pid_t *
#define TIOCSPGRP        0x00005410 // const pid_t *
#define TIOCOUTQ         0x00005411 // int *
#define TIOCSTI          0x00005412 // const char *
#define TIOCGWINSZ       0x00005413 // struct winsize *
#define TIOCSWINSZ       0x00005414 // const struct winsize *
#define TIOCMGET         0x00005415 // int *
#define TIOCMBIS         0x00005416 // const int *
#define TIOCMBIC         0x00005417 // const int *
#define TIOCMSET         0x00005418 // const int *
#define TIOCGSOFTCAR     0x00005419 // int *
#define TIOCSSOFTCAR     0x0000541A // const int *
#define FIONREAD         0x0000541B // int *
#define TIOCINQ          0x0000541B // int *
#define TIOCLINUX        0x0000541C // const char *
#define TIOCCONS         0x0000541D // void
#define TIOCGSERIAL      0x0000541E // struct serial_struct *
#define TIOCSSERIAL      0x0000541F // const struct serial_struct *
#define TIOCPKT          0x00005420 // const int *
#define FIONBIO          0x00005421 // const int *
#define TIOCNOTTY        0x00005422 // void
#define TIOCSETD         0x00005423 // const int *
#define TIOCGETD         0x00005424 // int *
#define TCSBRKP          0x00005425 // int
#define TIOCTTYGSTRUCT   0x00005426 // struct tty_struct *
#define FIONCLEX         0x00005450 // void
#define FIOCLEX          0x00005451 // void
#define FIOASYNC         0x00005452 // const int *
#define TIOCSERCONFIG    0x00005453 // void
#define TIOCSERGWILD     0x00005454 // int *
#define TIOCSERSWILD     0x00005455 // const int *
#define TIOCGLCKTRMIOS   0x00005456 // struct termios *
#define TIOCSLCKTRMIOS   0x00005457 // const struct termios *
#define TIOCSERGSTRUCT   0x00005458 // struct async_struct *
#define TIOCSERGETLSR    0x00005459 // int *
#define TIOCSERGETMULTI  0x0000545A // struct serial_multiport_struct *
#define TIOCSERSETMULTI  0x0000545B // const struct serial_multiport_struct *

struct winsize
{
    unsigned short ws_row;	/* rows, in characters */
    unsigned short ws_col;	/* columns, in characters */
    unsigned short ws_xpixel;	/* horizontal size, pixels */
    unsigned short ws_ypixel;	/* vertical size, pixels */
};

#endif // SYS_IOCTL_H
