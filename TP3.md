TP3: Multitarea con desalojo
============================

sys_yield
---------
1.Manejar, en kern/syscall.c, la llamada al sistema SYS_yield, que simplemente llama a sched_yield(). En la biblioteca de usuario de JOS ya se encuentra definida la función correspondiente sys_yield().

Esto se hizo en el codigo.

2.Leer y estudiar el código del programa user/yield.c. Cambiar la función i386_init() para lanzar tres instancias de dicho programa, y mostrar y explicar la salida de make qemu-nox.

```
SMP: CPU 0 found 1 CPU(s)
enabled interrupts: 1 2
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
Hello, I am environment 00001000.
Hello, I am environment 00001001.
Hello, I am environment 00001002.
Back in environment 00001000, iteration 0.
Back in environment 00001001, iteration 0.
Back in environment 00001002, iteration 0.
Back in environment 00001000, iteration 1.
Back in environment 00001001, iteration 1.
Back in environment 00001002, iteration 1.
Back in environment 00001000, iteration 2.
Back in environment 00001001, iteration 2.
Back in environment 00001002, iteration 2.
Back in environment 00001000, iteration 3.
Back in environment 00001001, iteration 3.
Back in environment 00001000, iteration 4.
All done in environment 00001000.
[00001000] exiting gracefully
[00001000] free env 00001000
Back in environment 00001001, iteration 4.
All done in environment 00001001.
[00001001] exiting gracefully
[00001001] free env 00001001
Back in environment 00001002, iteration 3.
Back in environment 00001002, iteration 4.
All done in environment 00001002.
[00001002] exiting gracefully
[00001002] free env 00001002
No runnable environments in the system!
Welcome to the JOS kernel monitor!
```
En la salida de make qemu-nox se ve claramente el trabajo de un scheduler RoundRobin. Cada proceso tiene su iteracion y se le da paso al otro proceso para la suya, y asi sucesivamente. 

Desde ya que esto se da por la llamada a `sys_yield()`, quien imprime la logica de cuando hacer los saltos entre procesos. Este seria un caso 'cooperativo' en el que cada proceso se 'desaloja' por si solo.

Adjuntamos la funcion:

```
#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int i;

	cprintf("Hello, I am environment %08x.\n", thisenv->env_id);
	for (i = 0; i < 5; i++) {
		sys_yield();
		cprintf("Back in environment %08x, iteration %d.\n",
			thisenv->env_id, i);
	}
	cprintf("All done in environment %08x.\n", thisenv->env_id);
}

```



dumbfork
--------


1.Si una página no es modificable en el padre ¿lo es en el hijo? En otras palabras: ¿se preserva, en el hijo, el flag de solo-lectura en las páginas copiadas?

Por este hint de la funcion sys_page_map, cuando el padre solo posee permisos de read-only, no deberian setearse permisos de escritura al hijo, y en caso de que esto ocurra se considerará un error. Por otro lado, no se preserva el flag de read-only en las páginas copiadas, ya que dentro de duppage se llama a sys_page_alloc() con permisos de escritura. 
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.


2.Mostrar, con código en espacio de usuario, cómo podría dumbfork() verificar si una dirección en el padre es de solo lectura, de tal manera que pudiera pasar como tercer parámetro a duppage() un booleano llamado readonly que indicase si la página es modificable o no:

    envid_t dumbfork(void) {
        // ...
        for (addr = UTEXT; addr < end; addr += PGSIZE) {
            bool readonly;
            if (uvpd[PDX(addr)] & PTE_P) != PTE_P) {
                continue;
            }
            readonly = uvpt[PGNUM(addr)] & PTE_W;
            duppage(envid, addr, readonly);
        }
        // ...

Ayuda: usar las variables globales uvpd y/o uvpt.

3.Supongamos que se desea actualizar el código de duppage() para tener en cuenta el argumento readonly: si este es verdadero, la página copiada no debe ser modificable en el hijo. Es fácil hacerlo realizando una última llamada a sys_page_map() para eliminar el flag PTE_W en el hijo, cuando corresponda:

    void duppage(envid_t dstenv, void *addr, bool readonly) {
        // Código original (simplificado): tres llamadas al sistema.
        sys_page_alloc(dstenv, addr, PTE_P | PTE_U | PTE_W);
        sys_page_map(dstenv, addr, 0, UTEMP, PTE_P | PTE_U | PTE_W);

        memmove(UTEMP, addr, PGSIZE);
        sys_page_unmap(0, UTEMP);

        // Código nuevo: una llamada al sistema adicional para solo-lectura.
        if (readonly) {
            sys_page_map(dstenv, addr, dstenv, addr, PTE_P | PTE_U);
        }
    }

Esta versión del código, no obstante, incrementa las llamadas al sistema que realiza duppage() de tres, a cuatro. Se pide mostrar una versión en el que se implemente la misma funcionalidad readonly, pero sin usar en ningún caso más de tres llamadas al sistema.

Ayuda: Leer con atención la documentación de sys_page_map() en kern/syscall.c, en particular donde avisa que se devuelve error:

        if (perm & PTE_W) is not zero, but srcva is read-only in srcenvid’s address space.

    void duppage(envid_t dstenv, void *addr, bool readonly) {
        int perm;
        if (readonly) {
            perm = PTE_P | PTE_U;
        } else {
            perm = PTE_P | PTE_U | PTE_W;
        }

        sys_page_alloc(dstenv, addr, perm);
        sys_page_map(dstenv, addr, 0, UTEMP, perm);

        memmove(UTEMP, addr, PGSIZE);
        sys_page_unmap(0, UTEMP);
    }


ipc_recv
--------

Un proceso podría intentar enviar el valor númerico -E_INVAL vía ipc_send(). ¿Cómo es posible distinguir si es un error, o no?

envid_t src = -1;
int r = ipc_recv(&src, 0, NULL);

// Facilmente checkeable en ipc_recv() podemos observar que cuando la syscall falla, se almacena el valor 0 en
// from_env_store, que en este caso es el puntero a src, por lo que src toma el valor 0.
if (r < 0)
  if (src == 0) 
    puts("Hubo error.");
  else
    puts("Valor negativo correcto.")


sys_ipc_try_send
----------------

Una estrategia que podemos utilizar es la siguiente: si un proceso A intenta enviar un mensaje a un proceso B, pero B no esta esperando un mensaje, entonces se setea al proceso A en estado ENV_NOT_RUNNABLE, y guardamos en una cola 'senders' propia del proceso B, una referencia al proceso A. Entonces, 'senders' va a contener en cada posicion una referencia a cada proceso que haya llamado a sys_ipc_send() para enviarle un mensaje. Luego, cuando el proceso B llame a sys_ipc_recv() lo primero que hara sera chequear si hay alguna referencia dentro de la cola 'senders'. En caso afirmativo, se realiza una llamada a sys_env_set_status(), seteando al proceso A (o el proceso remitente del mensaje) en estado ENV_RUNNABLE. 

Las modificaciones correspondientes serian:

- cambios en struct Env:
    1. senders: cola (atributo) que contiene una referencia a cada proceso que intento enviar un mensaje al Env correspondiente, mientras que este no estaba esperando un mensaje.
    2. env_add_sender(process): metodo para agregar un elemento a la cola senders.
    3. env_get_sender(): metodo que devuelve un elemento de la cola, y lo elimina de la misma. Devuelve NULL si esta vacia.
- sys_ipc_send(): verificamos que el proceso destinatario no este esperando un mensaje, agregamos una referencia al proceso remitente a la cola, y seteamos al proceso actual en ENV_NOT_RUNNABLE

    	if (!env->env_ipc_recving) {
            env->env_add_sender(curenv->env_id);
            curenv->env_status = ENV_NOT_RUNNABLE;
	    }
- sys_ipc_recv(): agregamos una llamada a env_get_sender() del env actual, que devuelve el primer elemento de la cola 'senders' y llamamos a sys_env_set_status para poner en estado ENV_RUNNABLE al proceso remitente

        envid_t sender_env_id;
        if ((sender_env_id = curenv.env_get_sender()) != NULL) {
            sys_env_set_status(sender_env_id, ENV_RUNNABLE);
        }

Con este diseño, varios procesos pueden enviar un mensaje a B, y quedar cada uno bloqueado mientras B no consuma su mensaje. Luego, cada proceso despertara teniendo en cuenta el orden en el que entraron a la cola, siguiendo el metodo FIFO. Por otro lado, si hay posibilidad de deadlock, ya que cada proceso remitente depende de que el proceso destinatario llame a ipc_recv() para volver a un estado ENV_RUNNABLE, entonces, si un proceso A llama a sys_ipc_send() para enviar un mensaje al proceso B, y luego el proceso B llama a sys_ipc_send() para enviar un mensaje al proceso A, ambos procesos estaran a la expectativa de que el otro llame a sys_ipc_recv() para volver a un estado ENV_RUNNABLE, lo cual no sera posible.