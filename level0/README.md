# Level0

### Premier etape - tester le binaire

	$ ./level0
	Segmentation fault (core dumped)

	$ ./level0 hello
	No !

Le programe segfault sans parametre et print "No !" avec parametre

### on start le binaire avec GDB

	$ gdb level0 42
	(gdb) disas main
	...
	   0x08048ed4 <+20>:	call   0x8049710 <atoi>
	   0x08048ed9 <+25>:	cmp    $0x1a7,%eax
	...

et on peut voir que la fonction atoi et appeler
et compare le resultat d'atoi avec "0x1a7" (423)

essayons se prompt

	$ ./level10 423

Parfait. on a acces au shell avec l'UID de level01

### regardons dans son home /home/user/level1

tada voici le premier flag !