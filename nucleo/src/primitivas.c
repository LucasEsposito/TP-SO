/*
 * primitivas.c
 *
 *  Created on: 25/5/2016
 *      Author: utnso
 */
#include "nucleo.h"

void recibirWait(int cliente){
	char* semid = leerLargoYMensaje(clientes[cliente].socket);
	primitivaWait(cliente,semid);
	free(semid);
}

void recibirSignal(int cliente){
	char* semid = leerLargoYMensaje(clientes[cliente].socket);
	primitivaSignal(cliente,semid);
	free(semid);
}


void primitivaWait(int cliente, char* semid) {
	//char* semid = leerLargoYMensaje(clientes[cliente].socket);
	log_info(activeLogger,"Llego un wait para el semaforo %s",semid);
	t_semaforo* sem = (t_semaforo*) dictionary_get(tablaSEM, semid);
	if (sem->valor > 0)
		sem->valor--;
	else
		bloquearProcesoSem(clientes[cliente].pid, semid);
}
void primitivaSignal(int cliente, char* semid) {
	//char* semid = leerLargoYMensaje(clientes[cliente].socket);
	log_info(activeLogger,"Llego un signal para el semaforo %s",semid);
	t_semaforo* sem = (t_semaforo*) dictionary_get(tablaSEM, semid);
	if (!queue_is_empty(sem->cola))
		desbloquearProceso((int)queue_pop(sem->cola));
	else
	sem->valor++;
}
void asignarCompartida(int cliente) {
	char* compartida = leerLargoYMensaje(clientes[cliente].socket);
	char* valor = malloc(sizeof(int));
	read(clientes[cliente].socket, valor, sizeof(int));
	*(int*) dictionary_get(tablaGlobales, compartida) = char4ToInt(valor);
	free(compartida);
	free(valor);
}
void devolverCompartida(int cliente) {
	char* compartida = leerLargoYMensaje(clientes[cliente].socket);
	char* valor = intToChar4(*(int*) dictionary_get(tablaGlobales, compartida));
	send_w(clientes[cliente].socket, valor, sizeof(int));
	free(compartida);
	free(valor);
}
void imprimirVariable(int cliente) {

	int socketConsola = clientes[(obtenerProceso(clientes[cliente].pid))->consola].socket;
	char* serialValor = malloc( sizeof(ansisop_var_t));
	read(cliente, serialValor, sizeof(ansisop_var_t));
	enviarHeader(socketConsola, HeaderImprimirVariableConsola);
	send_w(socketConsola, serialValor, sizeof(ansisop_var_t));
	free(serialValor);
}
void imprimirTexto(int cliente) {
	int socketConsola = clientes[(obtenerProceso(clientes[cliente].pid))->consola].socket;
	char* texto = leerLargoYMensaje(clientes[cliente].socket);
	enviarHeader(socketConsola, HeaderImprimirTextoConsola);
	enviarLargoYString(socketConsola, texto);
	free(texto);
}
void entradaSalida(int cliente) {
	log_info(activeLogger,"Proceso pidio IO");
	char* serialIO = leerLargoYMensaje(clientes[cliente].socket);
	char* serialTiempo = malloc(sizeof(int));
	read(clientes[cliente].socket,serialTiempo,sizeof(int));
	bloquearProcesoIO(clientes[cliente].pid,serialIO,char4ToInt(serialTiempo));
	free(serialIO);
	free(serialTiempo);
}
