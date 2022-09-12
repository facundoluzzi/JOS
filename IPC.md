TP3: IPC Challenge
============================

Implementacion
---------
---------
**¿Se pueden mantener ambas interfaces (tanto para user como para kernel), y solamente cambiar la implementación? ¿Por qué?**

Es posible mantener ambas interfaces, tanto para user como para kernel, y unicamente cambiar la implementacion debido a que estando en modo kernel tenemos todas las herramientas, y permisos posibles para hacerlo.

---------
**Como se pregunta en la tarea: sys_ipc_try_send: ¿Cuáles son los posibles deadlocks? ¿Cuáles son esos escenarios, y cómo se pueden solventar?**

- Un primer caso puede ser cuando un Env que llamaremos A intenta enviarle un mensaje a un Env que llamaremos B, y luego B decide enviarle un mensaje a A. En este caso, ambos Env se quedan bloqueados, esperando a que el otro reciba el mensaje para poder despertar. 
- En segundo lugar, si un Env envia un mensaje a un Env A o B (que surgen del primer caso) donde ambos se encuentran bloqueados, este se bloqueara y no podra ser desbloqueado hasta que A o B (al que enviemos el mensaje) reciba el mensaje, lo cual no sera posible.
- En tercer lugar, si por algun motivo todos los Env deciden enviar o recibir un mensaje, no quedara ningun Env disponible para:
    1. En caso de que todos decidan enviar un mensaje, no quedara ningun Env disponible para recibir estos mensajes.
    2. En caso de que todos decidan recibir un mensaje, no quedara ningun Env disponible para enviarles un mensaje.
- En cuarto lugar, cuando un Env envia un mensaje, y el Env receptor no solo no lo recibe, sino que tambien es destruido, este primer Env no podra ser desbloqueado de ninguna forma.

Luego, para solucionar varios de estos deadlocks podemos hacer lo siguiente:

1. Si enviamos un mensaje a un Env el cual se encuentra bloqueado esperando a que un Env receptor lo despierte, se podria cancelar el envio, o diferir para un futuro cercano en el cual el receptor ya se haya despertado. 
2. Si se detecta que la totalidad de los Envs estan bloqueados, ya sea porque enviaron un mensaje, o se quedaron escuchando, se podria forzar un desbloqueo masivo.
3. Cada vez que se destruye un Env, despertar a todos los Envs que hayan enviado un mensaje al mismo anteriormente.  


---------
**¿Cuáles son los cambios que se necesitan hacer en el struct Env?**

Agregue una referencia al Env al que deseamos enviar un mensaje, el mensaje, los permisos, y en caso de haber enviado una pagina de memoria, tambien la almacenamos.

	envid_t env_ipc_pending_sender;
	uint32_t env_ipc_pending_value;
	struct PageInfo *env_ipc_pending_page;
	int env_ipc_pending_perm;

---------
**¿Es necesario alguna estructura de datos extra?**

Si bien en un principio pense en agregar una cola, o una lista simple enlazada, y almacenar en ella una referencia a cada Env que intento enviarle un mensaje, al final decidi ir por el camino que me parecio mas sencillo, sin implementar ningun tipo de estructura de datos. 

---------
**Tener en cuenta las responsabilidades de cada syscall, es decir, ¿cómo dividirlas?**

- ipc_recv() tiene las siguientes responsabilidades:
    1. Cuando un receptor llama a ipc_recv(), si existe algun Env que haya intentado enviarle un mensaje anteriormente, entonces se encarga de recibir su mensaje, y despertarlo.
    2. Cuando un receptor llama a ipc_recv(), si no existe ningun Env que haya intentado enviarle un mensaje anteriormente, entonces se encarga de marcar al Env en estado recving, y NOT RUNNABLE, hasta que otro Env le envie un mensaje. 

- ipc_send() tiene las siguientes responsabilidades:
    1. Si el destinatario/receptor esta en estado recving, se encarga de enviarle el mensaje, y marcarlo en estado RUNNABLE.
    2. Si el destinatario/receptor no esta en estado recving, entonces se encarga de almacenar el mensaje en el Env emisor, y marcarlo en estado NOT RUNNABLE, hasta que el Env receptor reciba su mensaje.

---------

Analisis
---------
**¿Qué implementación es más efeciente en cuanto al uso de los recursos del sistema? (por ejemplo, cantidad de context switch realizados)**

Citando a la parte 4 del trabajo practico, seccion sys_ipc_try_send

    (...) desde userspace se hace necesario llamar a sys_ipc_try_send() en un ciclo, hasta que tenga éxito. Sin embargo, es más eficiente (y por tanto deseable) que el kernel marque al proceso como not runnable, ya que no se necesitaría el ciclo en la biblioteca estándar de JOS.

Entonces, podemos decir que nuestra implementacion es mas eficiente, debido a que se evita el consumo innecesario de recursos por parte del Env emisor, ya que anteriormente se intentaba enviar el mensaje dentro de un loop que solo cortaba cuando el envio era exitoso. 


Con respecto a la cantidad de context switchs realizados, la nueva implementacion es mas eficiente, debido a que en la anterior todo el tiempo que le toca correr al proceso, este lo va a malgastar intentando enviar el mensaje. En cambio, en la nueva solo intenta realizar un envio y hace un context switch inmediatamente.

---------
**¿Cuáles fueron las nuevas estructuras de datos utilizadas? ¿Por qué? ¿Qué otras opciones contemplaron?**

En mi implementacion no utilice ninguna estructura de datos. Aun asi, cada vez que se llama a ipc_recv, esta syscall recorre todos los Env buscando el primero que haya intentado enviarle un mensaje. En cambio, si utilizaria una estructura de datos, simplemente nos fijamos si la lista/cola esta vacia, y si no, tomamos el primer elemento (o el que decidamos elegir, no incorpore ningun tipo de prioridad en mi implementacion), siendo esta ultima implementacion mas eficiente en cuanto a complejidad temporal. 

Contemple la opcion de utilizar una lista simple enlazada, y una cola, siguiendo la heuristica FIFO, que resulta mas eficiente en el sentido que el receptor pueda recibir los mensajes a medida que estos son enviados (hablando de prioridades): el primero en enviarse, sera el primero en recibirse.

---------
**¿Existe algún escenario o caso de uso en el cual se desee tener una implementación no bloqueante de alguna de las dos syscalls? Ejemplificar, y de ser posible implementar programas de usuario que lo muestren.**

Podemos pensar por ejemplo, en un sistema de channels. En este, se implementaria la syscall ipc_send de modo no bloqueante, ya que a nosotros unicamente nos interesa enviar el mensaje, abstrayendonos de lo que pasa cuando el receptor lo reciba. Por lo que no sera necesario ninguna 'confirmacion' de que el receptor recibio el mensaje. En este caso, es importante no quedarnos bloqueados, ya que del otro lado se encuentra alguien escuchando por nuestros mensajes para detonar alguna accion, por lo que es preferible antes que esto, perder un mensaje y continuar con el siguiente. 

---------