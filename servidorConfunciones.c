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
#include <stdbool.h>

#define MSG_SIZE 250
#define MAX_CLIENTS 50


/*
 * El servidor ofrece el servicio de un chat
 */
void registrarse(char *buffer, int i);
void usuario(char *buffer, int i);
void pass(char *buffer, int i);

void manejador(int signum);
void salirCliente(int socket, fd_set * readfds, int * numClientes, int arrayClientes[]);
void escribirFichero(char * usr, char * passw);
void operaciones();
void verFraseO(char fraseO[]);
bool existeUsuario(char * usr);
bool existePass(char * usr, char * pass);
void manejaFunciones(int id, char *buffer);
struct cliente{
    int id;
    int puntuacion;
    int estado;
    //char * nombre;
};
/*-1= no ha iniciado sesion
0= ha iniciado sesion pero no ta en partida
1= esta en partida*/
struct cliente  vCliente[MAX_CLIENTS];
char *frase="SI TE CAES AL SUELO NO TE LEVANTES";
char *fraseOculta="-- -- ---- -- ----- -- -- --------";

char * nombreUsr;


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

    int on, ret;
    
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
                                    vCliente[new_sd].id=new_sd;
                                    vCliente[new_sd].puntuacion=0;
                                    vCliente[new_sd].estado=-1;
                                    //vCliente[new_sd].nombre="";


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
                            char *token = strtok(buffer, " ");

                            if(strcmp(buffer,"SALIR\n") == 0){
                                salirCliente(i,&readfds,&numClientes,arrayClientes);
                            }else{
                                if(strcmp(token,"USUARIO") == 0){
                                    usuario(token,i);
                                    //llamar pa comprobar k existes
                                }
                                else if(strcmp(token,"PASSWORD") == 0){
                                    
                                    pass(token,i);
                                    
                                    //llmar pa comprobar k existe
                                }
                                else if(strcmp(token,"REGISTER") == 0){
                                    registrarse(token,i);
                                    
                                }
                                else if(strcmp(buffer,"INICIAR_PARTIDA\n" ) == 0 || vCliente[i].estado==1){
                                    if(vCliente[i].estado!=1){
                                        printf("Empieza la parida\n");
                                        printf("%s",fraseOculta);
                                        //inisia ka partida  
                                    }
                                    else if(strcmp(token,"CONSONANTE") == 0){

                                    }
                                    else if(strcmp(token,"VOCAL") == 0){
                                        printf("vocal\n");    

                                    }
                                    else if(strcmp(token,"RESOLVER") == 0){
                                        printf("resolver\n");   
                                        
                                    }
                                    else if(strcmp(buffer,"PUNTUACION\n") == 0){
                                        printf("puntuacion\n");    
                                    }
                                    

                                }
                                else if(strcmp(buffer,"Operaciones\n") == 0){
                                    operaciones();
                                }
                                printf("asdibjnjsnnasuodknasdoknasdj osadas\n");
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

void manejaFunciones(int i, char *buffer){
    

}

void usuario(char * token, int i){
    token=strtok(NULL,"USUARIO ");
    token[strlen(token)] = '\0';                                        
    token[strcspn(token, "\n")] = 0;  
    if(existeUsuario(token)){
        nombreUsr=token;
        send(i,"Usuario correcto, introduzca la password con PASSWORS pass",59,0);
    }
    else{
        send(i,"Usuario no registrado",22,0);
    }
}

void pass(char * token, int i){
    printf("%d-%d\n",vCliente[i].id,i);
    printf("%s\n",nombreUsr);

    token=strtok(NULL,"PASSWORD ");
    token[strlen(token)] = '\0';
    token[strcspn(token, "\n")] = 0;

    if(existePass(nombreUsr,token)==true){
        send(i,"Sesion iniciada correctamente",30,0);
        vCliente[i].estado=0;
    }else{
        send(i,"Password incorrecta",20,0);
    }
}


void registrarse(char * token, int i){
    char * usr;
    char * pass;
    bool FC=false;
    for(int i=0; i<4;i++){
        token=strtok(NULL,"REGISTER ");
        token[strlen(token)] = '\0';
        if(i==0 && strcmp(token,"-u")==0 || i==2 && strcmp(token,"-p")==0){
            FC=true;
        }
        if(i==1){
            usr=token;
        }else if(i==3){
            pass=token;
        }
    }
    if(FC==true){
        if(!existeUsuario(usr)){
            escribirFichero(usr,pass);
            vCliente[i].estado=0;
        }else{
            send(i,"Error: Usuario en uso",22,0);
        }

    }
    else{
        send(i,"Formato incorrecto",19,0);
    }
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

void escribirFichero(char * usr, char * passw){

    //Abrimos el fichero "registros.txt" para añadir un usuario
    FILE * f;
    if((f=fopen("registro.txt","a"))==NULL){
        printf("Error al abrir el fichero\n");
    }

    //Creamos la cadena "usr,passw"
    char * credenciales=strcat(usr,",");
    credenciales=strcat(credenciales,passw);

    //Escribimos los datos del nuevo usuario
    fputs(credenciales,f);

    fclose(f);
}

    //Comprobar si un usuario ya está registrado en el sistema.
    //Devuelve false si el usuario NO está registrado.
bool existeUsuario(char * usr){
    bool encontrado=false;
    char * line;
    //Abrimos el fichero para leer
    FILE *f;
    if((f=fopen("registro.txt","a+"))==NULL){
        printf("Error al abrir el fichero\n");
    }
    while (fgets(line, sizeof(line), f)) {
        char * token = strtok(line, ",");
        if(strcmp(usr,token)==0){
            encontrado=true;
        } 
    }

    fclose(f);

    return encontrado;
}

//devuelve false si la password no coincide
bool existePass(char * usr, char * pass){
    bool encontrado=false;
    char * line;
    //Abrimos el fichero para leer
    FILE *f;
    if((f=fopen("registro.txt","r"))==NULL){
        printf("Error al abrir el fichero\n");
    }
    usr[strcspn(usr, "\n")] = 0;
    char * UC;
    UC=strcat(usr,",");
    printf("%s\n",UC);
    while (fgets(line, sizeof(line), f)) {
        char *token = strtok(line, ",");
        if(strcmp(usr,token )==0){
            printf("segundo if\n");
            token=strtok(NULL,UC);
            token[strlen(token)] = '\0';
            printf("%s\n",token);
            if(strcmp(pass,token)){
                encontrado=true;
            }
        } 
    }

    fclose(f);

    return encontrado;
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
