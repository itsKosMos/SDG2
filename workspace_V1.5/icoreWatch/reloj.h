/*
 * reloj.h
 *
 *  Created on: 7 de feb. de 2022
 *      Author: Ã�lvaro MartÃ­nez Guerrero y NicolÃ¡s LÃ³pez GomÃ©z
 */

#ifndef RELOJ_H_
#define RELOJ_H_

//------------------------------------------------------
// INCLUDES
#include "systemConfig.h"
#include "util.h"
#include "fsm.h"


// DEFINES Y ENUMS, FLAGS FSM
enum FSM_ESTADOS_RELOJ {
	WAIT_TIC,
};

#define FLAG_ACTUALIZA_RELOJ 0x01
#define FLAG_TIME_ACTUALIZADO 0x02
#define PRECISION_RELOJ_MS 100
#define MAX_MONTH 12
#define MIN_MONTH 1
#define MIN_DAY 1
#define MAX_DAY 31
#define MIN_YEAR 1970
#define MIN_HOUR 00
#define MAX_HOUR 23
#define MIN_MIN 00
#define MAX_MIN 59
#define MIN_SEG 00
#define MAX_SEG 59
#define TIME_FORMAT_12_H 12
#define TIME_FORMAT_24_H 24
#define DEFAULT_DAY 1
#define DEFAULT_MONTH 1
#define DEFAULT_YEAR 1970
#define DEFAULT_HOUR 00
#define DEFAULT_MIN 00
#define DEFAULT_SEC 00
#define DEFAULT_TIME_FORMAT 24


typedef struct {
	int dd;
	int MM;
	int yyyy;
} TipoCalendario;

typedef struct {
	int hh;
	int mm;
	int ss;
	int formato;
} TipoHora ;

typedef struct {
	int timestamp;
	TipoHora hora;
	TipoCalendario calendario;
	tmr_t* tmrTic;
} TipoReloj;

typedef struct {
	int flags;
} TipoRelojShared;

// DECLARACION DE ESTRUCTURAS
extern fsm_trans_t g_fsmTransReloj [];

// DECLARACION DE VARIABLES Y CONSTANTES
extern const int DIAS_MESES[2] [MAX_MONTH];

// FUNCIONES DE INCIALIZACION DE LAS VARIABLES

int ConfiguraInicializaReloj (TipoReloj *p_reloj);
void ResetReloj (TipoReloj *p_reloj);

// FUNCIONES PROPIAS

void ActualizaFecha(TipoCalendario *p_fecha);
void ActualizaHora (TipoHora *p_hora);
int CalculaDiasMes (int month, int year);
int EsBisiesto (int year);
TipoRelojShared GetRelojSharedVar();
int SetFecha (int nuevaFecha, TipoCalendario *p_fecha);
int SetFormato (int nuevoFormato, TipoHora *p_hora);
int SetHora (int nuevaHora, TipoHora *p_hora);
void SetRelojSharedVars (TipoRelojShared value);

//FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS

int CompruebaTic (fsm_t *p_this);

// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS

void ActualizaReloj (fsm_t *p_this);

// SUBRUTINAS DE ATENCION A LAS INTERRUPCIONES

void tmr_actualiza_reloj_isr (union sigval value);


#endif /* RELOJ_H_ */
