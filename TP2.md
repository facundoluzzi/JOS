TP2: Procesos de usuario
========================

env_alloc
---------

1-¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal)

`generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);`

El primer proceso creado arranca con un env_id seteado en 0, luego se le suma (1 << ENVGENSHIFT) que refiere a un shift a la izquierda de 12 bits, resultando en 0 + 4096. A esos 4096 le aplicamos el operador & con ~(NENV - 1) que significa lo siguiente: NENV tiene un valor de 1 << LOG2NENV, siendo el valor de LOG2NENV definido en 10, o sea 1 << 10: 1024 en decimal. En resumen: 4096 & 1024 = 4096.

Entonces, generation = 4096.

Luego, 

`e->env_id = generation | (e - envs);`

En esta linea se aplica el operador | con (e - envs), que toma un valor entre 0 y 1024, dependiendo de la posicion en el array de entorno que le corresponda al proceso.

Al primer proceso de le asigna el env_id 4096.

Los siguientes 4 procesos toman el env_id 4097, 4098, 4099, 4100

En hexadecimal los procesos toman los ID: 0x1000, 0x1001, 0x1002, 0x1003, 0x1004


2-Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación, se destruye el proceso asociado a envs[630] y se lanza un proceso que cada segundo, muere y se vuelve a lanzar (se destruye, y se vuelve a crear). ¿Qué identificadores tendrán esos procesos en las primeras cinco ejecuciones?

En principio tenemos como vimos en la respuesta anterior el ID 0x1000 asociado al proceso n°0. Entonces, si actualmente tenemos 630 procesos, le sumamos 630 que en hexadecimal es 0x0276, y nos queda como resultado para el primer proceso 0x1276. 
A partir de eso, tomamos el valor inicial de generation como 0x1276, entonces al realizar las siguientes operaciones
generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);

e->env_id es igual a 0x1276 en el primer proceso, luego se le suma 0x1000, y se aplica el operador & con 0x0400, dando como resultado 

e->env_id = generation | (e - envs);

env_pop_tf
----------

1-Dada la secuencia de instrucciones assembly en la función, describir qué contiene durante su ejecución:

- el tope de la pila justo antes popal: como env_pop_tf recibe por parametro un puntero a struct Trapframe (tf), tf se encuentra en el tope del stack, luego con la primera instrucción dentro de asm volatile mueve el stack pointer al puntero que diga tf.
- el tope de la pila justo antes iret: el tope de la pila contiene a tf->tf_eip
- el tercer elemento de la pila justo antes de iret: el tope de la pila contiene tf->tf_eflags


2-En la documentación de iret en [IA32-2A] se dice:

    If the return is to another privilege level, the IRET instruction also pops the stack pointer and SS from the stack, before resuming program execution.

¿Cómo determina la CPU (en x86) si hay un cambio de ring (nivel de privilegio)? Ayuda: Responder antes en qué lugar exacto guarda x86 el nivel de privilegio actual. ¿Cuántos bits almacenan ese privilegio?

El lugar exacto en que x86 guarda el nivel de privilegio actual es en %cs. Cuando los dos primeros bits del cs del stack (que vienen con tf) son 3 (11), iret hace un cambio de ring a privilegios de usuario. 


gdb_hello
---------

2.En QEMU, entrar en modo monitor (Ctrl-a c), y mostrar las cinco primeras líneas del comando info registers.
```
EAX=003bc000 EBX=f01e0000 ECX=f03bc000 EDX=00000210
ESI=00010094 EDI=00000000 EBP=f0119fd8 ESP=f0119fbc
EIP=f0102fa0 EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

3.De vuelta a GDB, imprimir el valor del argumento tf:
```
$1 = (struct Trapframe *) 0xf01e0000
```
4.Imprimir, con x/Nx tf tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int).
```
0xf01e0000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01e0010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01e0020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01e0030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01e0040:	0x00000023
```
6.Comprobar, con x/Nx  que los contenidos son los mismos que tf (donde N es el tamaÃ±o de tf).

Son los mismos.
```
0xf01e0000:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01e0010:     0x00000000      0x00000000      0x00000000      0x00000000
0xf01e0020:     0x00000023      0x00000023      0x00000000      0x00000000
0xf01e0030:     0x00800020      0x0000001b      0x00000000      0xeebfe000
0xf01e0040:     0x00000023
```

7.

8.
```
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01e0030
EIP=f0102fb3 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```

9.

Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.

imprimir el valor del contador de programa con p $pc o p $eip

    $3 = (void (*)()) 0x800020

cargar los símbolos de hello con el comando add-symbol-file, así:

    (gdb) add-symbol-file obj/user/hello 0x800020
    add symbol table from file "obj/user/hello" at
            .text_addr = 0x800020
    (y or n) y
    Reading symbols from obj/user/hello...

volver a imprimir el valor del contador de programa

    $4 = (void (*)()) 0x800020 <_start>

Mostrar una última vez la salida de info registers en QEMU, y explicar los cambios producidos.

```
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]
```
10.Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la funcion syscall() y explicar que ocurre justo tras ejecutar la instruccion int x30. Usar, de ser necesario, el monitor de QEMU.

```
The target architecture is assumed to be i8086
[f000:e05b]    0xfe05b: cmpw   $0xffc8,%cs:(%esi)
0x0000e05b in ?? ()
```

kern_idt
--------

...


user_evilhello
--------------

...

