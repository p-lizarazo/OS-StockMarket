#include "lib_operaciones.h"


void construct(struct Lib* libro){
    libro->sizeCompra=0;
    libro->sizeVenta=0;
}


void insertarRegistro(struct Lib* libro, char actividad, char* empresa, int numero, int dinero, char* broker){
    if(actividad=='C' || actividad=='c'){
        struct registro* temp = malloc(sizeof(*temp));
        temp->actividad=actividad;
        strcpy(temp->empresa,empresa);
        strcpy(temp->broker,broker);
        temp->numero=numero;
        temp->dinero=dinero;

        int i,min,p=-1;
        min=1e9;
        for(i=0;i<libro->sizeVenta;++i){
            if(strcmp(empresa,libro->regsVenta[i].empresa)==0){
                if(dinero >= libro->regsVenta[i].dinero && min > libro->regsVenta[i].dinero){
                    min = libro->regsVenta[i].dinero;
                    p = i;
                }
            }
        }
        //evaluar si encontro una empresa correspondiente con precio valido
        if(p!=-1){
            
            char dir1[256]="",dir2[256]="",arr1[256]="",arr2[256]="";  
            strcat(dir1,broker);
            strcat(dir1,"pipe");
            strcat(dir2,libro->regsVenta[p].broker);
            strcat(dir2,"pipe");
            int fd1,fd2;
            fd1 = open(dir1, O_WRONLY);
            fd2 = open(dir2, O_WRONLY);
          
            if(numero>libro->regsVenta[p].numero){

                // Se hace temp-> numero = numero-libro->regsVenta[p].numero y se vuelve a llamar la funcion recursivamente para ver si se pueden comprar mas o se agrega a la lista
                // Se notifica a libro->regsVenta[p].broker que se vendio todo a su precio por pipes, se borra la venta, se notifica tambien al comprador de la compra
                printf("Compra de %d de acciones del broker %s a %d$ \n",libro->regsVenta[p].numero,broker,libro->regsVenta[p].dinero);
                printf("Venta de %d de acciones del broker %s a %d$ \n",libro->regsVenta[p].numero, libro->regsVenta[p].broker, libro->regsVenta[p].dinero);
                int j,resta;
                resta=numero-libro->regsVenta[p].numero;
                
                // Compra,precio venta ,precio compra,empresa
                snprintf(arr1, sizeof(arr1), "%c,%d,%d,%d,%s;",'C',libro->regsVenta[p].dinero,dinero,libro->regsVenta[p].numero,empresa);
                // Compra,precio venta,empresa
                snprintf(arr2, sizeof(arr2), "%c,%d,%d,%s;",'V',libro->regsVenta[p].dinero,libro->regsVenta[p].numero,empresa);
                write(fd1, arr1, strlen(arr1)+1);
                write(fd2, arr2, strlen(arr2)+1);
              	//borrar venta
                libro->sizeVenta--;
                for(j=p;j<libro->sizeVenta;j++){
                    libro->regsVenta[j]=libro->regsVenta[j+1];
                }
                insertarRegistro(libro,actividad,empresa,resta,dinero,broker);
            } else if(numero==libro->regsVenta[p].numero){
                // se borra las ventas y se notifica a ambos de las transacciones
                printf("Compra de %d de acciones del broker %s a %d$ \n",libro->regsVenta[p].numero,broker,dinero);
                printf("Venta de %d de acciones del broker %s a %d$ \n",libro->regsVenta[p].numero, libro->regsVenta[p].broker,dinero);
                
		
                snprintf(arr1, sizeof(arr1), "%c,%d,%d,%d,%s;",'C',libro->regsVenta[p].dinero,dinero,numero,empresa);
                snprintf(arr2, sizeof(arr2), "%c,%d,%d,%s;",'V',libro->regsVenta[p].dinero,numero,empresa);
                write(fd1, arr1, strlen(arr1)+1);
                write(fd2, arr2, strlen(arr2)+1);

                libro->sizeVenta--;
                int j;
                for(j=p;j<libro->sizeVenta;j++){
                    libro->regsVenta[j]=libro->regsVenta[j+1];
                }
            } else {

                snprintf(arr1, sizeof(arr1), "%c,%d,%d,%d,%s;",'C',libro->regsVenta[p].dinero,dinero,numero,empresa);
                snprintf(arr2, sizeof(arr2), "%c,%d,%d,%s;",'V',libro->regsVenta[p].dinero,numero,empresa);
                write(fd1, arr1, strlen(arr1)+1);
                write(fd2, arr2, strlen(arr2)+1);
                // Si las que compra son menos que las que venden, entonces se notifica de la compra y venta a ambos y se acualiza el numero restante de las ventas
                libro->regsVenta[p].numero-=numero;
                printf("Compra de %d de acciones del broker %s a %d$ \n",numero,broker,libro->regsVenta[p].dinero);
                printf("Venta de %d de acciones del broker %s a %d$ \n",numero, libro->regsVenta[p].broker,libro->regsVenta[p].dinero);
            }
            free(temp);
	close(fd1);
            close(fd2);
        } else {
            libro->regsCompra[libro->sizeCompra]=*(temp);
            libro->sizeCompra++;
        }

// MARCADO...
    } else {
        struct registro* temp = malloc(sizeof(*temp));
        temp->actividad=actividad;
        temp->numero=numero;
        temp->dinero=dinero;
        strcpy(temp->empresa,empresa);
        strcpy(temp->broker,broker);


        int i,max,p=-1;
        max=0;
        for(i=0;i<libro->sizeCompra;++i){
            if(strcmp(empresa,libro->regsCompra[i].empresa)==0){
                if(dinero <= libro->regsCompra[i].dinero && max < libro->regsCompra[i].dinero){
                    max = libro->regsCompra[i].dinero;
                    p = i;
                }
            }
        }
        //evaluar si encontro una empresa correspondiente con precio valido
        if(p!=-1){
            
            char dir1[256]="",dir2[256]="",arr1[256]="",arr2[256]="";
            strcat(dir1,broker);
            strcat(dir1,"pipe");
            strcat(dir2,libro->regsCompra[p].broker);
            strcat(dir2,"pipe");
            int fd1,fd2;
            fd1 = open(dir2, O_WRONLY);
            fd2 = open(dir1, O_WRONLY);

            if(numero > libro->regsCompra[p].numero){
                // Vendo mas que las que me compran
                printf("Compra de %d de acciones del broker %s a %d$ \n",libro->regsCompra[p].numero,libro->regsCompra[p].broker,dinero);
                printf("Venta de %d de acciones del broker %s a %d$ \n",libro->regsCompra[p].numero,broker,dinero);
                
                snprintf(arr1, sizeof(arr1), "%c,%d,%d,%d,%s;",'C',dinero,libro->regsCompra[p].dinero,libro->regsCompra[p].numero,empresa);
                snprintf(arr2, sizeof(arr2), "%c,%d,%d,%s;",'V',dinero,libro->regsCompra[p].numero,empresa);
                write(fd1, arr1, strlen(arr1)+1);
                write(fd2, arr2, strlen(arr2)+1);
		
                //borrar compra
                int j,resta;
                resta=numero-libro->regsCompra[p].numero;
                libro->sizeCompra--;
                for(j=p;j<libro->sizeCompra;j++){
                    libro->regsCompra[j]=libro->regsCompra[j+1];
                }
                insertarRegistro(libro,actividad,empresa,resta,dinero,broker);
            } else if(numero==libro->regsCompra[p].numero){
                // se borra las ventas y se notifica a ambos de las transacciones
                printf("Compra de %d de acciones del broker %s a %d$ \n",numero,libro->regsCompra[p].broker,dinero);
                printf("Venta de %d de acciones del broker %s a %d$ \n",numero, broker,dinero);
              
                snprintf(arr1, sizeof(arr1), "%c,%d,%d,%d,%s;",'C',dinero,libro->regsCompra[p].dinero,libro->regsCompra[p].numero,empresa);
                snprintf(arr2, sizeof(arr2), "%c,%d,%d,%s;",'V',dinero,libro->regsCompra[p].numero,empresa);
                write(fd1, arr1, strlen(arr1)+1);
                write(fd2, arr2, strlen(arr2)+1);
                libro->sizeCompra--;
                int j;
                for(j=p;j<libro->sizeCompra;j++){
                    libro->regsCompra[j]=libro->regsCompra[j+1];
                }
            } else {
                // Si las ventas son menos que compras
                
                snprintf(arr1, sizeof(arr1), "%c,%d,%d,%d,%s;",'C',dinero,libro->regsCompra[p].dinero,numero,empresa);
                snprintf(arr2, sizeof(arr2), "%c,%d,%d,%s;",'V',dinero,numero,empresa);
                write(fd1, arr1, strlen(arr1)+1);
                write(fd2, arr2, strlen(arr2)+1);
                libro->regsCompra[p].numero-=numero;
                printf("Compra de %d de acciones del broker %s a %d$ \n",numero,libro->regsCompra[p].broker,dinero);
                printf("Venta de %d de acciones del broker %s a %d$ \n",numero, broker,dinero);
            }
            free(temp);
            close(fd1);
            close(fd2);

        } else {
            libro->regsVenta[libro->sizeVenta]=*(temp);
            libro->sizeVenta++;
        }
    }
}

void consulta(struct Lib *libro,char actividad, char empresa[],char broker[]){
    char dir[256],arr[256];
    strcpy(dir,broker);
    strcat(dir, "pipe");
    int fd = open(dir, O_WRONLY);
    // enviar precio consultado y nombre de la empresa


    int i,min,p=-1;
    min=1e9;
    // min = precio minimo en el mercado
    for(i=0;i<libro->sizeVenta;++i){
        if(strcmp(empresa,libro->regsVenta[i].empresa)==0){
            if(min > libro->regsVenta[i].dinero){
                min = libro->regsVenta[i].dinero;
                p=i;
            }
        }
    }
    strcpy(arr,"");
    if(p==-1)
        min =0;
    snprintf(arr, sizeof(arr), "%c,%d,%s;",actividad,min,empresa);
    write(fd, arr, strlen(arr)+1);
    close(fd);
}

void consulta_aux(struct Lib *libro,char actividad, char empresa[],char broker[],int cant_aux){
  char dir[256],arr[256];
    strcpy(dir,broker);
    strcat(dir, "pipe");
    int fd = open(dir, O_WRONLY);
    // enviar precio consultado y nombre de la empresa
    int i,min,p=-1;
    min=1e9;
    // min = precio minimo en el mercado
    for(i=0;i<libro->sizeVenta;++i){
        if(strcmp(empresa,libro->regsVenta[i].empresa)==0){
            if(min > libro->regsVenta[i].dinero){
                min = libro->regsVenta[i].dinero;
                p=i;
            }
        }
    }
    strcpy(arr,"");
    if(p==-1)
        min =0;
    snprintf(arr, sizeof(arr), "%c,%d,%d,%s;",actividad,min,cant_aux,empresa);
    write(fd, arr, strlen(arr)+1);
    close(fd);
}






