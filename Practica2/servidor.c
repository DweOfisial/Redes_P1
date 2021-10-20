#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>


#define MSG_SIZE 250
#define MAX_CLIENTS 50


/*
 * El servidor ofrece el servicio de un chat
 */

void manejador(int signum);
void salirCliente(int socket, fd_set * readfds, int * numClientes, int arrayClientes[]);
void escribirFichero(char *buff);
void operaciones();
void verFraseO(char fraseO[]);


int main ( )
{
  
	/*---------------------------------------------------- 
		Descriptor del socket y buffer de datos                
	-----------------------------------------------------*/
	int sd, new_sd;
	struct sockaddr_in sockname, from;
	char buffer[MSG_SIZE];
	socklen_t from_len;
    fd_set readfds, auxfds;
   	int salida;
   	int arrayClientes[MAX_CLIENTS];
    int numClientes = 0;
   	 //contadores
    int i,j,k;
	int recibidos;
   	char identificador[MSG_SIZE];
    char frase[]="SI TE CAES AL SUELO NO TE LEVANTES";
    char fraseOculta[]="-- -- ---- -- ----- -- -- --------";
    int on, ret;

    /*typedef struct cliente{
        char * usr;
        char * pass;
        int puntuacion;
        int iniciado;
    };*/
    
	/* --------------------------------------------------
		Se abre el socket 
	---------------------------------------------------*/
  	sd = socket (AF_INET, SOCK_STREAM, 0);
	if (sd == -1)
	{
		perror("No se puede abrir el socket cliente\n");
    		exit (1);	
	}
    
    	// Activaremos una propiedad del socket para permitir· que otros
    	// sockets puedan reutilizar cualquier puerto al que nos enlacemos.
    	// Esto permite· en protocolos como el TCP, poder ejecutar un
    	// mismo programa varias veces seguidas y enlazarlo siempre al
   	    // mismo puerto. De lo contrario habrÌa que esperar a que el puerto
    	// quedase disponible (TIME_WAIT en el caso de TCP)
    	on=1;
    	ret = setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));



	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(2050);
	sockname.sin_addr.s_addr =  INADDR_ANY;

	if (bind (sd, (struct sockaddr *) &sockname, sizeof (sockname)) == -1)
	{
		perror("Error en la operación bind");
		exit(1);
	}
	

   	/*---------------------------------------------------------------------
		Del las peticiones que vamos a aceptar sólo necesitamos el 
		tamaño de su estructura, el resto de información (familia, puerto, 
		ip), nos la proporcionará el método que recibe las peticiones.
   	----------------------------------------------------------------------*/
	from_len = sizeof (from);


	if(listen(sd,1) == -1){
		perror("Error en la operación de listen");
		exit(1);
	}
    
	//Inicializar los conjuntos fd_set
    	FD_ZERO(&readfds);
    	FD_ZERO(&auxfds);
    	FD_SET(sd,&readfds);
    	FD_SET(0,&readfds);
    
   	
    	//Capturamos la señal SIGINT (Ctrl+c)
    	signal(SIGINT,manejador);
    
	/*-----------------------------------------------------------------------
		El servidor acepta una petición
	------------------------------------------------------------------------ */
		while(1){
            
            //Esperamos recibir mensajes de los clientes (nuevas conexiones o mensajes de los clientes ya conectados)
            
            auxfds = readfds;
            
            salida = select(FD_SETSIZE,&auxfds,NULL,NULL,NULL);
            
            if(salida > 0){
                
                
                for(i=0; i<FD_SETSIZE; i++){
                    
                    //Buscamos el socket por el que se ha establecido la comunicación
                    if(FD_ISSET(i, &auxfds)) {
                        
                        if( i == sd){
                            
                            if((new_sd = accept(sd, (struct sockaddr *)&from, &from_len)) == -1){
                                perror("Error aceptando peticiones");
                            }
                            else
                            {
                                if(numClientes < MAX_CLIENTS){
                                    arrayClientes[numClientes] = new_sd;
                                    numClientes++;
                                    FD_SET(new_sd,&readfds);
                                
                                    strcpy(buffer, "Bienvenido al chat\n");

                                    send(new_sd,buffer,sizeof(buffer),0);
                                
                                    for(j=0; j<(numClientes-1);j++){
                                    
                                        bzero(buffer,sizeof(buffer));
                                        sprintf(buffer, "Nuevo Cliente conectado: %d\n",new_sd);
                                        send(arrayClientes[j],buffer,sizeof(buffer),0);
                                    }
                                }
                                else
                                {
                                    bzero(buffer,sizeof(buffer));
                                    strcpy(buffer,"Demasiados clientes conectados\n");
                                    send(new_sd,buffer,sizeof(buffer),0);
                                    close(new_sd);
                                }
                                
                            }
                            
                            
                        }
                        else if (i == 0){
                            //Se ha introducido información de teclado
                            bzero(buffer, sizeof(buffer));
                            fgets(buffer, sizeof(buffer),stdin);
                            
                            //Controlar si se ha introducido "SALIR", cerrando todos los sockets y finalmente saliendo del servidor. (implementar)
                            if(strcmp(buffer,"SALIR\n") == 0){
                             
                                for (j = 0; j < numClientes; j++){
						            bzero(buffer, sizeof(buffer));
						            strcpy(buffer,"Desconexión servidor\n"); 
                                    send(arrayClientes[j],buffer , sizeof(buffer),0);
                                    close(arrayClientes[j]);
                                    FD_CLR(arrayClientes[j],&readfds);
                                }

                                close(sd);
                                exit(-1);
                                
                                
                            }
                            //Mensajes que se quieran mandar a los clientes (implementar)
                            
                        } 
                        else{
                            bzero(buffer,sizeof(buffer));                        
                            recibidos = recv(i,buffer,sizeof(buffer),0);
                            
                            if(recibidos > 0){
                                if(strcmp(buffer,"SALIR\n") == 0){
                                    salirCliente(i,&readfds,&numClientes,arrayClientes);
                                }else if(strcmp(strtok(buffer," "),"USUARIO") == 0){
                                    printf("usuario\n");
                                    //llamar pa comprobar k existes
                                }else if(strcmp(strtok(buffer," "),"PASSWORD") == 0){
                                    printf("pass\n");
                                    //llmar pa comprobar k existe
                                }else if(strcmp(strtok(buffer," "),"REGISTER") == 0){
                                    printf("registrase\n");
                                    escribirFichero(buffer+12);
                                    //guardar en el fichero las cosas k le pasamo    
                                }else if(strcmp(buffer,"INICIAR_PARTIDA\n") == 0){
                                    printf("Empieza la parida\n");
                                    printf("%s",fraseOculta);



                                    //inisia ka partida  
                                }else if(strcmp(strtok(buffer," "),"CONSONANTE") == 0){
                                    //char letra[1]=buffer+10;
                                    //if(strlen(buffer)==13){
                                        //printf("Al menos entra?\n");
                                        //printf("%ld\n",strlen(buffer));
                                        //printf("%ld\n",strlen(frase));
                                        char aux[2];
                                        sprintf(aux,"%s",buffer+11);
                                        printf("%s",aux);
                                        printf("%s\n",fraseOculta);
                                        //char *letra=strtok(buffer," ");
                                        char *p;
                                        p=frase;
                                        printf("%s/n",*p);
                                        int cont =1;                     
                                        while (*p != '\0') {
                                            printf("as\n");
                                            if (*p ==  aux){
                                                printf("hay un %s\n",aux);
                                            }
                                            p++;
                                        }

                                        if(cont==0){
                                            printf("La letra %s no esta en la frase",buffer+11);
                                        }
                                        printf("%s\n",fraseOculta);
                                    //}else{
                                      //  printf("Formato incorrecto\n");
                                    //}

                                }else if(strcmp(strtok(buffer," "),"VOCAL") == 0){
                                    printf("vocal\n");    

                                }else if(strcmp(strtok(buffer," "),"RESOLVER") == 0){
                                    printf("resolver\n");   
                                     
                                }else if(strcmp(buffer,"PUNTUACION\n") == 0){
                                    printf("puntuacion\n");    
                                }else if(strcmp(buffer,"Operaciones\n") == 0){
                                    operaciones();
                                }

                                for(j=0; j<numClientes; j++){
                                    if(arrayClientes[j] != i){
                                        send(arrayClientes[j],buffer,sizeof(buffer),0);
                                    }
                                }

                            }
                            //Si el cliente introdujo ctrl+c
                            if(recibidos == 0)
                            {
                                printf("El socket %d, ha introducido ctrl+c\n", i);
                                //Eliminar ese socket
                                salirCliente(i,&readfds,&numClientes,arrayClientes);
                            }
                        }
                    }
                }
            }
		}

	close(sd);
	return 0;
	
}

void salirCliente(int socket, fd_set * readfds, int * numClientes, int arrayClientes[]){
  
    char buffer[250];
    int j;
    
    close(socket);
    FD_CLR(socket,readfds);
    
    //Re-estructurar el array de clientes
    for (j = 0; j < (*numClientes) - 1; j++)
        if (arrayClientes[j] == socket)
            break;
    for (; j < (*numClientes) - 1; j++)
        (arrayClientes[j] = arrayClientes[j+1]);
    
    (*numClientes)--;
    
    bzero(buffer,sizeof(buffer));
    sprintf(buffer,"Desconexión del cliente: %d\n",socket);
    
    for(j=0; j<(*numClientes); j++)
        if(arrayClientes[j] != socket)
            send(arrayClientes[j],buffer,sizeof(buffer),0);


}

void manejador (int signum){
    printf("\nSe ha recibido la señal sigint\n");
    signal(SIGINT,manejador);
    exit(EXIT_FAILURE);

    //Implementar lo que se desee realizar cuando ocurra la excepción de ctrl+c en el servidor
}

void escribirFichero(char * buff){
    FILE * f;
    if((f=fopen("registro.txt","a"))==NULL){
        printf("Error al abrir el fichero\n");
    }
    int cont=1;
    for(int i=0; i<strlen(buff);i++){
        if(buff[i]=='-' && buff[i+1]=='p'){
            break;            
        }
        cont++;
    }

    char * pass=buff+cont+2;
    char * usr;
    usr=strtok(buff," -");

    char * linea;
    linea=strcat(usr,",");
    linea=strcat(linea,pass);
    fputs(linea,f);
    fclose(f);
}

void operaciones(){
    printf("Operaciones disponibles:\n");
    printf("Usuario usr\n");
    printf("Password pass\n");
    printf("REGISTER -u usr -p pass\n");
    printf("INICIAR_PARTIDA\n");
    printf("CONSONANTE letra\n");
    printf("VOCAL letra\n");
    printf("RESOLVER frase\n");
    printf("Puntuacion\n");
    printf("SALIR\n");
}

void verFraseO(char fraseO[]){
    printf("%s",fraseO);
}