TP4: Sistema de archivos e intérprete de comandos
=================================================

caché de bloques
----------------
¿Qué es super->s_nblocks?

Citado textualmente del struct Super definido en JOS. Super hace referencia al super bloque que contiene la metadata del filesystem. Por otro lado, super->n_blocks representa la cantidad de bloques dentro del filesystem

```
Total number of blocks on disk
```


¿Dónde y cómo se configura este bloque especial?
Se setea dentro de la funcion opendisk en fs/fsformat.c. El super bloque representa al bloque numero 1. 

``` 
void opendisk(const char *name){
    ...
	alloc(BLKSIZE);
	super = alloc(BLKSIZE);
	super->s_magic = FS_MAGIC;
	super->s_nblocks = nblocks;
	super->s_root.f_type = FTYPE_DIR;
    ...
}
```