#ifndef LIB_OPERACIONES_H_
#define LIB_OPERACIONES_H_

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_STRING_SIZE 256
#define MAX_SIZE 1000
struct registro{
    char actividad;
    char broker [MAX_STRING_SIZE];
    char empresa[MAX_STRING_SIZE];
    int numero;
    int dinero;
};

struct Lib{
    struct registro regsCompra[MAX_SIZE];;
    struct registro regsVenta[MAX_SIZE];
    int sizeCompra;
    int sizeVenta;
};
void construct(struct Lib* libro);
void insertarRegistro(struct Lib *libro, char actividad, char empresa[], int numero, int dinero, char broker[]);
void to_print(struct registro* reg);
void consulta(struct Lib *libro,char actividad, char empresa[],char broker[]);
void consulta_aux(struct Lib *libro,char actividad, char empresa[],char broker[],int cant_aux);



#endif // LIB_OPERACIONES_H



