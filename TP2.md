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


<<<<<<< HEAD
2-Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación, se destruye el proceso asociado a envs[630] y se lanza un proceso que cada segundo, muere y se vuelve a lanzar (se destruye, y se vuelve a crear). ¿Qué identificadores tendrán esos procesos en las primeras cinco ejecuciones?


Aplicando lo del ejercicio anterior, se toma el 0 como ID base, y el nuevo generation sera la suma entre este id, 0x1000 (1 << ENVGENSHIFT) y a eso aplicar el operador & con ~(NENV - 1). A eso se le debe sumar el offset 0x0276 (630 en hexa, ya que tenemos 630 procesos), resultando entonces los siguientes ID's:

La operación es la siguiente: (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1) + 0x0276

Arrancamos con e->env_id igual a 0:

Los primeros 5 procesos tendran el ID:

1er proceso: (0x0000 + (0x1000) & ~(0x03FF) + 0x0276 = 0x1276

2do proceso: (0x1276 + (0x1000) & ~(0x03FF) + 0x0276 = 0x2276

3er proceso: (0x2276 + (0x1000) & ~(0x03FF) + 0x0276 = 0x3276

4to proceso: (0x3276 + (0x1000) & ~(0x03FF) + 0x0276 = 0x4276

5to proceso: (0x4276 + (0x1000) & ~(0x03FF) + 0x0276 = 0x5276


=======
>>>>>>> catedra/tp3
env_pop_tf
----------

1-Dada la secuencia de instrucciones assembly en la función, describir qué contiene durante su ejecución:

- el tope de la pila justo antes popal: como env_pop_tf recibe por parametro un puntero a struct Trapframe (tf), tf se encuentra en el tope del stack, luego con la primera instrucción dentro de asm volatile mueve el stack pointer al puntero que diga tf.
- el tope de la pila justo antes iret: el tope de la pila contiene a tf->tf_eip
- el tercer elemento de la pila justo antes de iret: el tope de la pila contiene tf->tf_eflags


2-En la documentación de iret en [IA32-2A] se dice:

    If the return is to another privilege level, the IRET instruction also pops the stack pointer and SS from the stack, before resuming program execution.

¿Cómo determina la CPU (en x86) si hay un cambio de ring (nivel de privilegio)? Ayuda: Responder antes en qué lugar exacto guarda x86 el nivel de privilegio actual. ¿Cuántos bits almacenan ese privilegio?

El lugar exacto en que x86 guarda el nivel de privilegio actual es en %cs. Cuando los dos primeros bits del cs del stack (que vienen con tf) son 3 (11), iret hace un cambio de ring a privilegios de usuario. Adjuntamos el codigo de microinstrucciones del procesador.

```PROTECTED-MODE-RETURN: (* PE = 1 *)
IF CS(RPL) > CPL
THEN GOTO RETURN-TO-OUTER-PRIVILEGE-LEVEL;
ELSE GOTO RETURN-TO-SAME-PRIVILEGE-LEVEL; 
FI;
END;
```


gdb_hello
---------
1.Poner un breakpoint en env_pop_tf() y continuar la ejecución hasta allí.

<<<<<<< HEAD
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
4.Imprimir, con x/Nx tf tantos enteros como haya en el struct Trapframe donde N = sizeof(Trapframe) / sizeof(int) = 17.
```
0xf01e0000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01e0010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01e0020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01e0030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01e0040:	0x00000023
```

5.Avanzar hasta justo después del movl ...,%esp, usando si M para ejecutar tantas instrucciones como sea necesario en un solo paso.

6.Comprobar, con x/Nx %sp que los contenidos son los mismos que tf (donde N es el tamano de tf).
Lo mismo quedo.

```
0xf01e0000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01e0010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01e0020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01e0030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01e0040:	0x00000023

```

7.Describir cada uno de los valores. Para los valores no nulos, se debe indicar dónde se configuró inicialmente el valor, y qué representa.

Los primeros 8 numeros son los registros de uso general.Luego:

El tf_es (selector de segmento para segmentos de datos) 0x0000023

El tf_ds (descriptor de segmentos para segmento de datos) 0x00000023.

El tf_trapno (numero de trap), el 0x00000000.

El tf_err (codigo error), el  0x00000000.

El tf_eip (env instruction pointer), el 0x00800020.

El tf_cs (codigo segmento), el 0x0000001b.

El tf_eflags (flags), el 0x00000000.

El tf_esp (stack pointer), el 0xeebfe000.

El ultimo es le tf_ss (segmento de pila), el 0x00000023.

Las definiciones de los no nulos se dan en env_alloc(). Adjuntamos parte del codigo.

```
	e->env_tf.tf_ds = GD_UD | 3;
	e->env_tf.tf_es = GD_UD | 3;
	e->env_tf.tf_ss = GD_UD | 3;
	e->env_tf.tf_esp = USTACKTOP;
	e->env_tf.tf_cs = GD_UT | 3;
```

8.Continuar hasta la instrucción iret, sin llegar a ejecutarla. Mostrar en este punto, de nuevo, las cinco primeras líneas de info registers en el monitor de QEMU. Explicar los cambios producidos.
```
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01e0030
EIP=f0102fb3 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]
```
Se limpian los registros de uso general, se produce el cambio de contexto, saltando el ESP y pasando el DPL a 3.

9.Ejecutar la instrucción iret. En ese momento se ha realizado el cambio de contexto y los símbolos del kernel ya no son válidos.

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
10.Poner un breakpoint temporal (tbreak, se aplica una sola vez) en la funcion syscall() y explicar que ocurre justo tras ejecutar la instruccion int 0x30. Usar, de ser necesario, el monitor de QEMU.


```
The target architecture is assumed to be i8086
[f000:e05b]    0xfe05b: cmpw   $0xffc8,%cs:(%esi)
0x0000e05b in ?? ()

info registers

EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000663
ESI=00000000 EDI=00000000 EBP=00000000 ESP=00000000
EIP=0000e05b EFL=00000002 [-------] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0000 00000000 0000ffff 00009300
CS =f000 000f0000 0000ffff 00009b00
```

int 0x30 es el numero de trap de la syscall, que todavia no esta codeado. Cuando se intenta lanzar la interrupcion, todavia no esta la tabla de interrupciones ni los handlers, entonces rompe. Se puede ver el ESP = 0, cosas que denota un error grave.

kern_idt
--------
La interrupción que trata de generar user/softint.c es un page fault. La interrupción que se genera es 
```
trap 0x0000000d General Protection
```
En user/softint.c estamos intentando lanzar una interrupción de tipo page fault, en un proceso con nivel de privilegio 3 (user mode), y como la interrupcion requiere un nivel de privilegio 0 para poder ser lanzada, el sistema se protege lanzando la interrupción general protection. 

Podriamos habilitar los permisos necesarios para que el nivel de privilegio 3 pueda lanzar la interrupción page fault.

=======
...


kern_idt
--------

...
>>>>>>> catedra/tp3


user_evilhello
--------------

<<<<<<< HEAD

-¿En qué se diferencia el código de la versión en evilhello.c mostrada arriba?

El modificado desrreferencia el puntero al inicio del kernel en el lado del usuario. 

El original manda la syscall para que se desrreferencie dentro, entonces el kernel lo hace.

-¿En qué cambia el comportamiento durante la ejecución?
¿Por qué? ¿Cuál es el mecanismo?


Original
```
[00000000] new env 00001000
Incoming TRAP frame at 0xefffffbc
f�rIncoming TRAP frame at 0xefffffbc
[00001000] exiting gracefully
[00001000] free env 00001000
Destroyed the only environment - nothing more to do!
```
Modificado
```[00000000] new env 00001000
Incoming TRAP frame at 0xefffffbc
[00001000] user fault va f010000c ip 0080003d
TRAP frame at 0xf01e0000
  edi  0x00000000
  esi  0x00000000
  ebp  0xeebfdfd0
  oesp 0xefffffdc
  ebx  0x00000000
  edx  0x00000000
  ecx  0x00000000
  eax  0x00000000
  es   0x----0023
  ds   0x----0023
  trap 0x0000000e Page Fault
  cr2  0xf010000c
  err  0x00000005 [user, read, protection]
  eip  0x0080003d
  cs   0x----001b
  flag 0x00000082
  esp  0xeebfdfb0
  ss   0x----0023
[00001000] free env 00001000
Destroyed the only environment - nothing more to do!
```
Se ve claramente que el caso modificado tira Page Fault mientras que el otro pasa de largo. Pasa de largo (original) porque la parte de acceder a la memoria se la deja al kernel, y este con sus permisos y sin una barrera logica que lo evite, la accede sin problemas. En el otro caso (modificado) al querer acceder a memoria en `char first = *entry;` desde el lado de usuario, tira page fault.

-Listar las direcciones de memoria que se acceden en ambos casos, y en qué ring se realizan. ¿Es esto un problema? ¿Por qué?

Caso original:
    
-Se accede al comienzo del kernel dentro de la syscall cputs, o sea que lo hace el kernel. Esto se hace en ring 0.

Esta tiene el problema de que no verifica la que la memoria.

Caso modificado:

-Se accede al comienzo del kernel en `char first = *entry;`, lo hace el usuario. Se hace en ring 1, y tira page fault.
=======
...

>>>>>>> catedra/tp3
