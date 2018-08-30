// Proyecto 2da entrega


#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "lib_operaciones.h"
char *DIR;

void signalhandler(int n){
	unlink(DIR);
	exit(1);
}

int main(int argc, char *argv[]){
    struct Lib libro;
    construct(&libro);
    if(argc != 2){
        printf("Usted no ingreso los parametros suficientes, la estructura es: ./nombredelprograma pipename\n");
        exit(-1);
    }
    DIR = argv[1];
    unlink(argv[1]);
    mkfifo(argv[1], 0666);
	signal(SIGINT,signalhandler);

    int fd1;
    char str1[256];

    while(1){
        char * pch;
        fd1 = open(argv[1],O_RDONLY);
        read(fd1, str1, 256);
        // Print the read string and close
        //printf("User1: %s\n", str1);
        pch = strtok (str1,",;");
        if(strcmp(pch,"C")==0 || strcmp(pch,"V")==0){
            // Formato ingresar Actividad,Precio, cantidad, empresa, broker;
            int precio,cantidad;
            char empresa[256],broker[256];
            char actividad = pch[0];
            pch = strtok (NULL, " ,;");
            precio = atoi(pch);
            pch = strtok (NULL, " ,;");
            cantidad= atoi(pch);
            pch = strtok (NULL, " ,;");
            strcpy(empresa,pch);
            pch = strtok (NULL, " ,;");
            strcpy(broker,pch);
           //printf("actividad %c precio: %d, cantidad %d, empresa: %s, broker %s,",actividad,precio,cantidad,empresa,broker);
            insertarRegistro(&libro,actividad,empresa,cantidad,precio,broker);
            
            
        } else if(strcmp(pch,"R")==0){
            char empresa[256],broker[256];
            char actividad = pch[0];
            pch = strtok (NULL, " ,;");
            strcpy(empresa,pch);
            pch = strtok (NULL, " ,;");
            strcpy(broker,pch);
            consulta(&libro,actividad,empresa,broker);

        } else if(strcmp(pch,"N")==0){
            char empresa[256],broker[256];
            int aux;
            char actividad = pch[0];
            pch = strtok (NULL, " ,;");
            aux = atoi(pch);
            pch = strtok (NULL, " ,;");
            strcpy(empresa,pch);
            pch = strtok (NULL, " ,;");
            strcpy(broker,pch);
            consulta_aux(&libro,actividad,empresa,broker,aux);
        }
        close(fd1);



    }
    unlink(argv[1]);

	return 0;
}

