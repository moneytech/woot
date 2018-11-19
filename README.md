WOOT is yet another a hobby operating system.

First time you build it you have to create disk image file. To do that type `make clean-img setup-grub hdd.img` in project's root directory. Any other subsequent build is `make hdd.img`. If you just want to build files without copying them to disk image use `make all` or `make` alone.

Makefiles are meant to be build under Linux. Some makefile rules use sudo so you may want to edit your sudoers file accordingly. 

There is also a way to create ISO file. More info in top-level Makefile.

And this is how it looks now. 
![Screenshot](screenshot.png?raw=true "Screenshot")
