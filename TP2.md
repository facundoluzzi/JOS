TP2: Procesos de usuario
========================

env_alloc
---------

¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal)

generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);
El primer proceso creado arranca con un env_id seteado en 0, luego se le suma (1 << ENVGENSHIFT) que refiere a un shift a la izquierda de 12 bits, resultando en 0 + 4096. A esos 4096 le aplicamos el operador & con ~(NENV - 1) que significa lo siguiente: NENV tiene un valor de 1 << LOG2NENV, siendo el valor de LOG2NENV definido en 10, o sea 1 << 10: 1024 en decimal. En resumen: 4096 & 1024 = 4096.
Entonces, generation = 4096.
Luego, 
e->env_id = generation | (e - envs);
en esta linea se aplica el operador | con (e - envs), que toma un valor entre 0 y 1024, dependiendo de la posicion en el array de entorno que le corresponda al proceso.
Al primer proceso de le asigna el env_id 4096.
Los siguientes 4 procesos toman el env_id 4097, 4098, 4099, 4100
En hexadecimal los procesos toman los ID: 0x1000, 0x1001, 0x1002, 0x1003, 0x1004


Supongamos que al arrancar el kernel se lanzan NENV procesos a ejecución. A continuación, se destruye el proceso asociado a envs[630] y se lanza un proceso que cada segundo, muere y se vuelve a lanzar (se destruye, y se vuelve a crear). ¿Qué identificadores tendrán esos procesos en las primeras cinco ejecuciones?

En principio tenemos como vimos en la respuesta anterior el ID 0x1000 asociado al proceso n°0. Entonces, si actualmente tenemos 630 procesos, le sumamos 630 que en hexadecimal es 0x0276, y nos queda como resultado para el primer proceso 0x1276. 
A partir de eso, tomamos el valor inicial de generation como 0x1276, entonces al realizar las siguientes operaciones
generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);

e->env_id es igual a 0x1276 en el primer proceso, luego se le suma 0x1000, y se aplica el operador & con 0x0400, dando como resultado 

e->env_id = generation | (e - envs);

env_pop_tf
----------

...


gdb_hello
---------

...


kern_idt
--------

...


user_evilhello
--------------

...

