#as: --64 -madd-bnd-prefix
#ld: -shared -melf_x86_64 -z bndplt --hash-style=sysv
#objdump: -dw

#...
[ 	]*[a-f0-9]+:	f2 e8 f0 ff ff ff    	bnd callq 210 <\*ABS\*\+0x218@plt>
#pass
