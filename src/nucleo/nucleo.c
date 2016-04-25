/*
 * kernel.c
 *
 *  Created on: 16/4/2016
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "../otros/handshake.h"
#include "../otros/header.h"
#include "../otros/sockets/cliente-servidor.h"
#include "../otros/log.h"

#define PUERTO 8080

//typedef struct customConfig {
//	int puertoConsola;
//	int puertoCPU;
//	int quantum;
//	int queantum_sleep;
//	char** sem_ids;
//	int* semInit;
//	char** io_ids;
//	int* ioSleep;
//	char** sharedVars;
//} customConfig_t;

//struct customConfig_t config;
//t_config* configNucleo;

//void cargarCFG()
//{
//	configNucleo = config_create("archivoConfigNucleo");
//	config.puertoConsola = config_get_int_value(configNucleo,"PUERTO_PROG");
//	config.puertoCPU = config_get_int_value(configNucleo,"PUERTO_CPU");
//	config.quantum = config_get_int_value(configNucleo,"QUANTUM");
//	config.queantum_sleep = config_get_int_value(configNucleo,"QUANTUM_SLEEP");
//	config.sem_ids = config_get_array_value(configNucleo,"SEM_IDS");
//	config.semInit = config_get_array_value(configNucleo,"SEM_INIT");
//	config.io_ids = config_get_array_value(configNucleo,"IO_IDS");
//	config.ioSleep = config_get_array_value(configNucleo,"IO_SLEEP");
//	config.sharedVars = config_get_array_value(configNucleo,"SHARED_VARS");
//}

void procesarHeader(int cliente, char *header){
	// Segun el protocolo procesamos el header del mensaje recibido
	char* payload;
	int payload_size;
	log_debug(bgLogger,"Llego un mensaje con header %d\n",charToInt(header));

	switch(charToInt(header)) {

	case HeaderError:
		log_error(activeLogger,"Header de Error\n");
		quitarCliente(cliente);
		break;

	case HeaderHandshake:
		log_debug(bgLogger,"Llego un handshake\n");
		payload_size=1;
		payload = malloc(payload_size);
		read(socketCliente[cliente] , payload, payload_size);
		log_debug(bgLogger,"Llego un mensaje con payload %d\n",charToInt(payload));
		if ((charToInt(payload)==SOYCONSOLA) || (charToInt(payload)==SOYCPU)){
			log_debug(bgLogger,"Es un cliente apropiado! Respondiendo handshake\n");
			send(socketCliente[cliente], intToChar(SOYNUCLEO), 1, 0);
		}
		else {
			log_error(activeLogger,"No es un cliente apropiado! rechazada la conexion\n");
			log_warning(activeLogger,"Se quitará al cliente %d.",cliente);
			quitarCliente(cliente);
		}
		free(payload);
		break;

	case HeaderScript: /*A implementar*/ break; //TODO
	/* Agregar futuros casos */

	default:
		log_error(activeLogger,"Llego cualquier cosa.");
		log_error(activeLogger,"Llego el header numero %d y no hay una acción definida para él.",charToInt(header));
		log_warning(activeLogger,"Se quitará al cliente %d.",cliente);
		quitarCliente(cliente);
		break;
	}
}

struct timeval newEspera()
{
	struct timeval espera;
	espera.tv_sec = 2; 				//Segundos
	espera.tv_usec = 500000; 		//Microsegundos
	return espera;
}

int main(void) {

	int mayorDescriptor, i;
	struct timeval espera = newEspera(); 		// Periodo maximo de espera del select
	char header[1];

	crearLogs("Nucleo","Nucleo");
	configurarServidor(PUERTO);
	inicializarClientes();
	log_info(activeLogger,"Esperando conexiones ...");

	while(1){
		mayorDescriptor = incorporarSockets();
		select( mayorDescriptor + 1 , &socketsParaLectura , NULL , NULL , &espera);

		if (tieneLectura(socketNuevasConexiones))
			procesarNuevasConexiones();

		for (i = 0; i < getMaxClients(); i++){
			if (tieneLectura(socketCliente[i]))	{
				if (read( socketCliente[i] , header, 1) == 0)
					quitarCliente(i);
				else
				{
					log_debug(bgLogger,"LLEGO main %c\n",header);
					procesarHeader(i,header);
				}
			}
		}
	}

	destruirLogs();
	return EXIT_SUCCESS;
}
