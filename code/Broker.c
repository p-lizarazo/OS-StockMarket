
// Proyecto 2da entrega Broker

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <errno.h>

struct argumentos
{
    int *monto;
    char archivo[256];
    char nombre[256];
    char recursos[256];
};

void *monitoreo(void *arg);
void leerLinea(int fd, char *str);
int stoi(char *string);
int stoi2(char *string, int i);
int candig(int n);
void pedirVenta(char *empresa, int cantidad, int precio, char *nombre);
void pedirCompra(char *empresa, int cantidad, int precio, char *nombre);
void pedirConsulta(char *empresa, char *nombre);
void pedirConsultaOculta(char *empresa, int cantidad, char *nombre);
int contarAcciones(char *archivo, char *empresa, int i);
void borrarAcciones(char *archivo, char *empresa, int n, int i);
void agregarAcciones(char *archivo, char *empresa, int n, int i);
void nuevaEmpresa(char *archivo, char *empresa, int n);
FILE* crearTemporal(char *archivo, int i);
void borrarEmpresa(char *archivo, char *empresa, int i);
FILE* localizarEmpresa(char *archivo, char *empresa, int i);
int hayEmpresa(char *archivo, char *empresa);


/*
    compra extitosa: empresa, noAcc, prec.
    venta exitosa: empresa, noAcc, prec.
    resultado consulta: empresa, prec.
*/

char DIR[256];
int fd;
sem_t semaforo;

void signalhandler(int n){
	close(fd);
	unlink(DIR);
	sem_destroy(&semaforo);
	exit(0);
}

int main(int argc, char *argv[])
{
    {
        if(argc != 5)
        {
            printf("ERROR DE ARGUMENTOS, LA FORMA CORRECTA ES:\n\tbroker  <nombre> <pipename> <recursos_iniciales> <monto>\n");
            exit(1);
        }
        char nombre[252] = "";
        char pipename[256] = "";
        char recursos[256] = "/";
        int monto = stoi(argv[4]);
        char empresa[5], operacion[10], linea[50];
        int cantidad, precio, i=0, cont, n=1, ban;
        struct argumentos args;
        pthread_t thread1;
        sem_init(&semaforo, 0, 1);
        strcpy(nombre, argv[1]);
        strcpy(pipename,argv[2]);
        strcpy(recursos,argv[3]);
        do {
            fd = open(pipename, O_WRONLY|O_NONBLOCK);
            if (fd ==-1)
                sleep(1);
        } while(fd  == -1);
        strcpy(DIR, nombre);
        strcat(DIR, "pipe");
        unlink(DIR);
        mkfifo(DIR, 0666);
        strcpy(args.archivo,DIR);
        strcpy(args.nombre,nombre);
        args.monto = &monto;
        strcpy(args.recursos,recursos);
        pthread_create(&thread1, NULL, monitoreo, (void *)&args);
        signal(SIGINT,signalhandler);
        while(n)
        {
            fgets(linea, 49, stdin);
            linea[strlen(linea)-1] = '\0';
            i=0;
            cont=0;
            while(linea[i]!='\0')
            {
                if(linea[i]==':')
                    cont++;
                ++i;
            }
            if(cont!=3)
            {
                printf("COMANDO NO VALIDO\n");
                continue;
            }
            else
            {
                i=0;
                strcpy(operacion,"");
                while(linea[i]!=':')
                {
                    if(linea[i]!=' ')
                    {
                        if(strlen(operacion)==8)
                        {
                            strcpy(operacion,"");
                            break;
                        }
                        operacion[strlen(operacion)+1] = '\0';
                        operacion[strlen(operacion)] = linea[i];
                    }
                    ++i;
                }
                ban=1;
                ++i;
                precio=cantidad=0;
                strcpy(empresa, "");
                while(linea[i]!=':')
                {
                    if(linea[i]!=' ')
                    {
                        if(strlen(empresa)==3)
                        {
                            printf("COMANDO NO VALIDO\n");
                            ban=0;
                            break;
                        }
                        empresa[strlen(empresa)+1]='\0';
                        empresa[strlen(empresa)]=linea[i];
                    }
                    ++i;
                }
                ++i;
                while(linea[i]!=':')
                {
                    if(linea[i]!=' ')
                    {
                        if(linea[i]<'0' || linea[i]>'9')
                        {
                            printf("COMANDO NO VALIDO\n");
                            ban=0;
                            break;
                        }
                        cantidad*=10;
                        cantidad+= linea[i]-'0';
                    }
                    ++i;
                }
                ++i;
                while(linea[i]!='\0')
                {
                    if(linea[i]!=' ')
                    {
                        if(linea[i]<'0' || linea[i]>'9')
                        {
                            printf("COMANDO NO VALIDO\n");
                            ban=0;
                            break;
                        }
                        precio*=10;
                        precio+= linea[i]-'0';
                    }
                    ++i;
                }
                if(ban==0)
                    continue;
            }
            if(strcmp(operacion,"compra")==0)
            {
                if(strlen(empresa)==0 || precio==0 || cantidad==0)
                {
                    printf("COMANDO NO VALIDO\n");
                    continue;
                }
                if(cantidad*precio <= monto)
                {
                    monto -= cantidad*precio;
                    pedirCompra(empresa, cantidad, precio, nombre);
                    printf("SOLICITUD DE COMPRA DE %d ACCIONES DE %s POR $%d.00 COP\n", cantidad, empresa, precio);
                    continue;
                }
                printf("FONDOS INSUFICIENTES\n");
            }
            else if(strcmp(operacion,"venta")==0)
            {
                if(strlen(empresa)!=0 && precio==0 && cantidad==0)
                {
                	sem_wait(&semaforo);
                	int aux = hayEmpresa(recursos, empresa);
                	if(aux==0)
                	{
                		printf("EMPRESA NO EXISTENTE\n");
                		sem_post(&semaforo);
                		continue;
                	}
									cantidad = contarAcciones(recursos, empresa, aux);
									if(cantidad <=0)
									{
										printf("NO PUEDES VENDER ACCIONES DE %s\n", empresa);
										sem_post(&semaforo);
										continue;
									}
									sem_post(&semaforo);
									pedirConsultaOculta(empresa, cantidad, nombre);
                }
                else if(strlen(empresa)!=0 && precio!=0 && cantidad!=0)
                {
                	sem_wait(&semaforo);
                	int aux = hayEmpresa(recursos, empresa);
                	if(aux==0)
                	{
                		printf("EMPRESA NO EXISTENTE\n");
                		sem_post(&semaforo);
                		continue;
                	}
                	int aux2 = contarAcciones(recursos, empresa, aux);
                	if(aux2-cantidad<0)
                	{
                		printf("NO HAY ACCIONES SUFICIENTES PARA VENDER\n");
                		sem_post(&semaforo);
                		continue;
                	}
                	if(aux2-cantidad==0)
                		borrarEmpresa(recursos, empresa, aux);
                	else
                		borrarAcciones(recursos, empresa, cantidad, aux);
									sem_post(&semaforo);
									pedirVenta(empresa, cantidad, precio, nombre);
									printf("%d ACCIONES DE %s PUESTAS A LA VENTA POR $%d.00 COP\n",cantidad, empresa, precio);
									continue;
                }
                else
                {
                    printf("COMANDO NO VALIDO\n");
                    continue;
                }
            }
            else if(strcmp(operacion,"consulta")==0)
            {
                if(strlen(empresa)==0 || precio!=0 || cantidad!=0)
                {
                    printf("COMANDO NO VALIDO\n");
                    continue;
                }
                pedirConsulta(empresa, nombre);
            }
            else if(strcmp(operacion,"monto")==0)
            {
                if(strlen(empresa)!=0 || precio!=0 || cantidad!=0)
                {
                    printf("COMANDO NO VALIDO\n");
                    continue;
                }
                printf("MONTO ACTUAL: $%d.00 COP\n", monto);
            }
            else
            {
                printf("COMANDO NO VALIDO\n");
            }
        }
    }
	close(fd);
	unlink(DIR);
	sem_destroy(&semaforo);
	exit(0);
}


int candig(int n)
{
    int res=0;
	if(n==0)
		return 1;
    while(n!=0)
    {
        res++;
        n/=10;
    }
    return res;
}

int stoi2(char *string, int i)
{
    int n=0;
    while(string[i]!=';' && string[i]!=',')
    {
        n*=10;
        n+= string[i++]-'0';
    }
    return n;
}

int stoi(char *string)
{
    int n=0, i=0;
    while(i<strlen(string))
    {
        n*=10;
        n+= string[i++]-'0';
    }
    return n;
}

void leerLinea(int fd2, char *str)
{
        read(fd2, str, 50);
}

void *monitoreo(void *arg)
{
    struct argumentos *args = arg;
    char *archivo = args->archivo, str[50], empresa[5];
    int *monto = args->monto, i;
    strcpy(archivo,args->archivo);
    int fd2 = open(archivo, O_RDONLY), cantidad, precio, precioant;
    while(1)
    {
        i=0;
        strcpy(str, "");
        strcpy(empresa, "");
    	leerLinea(fd2, str);
        cantidad=precio=precioant=0;
        if(str[0]=='R')
        {
            i=2;
            precio = stoi2(str, i);
            i+=candig(precio)+1;
            while(str[i]!=';')
            {
                empresa[strlen(empresa)+1]='\0';
                empresa[strlen(empresa)]=str[i++];
            }
			if(precio==0)
				printf("IMPOSIBLE DETERMINAR PRECIO DE MERCADO DE ACCIONES DE %s\n", empresa);
			else
            	printf("LAS ACCIONES DE %s TIENEN UN VALOR DE %d\n", empresa, precio);
            continue;
        }
        if(str[0]=='N')
        {
            i=2;
            precio = stoi2(str, i);
            i+=candig(precio)+1;
            cantidad = stoi2(str, i);
            i+=candig(cantidad)+1;
            while(str[i]!=';')
            {
                empresa[strlen(empresa)+1]='\0';
                empresa[strlen(empresa)]=str[i++];
            }
			if(precio==0)
			{
				printf("NO SE PUDO DETERMINAR EL PRECIO DE MERCADO DE %s\n", empresa);
				continue;
			}
				sem_wait(&semaforo);
					int aux = hayEmpresa(args->recursos, empresa);
					printf("%d ACCIONES DE %s PUESTAS A LA VENTA POR $%d.00 COP\n", cantidad, empresa, precio);
					borrarEmpresa(args->recursos, empresa, aux);
					sem_post(&semaforo);
          pedirVenta(empresa, cantidad, precio, args->nombre);
          continue;
        }
        if(str[0]=='C')
        {
            i=2;
            precio = stoi2(str, i);
            i+=candig(precio)+1;
            precioant = stoi2(str, i);
            i+=candig(precioant)+1;
            cantidad = stoi2(str, i);
            i+=candig(cantidad)+1;
            while(str[i]!=';')
            {
                empresa[strlen(empresa)+1]='\0';
                empresa[strlen(empresa)]=str[i++];
            }
        	sem_wait(&semaforo);
        	int aux = hayEmpresa(args->recursos, empresa);
        	if(aux==0)
        	{
        		nuevaEmpresa(args->recursos, empresa, cantidad);
        	}
        	else
        	{
        		agregarAcciones(args->recursos, empresa, cantidad, aux);
        	}
            sem_post(&semaforo);
            precioant -= precio;
            *monto += precioant*cantidad;
            printf("COMPRA EXITOSA - %d ACCIONES de %s POR $%d.00 COP\n", cantidad, empresa, precio);
            continue;
        }
        if(str[0]=='V')
        {
            i=2;
            precio = stoi2(str, i);
            i+=candig(precio)+1;
            cantidad = stoi2(str, i);
            i+=candig(cantidad)+1;
            while(str[i]!=';')
            {
                empresa[strlen(empresa)+1]='\0';
                empresa[strlen(empresa)]=str[i++];
            }
            *monto+=cantidad*precio;
            printf("VENTA EXITOSA - %d ACCIONES de %s POR $%d.00 COP\n", cantidad, empresa, precio);
            continue;
        }
    }
    close(fd2);
    pthread_exit(NULL);
}


void pedirVenta(char *empresa, int cantidad, int precio, char *nombre)
{
    char c[50];
    strcpy(c,"");
    sprintf(c, "V,%d,%d,%s,%s;", precio, cantidad, empresa, nombre);
    write(fd, c, strlen(c)+1);
}

void pedirCompra(char *empresa, int cantidad, int precio, char *nombre)
{
    char c[50];
    strcpy(c,"");
    sprintf(c, "C,%d,%d,%s,%s;", precio, cantidad, empresa, nombre);
    write(fd, c, strlen(c)+1);
}

void pedirConsulta(char *empresa, char *nombre)
{
    char c[50];
    strcpy(c,"");
    sprintf(c, "R,%s,%s;", empresa, nombre);
    write(fd, c, strlen(c)+1);
}

void pedirConsultaOculta(char *empresa, int cantidad, char *nombre)
{
    char c[50];
    strcpy(c,"");
    sprintf(c, "N,%d,%s,%s;",cantidad, empresa, nombre);
    write(fd, c, strlen(c)+1);
}


int contarAcciones(char *archivo, char *empresa, int i)
{
	FILE *file = localizarEmpresa(archivo, empresa, i);
	char emp[5];
	int n;
	fscanf(file, "%s %d", emp, &n);
	return n;
}

void borrarAcciones(char *archivo, char *empresa, int n, int i)
{
	char emp[5] = "";
	int cant = 0;
	FILE *file = localizarEmpresa(archivo, empresa, i);
	FILE *temp = crearTemporal(archivo, i);
	fscanf(file, "%s %d", emp, &cant);
	fprintf(temp, "%s %d\n", emp, cant-n);
	while(fscanf(file, "%s %d", emp, &cant)!=EOF)
	{
		fprintf(temp, "%s %d\n", emp, cant);
	}
	fclose(file);
	remove(archivo);
	fclose(temp);
	file = fopen(archivo,"w");
	temp = fopen("temp", "r");
	while(fscanf(temp, "%s %d", emp, &cant)!=EOF)
	{
		fprintf(file, "%s %d\n", emp, cant);
	}
	fclose(file);
	fclose(temp);
	remove("temp");
}

void agregarAcciones(char *archivo, char *empresa, int n, int i)
{
	char emp[5] = "";
	int cant = 0;
	FILE *file = localizarEmpresa(archivo, empresa, i);
	FILE *temp = crearTemporal(archivo, i);
	fscanf(file, "%s %d", emp, &cant);
	fprintf(temp, "%s %d\n", emp, cant+n);
	while(fscanf(file, "%s %d", emp, &cant)!=EOF)
	{
		fprintf(temp, "%s %d\n", emp, cant);
	}
	fclose(file);
	remove(archivo);
	fclose(temp);
	file = fopen(archivo,"w");
	temp = fopen("temp", "r");
	while(fscanf(temp, "%s %d", emp, &cant)!=EOF)
	{
		fprintf(file, "%s %d\n", emp, cant);
	}
	fclose(file);
	fclose(temp);
	remove("temp");
}

void nuevaEmpresa(char *archivo, char *empresa, int n)
{
	FILE *file = fopen(archivo, "a");
	fprintf(file, "%s %d\n", empresa, n);
	fclose(file);
}

FILE* crearTemporal(char *archivo, int i)
{
	FILE *temp = fopen("temp", "w+");
	FILE *file = fopen(archivo, "r");
	int cant;
	char emp[5];
	while(--i)
	{
		fscanf(file, "%s %d", emp, &cant);
		fprintf(temp, "%s %d\n", emp, cant);
	}
	return temp;
}


void borrarEmpresa(char *archivo, char *empresa, int i)
{
	char emp[5] = "";
	int cant = 0;
	FILE *file = localizarEmpresa(archivo, empresa, i);
	FILE *temp = crearTemporal(archivo, i);
	fscanf(file, "%s %d", emp, &cant);
	while(fscanf(file, "%s %d", emp, &cant)!=EOF)
	{
		fprintf(temp, "%s %d\n", emp, cant);
	}
	fclose(file);
	remove(archivo);
	fclose(temp);
	file = fopen(archivo,"w");
	temp = fopen("temp", "r");
	while(fscanf(temp, "%s %d", emp, &cant)!=EOF)
	{
		fprintf(file, "%s %d\n", emp, cant);
	}
	fclose(file);
	fclose(temp);
	remove("temp");
}

FILE* localizarEmpresa(char *archivo, char *empresa, int i)
{
	FILE *file = fopen(archivo, "r+");
	char temp[5] = "";
	int cant = 0;
	while(--i)
	{
		fscanf(file, "%s %d", temp, &cant);
	}
	return file;
}

int hayEmpresa(char *archivo, char *empresa)
{
	FILE *file = fopen(archivo, "r");
	if(file==NULL)
		return 0;
	char temp[5] = "";
	int cant = 0, i=0, p=0;
	while(fscanf(file, "%s %d", temp, &cant)!=EOF)
	{
		++i;
		if(strcmp(empresa, temp)==0)
		{
			p=i;
			break;
		}
	}
	fclose(file);
	return p;
}

