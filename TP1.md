TP1: Memoria virtual en JOS
===========================

boot_alloc_pos
--------------

1) 
Buscamos el final del kernel:
```
(base) ➜  sisop_2022a_g04_argel_luzzi git:(entrega_tp1) nm obj/kern/kernel | grep end 
f0118970 B end
```
La direccion final del kernel es f0118970 = 4027681136
Luego del arranque, la dirección que recibe boot_alloc() es 4027681136. A este valor, boot_alloc() lo redondea a PGSIZE utilizando ROUNDUP, y almacena en nextfree una pagina mas de lo recibido. 


Por ejemplo, boot_alloc() hace lo siguiente:
```
4027681136 mod 4096 = 2416 \\ resto
4096 - 2416 = 1680 \\ le restamos a 4096 el resto, y es lo que nos faltaria agregar para que sea multiplo de PGSIZE
4027681136 + 1680 = 4027682816 
```
La primera direccion de memoria que devolvera es 4027682816 = f0119000

2)
```
(base) ➜  sisop_2022a_g04_argel_luzzi git:(entrega_tp1) ✗ make gdb
gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
Reading symbols from obj/kern/kernel...
Remote debugging using 127.0.0.1:26000
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000fff0 in ?? ()
(gdb) break boot_alloc
Breakpoint 1 at 0xf0100a60: file kern/pmap.c, line 98.
(gdb) continue
Continuing.
The target architecture is assumed to be i386
=> 0xf0100a60 <boot_alloc>:	cmpl   $0x0,0xf0117538

Breakpoint 1, boot_alloc (n=4096) at kern/pmap.c:98
98		if (!nextfree) {
(gdb) print (char*) &end
$1 = 0xf0118970 ""
(gdb) watch &end
Watchpoint 2: &end
(gdb) watch nextfree
Hardware watchpoint 3: nextfree
(gdb) continue
Continuing.
=> 0xf0100aa5 <boot_alloc+69>:	jmp    0xf0100a69 <boot_alloc+9>

Hardware watchpoint 3: nextfree

Old value = 0x0
New value = 0xf0119000 ""
0xf0100aa5 in boot_alloc (n=4096) at kern/pmap.c:100
100			nextfree = ROUNDUP((char *) end, PGSIZE);
(gdb) continue
Continuing.
=> 0xf0100a8f <boot_alloc+47>:	test   %ecx,%ecx

Hardware watchpoint 3: nextfree

Old value = 0xf0119000 ""
New value = 0xf011a000 ""
boot_alloc (n=4027686912) at kern/pmap.c:111
111		if (exceeded > 0) {
(gdb) continue
Continuing.
=> 0xf0100a60 <boot_alloc>:	cmpl   $0x0,0xf0117538

Breakpoint 1, boot_alloc (n=131072) at kern/pmap.c:98
98		if (!nextfree) {
(gdb) 
```

Podemos observar que end comienza en 0xf0118970, le asigna a nextfree el valor 0xf0118970 redondeado a 4096, o sea, 0xf0119000, y finalmente avanza una pagina y almacena el valor 0xf011a000

map_region_large
----------------

1) 
------
La siguiente llamada de boot_map_region() no tiene la physical address alineada:

```
boot_map_region(kern_pgdir,
	            UPAGES,
	            ROUNDUP(npages * sizeof(struct PageInfo), PGSIZE),
	            PADDR(pages),
	            PTE_U | PTE_P);
```
Corrimos qemu printeando las direcciones de UPAGES y PADDR(pages). Consideramos que no hay forma teorica de encontrar las posiciones tanto fisica como virtual porque son resultado de una operacion que depende de la memoria:
```
UPAGES: 4009754624 , PADDR = 1286144
```
Se ve claramente que PADDR(pages) no es multiplo de 4MiB y por lo tanto no esta alineado.

------------

La siguiente llamada de boot_map_region() es menor a 4MiB => no asigna large page:

```
boot_map_region(kern_pgdir,
	                KSTACKTOP - KSTKSIZE,
	                KSTKSIZE,
	                PADDR(bootstack),
	                PTE_W);
```
------
La siguiente llamada de boot_map_region() llama y hace esta cuenta.
```
boot_map_region(kern_pgdir, KERNBASE, (0xFFFFFFFF - KERNBASE), 0, PTE_W);

```
```
a = 4294967295 // 0xFFFFFFFF
b = 4026531840 // 0xF0000000 
c = (a - b)
print(c / 4194304) //

=> ~64
```
En total se utilizaron 64 large pages, ahorrando de esta manera 64 * 4KiB = 256 KiB.

2.
Adjuntamos parte del enunciado:
```
El arreglo se crea en tiempo de ejecución porque el número de páginas no es una constante sino que depende de la cantidad real de memoria de la máquina donde corre JOS.
```
Esto refiere a el vector de pages y aclara que depende de la memoria de la maquina en la que corre. Por lo tanto, la cantidad de pages y/o su posicion puedan quedar alineadas y mapearse con paginas largas. Las otras dos llamadas son con posiciones fisicas y virtuales constantes, asi como el size tambien constante.