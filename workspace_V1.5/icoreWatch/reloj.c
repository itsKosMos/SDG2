/*
 * reloj.c
 *
 *  Created on: 7 de feb. de 2022
 *      Author: Ã�lvaro MartÃ­nez Guerrero y NicolÃ¡s LÃ³pez GomÃ©z
 */

//------------------------------------------------------
// INCLUDES
#include "reloj.h"

fsm_trans_t g_fsmTransReloj[] = {{ WAIT_TIC, CompruebaTic, WAIT_TIC, ActualizaReloj}, {-1, NULL, -1, NULL}, };

const int DIAS_MESES[2] [MAX_MONTH] = {
		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, // ANO No Bisiesto
		{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, // ANO Bisiesto
};

static TipoRelojShared g_relojSharedVars;

void ResetReloj (TipoReloj *p_reloj){
	TipoCalendario calendario = {DEFAULT_DAY, DEFAULT_MONTH, DEFAULT_YEAR};
	p_reloj->calendario = calendario;
	TipoHora hora = {DEFAULT_HOUR, DEFAULT_MIN, DEFAULT_SEC, DEFAULT_TIME_FORMAT};
	p_reloj->hora = hora;
	p_reloj->timestamp = 0;
	piLock (RELOJ_KEY);
	g_relojSharedVars.flags = 0;
	piUnlock (RELOJ_KEY);

}

int ConfiguraInicializaReloj (TipoReloj *p_reloj){
	ResetReloj(p_reloj);
	p_reloj->tmrTic = tmr_new(tmr_actualiza_reloj_isr);
	tmr_startms_periodic(p_reloj->tmrTic, PRECISION_RELOJ_MS);
	return 0;
}


int CompruebaTic (fsm_t* p_this){
	piLock(RELOJ_KEY);
	int result;
	result = (g_relojSharedVars.flags & FLAG_ACTUALIZA_RELOJ);
	piUnlock (RELOJ_KEY);

	return result;
}


void ActualizaReloj (fsm_t* p_this){
	TipoReloj *p_miReloj = (TipoReloj*)(p_this->user_data);
	p_miReloj->timestamp += 1;
	ActualizaHora(&p_miReloj->hora);
	piLock (RELOJ_KEY);
	g_relojSharedVars.flags &= ~FLAG_ACTUALIZA_RELOJ;
	g_relojSharedVars.flags |= FLAG_TIME_ACTUALIZADO;
	piUnlock (RELOJ_KEY);

#if VERSION == 1
	piLock(SYSTEM_KEY);
	printf("Son las:  %d:%d:%d del %d/%d/%d\n", p_miReloj->hora.hh, p_miReloj->hora.mm,
			p_miReloj->hora.ss, p_miReloj->calendario.dd, p_miReloj->calendario.MM,
			p_miReloj->calendario.yyyy);
	piUnlock(SYSTEM_KEY);
	#endif

}

void tmr_actualiza_reloj_isr (union sigval value){
	piLock(RELOJ_KEY);
	g_relojSharedVars.flags |= FLAG_ACTUALIZA_RELOJ;
	piUnlock(RELOJ_KEY);
}

void ActualizaFecha (TipoCalendario *p_fecha){
	CalculaDiasMes(p_fecha->MM, p_fecha->yyyy);
	int dia_actual = p_fecha->dd + 1;
	int dia_mes_actual = DIAS_MESES[EsBisiesto(p_fecha->yyyy)][p_fecha->MM] + 1;
	int modulo_dia = dia_actual % dia_mes_actual;
	p_fecha->dd = MAX(1, modulo_dia);
	int modulo_mes;
	if (modulo_dia == 0)
	{
		int mes_actual = p_fecha->MM +1;
		modulo_mes = mes_actual % MAX_MONTH+1;
		p_fecha->MM = MAX(1, modulo_mes);
	}
	if ((modulo_dia == 0) && (modulo_mes == 0))
	{
		p_fecha->yyyy = p_fecha->yyyy+1;
	}
}

void ActualizaHora (TipoHora *p_hora){
	int segundos_act = p_hora->ss +1;
	int modulo_sec = segundos_act % 60;
	p_hora->ss = modulo_sec;
	if(modulo_sec == 0){
		p_hora->mm += 1;
	}
	if (modulo_sec == 0 && p_hora->mm == 0){
		int hora_act = p_hora->hh +1;
		int modulo_hora = hora_act % p_hora->formato;
		p_hora->hh = MAX(0, modulo_hora);
	}
}

int CalculaDiasMes (int month, int year){
	EsBisiesto(year);
	return DIAS_MESES[EsBisiesto(year)][month] + 1;
}

int EsBisiesto (int year){
	int modulo1 = year % 4;
	if (modulo1 == 0){
		int modulo2 = year % 100;
		if (modulo2 == 0){
			int modulo3 = year % 400;
			if (modulo3 == 0){
				return 1;
			} else {
				return 0;
			}
		} else {
			return 1;
		}
	} else {
		return 0;
	}
}

int SetHora (int horaInt, TipoHora *p_hora){
	if (horaInt >= 0){
		int numero_digitos = 0;
		int auxiliar = horaInt;
		int hora;
		int minutos;
		do{
			auxiliar = auxiliar / 10;
			numero_digitos++;
		} while (auxiliar !=0);
		if (numero_digitos <= 4){
			hora = (int)(horaInt/100);
			minutos = horaInt - (hora*100);
			p_hora->hh = MIN(hora, MAX_HOUR);
			p_hora->mm = MIN(minutos, MAX_MIN);
			p_hora->ss = 0;
			return 0;
		} else
			return 8;
	} else
		return 7;
}

TipoRelojShared GetRelojSharedVar(){
	piLock (RELOJ_KEY);
	TipoRelojShared g_relojSharedVars_security_copy = g_relojSharedVars;
	piUnlock (RELOJ_KEY);

	return g_relojSharedVars_security_copy ;
}

void SetRelojSharedVars (TipoRelojShared value){
	piLock (RELOJ_KEY);
	g_relojSharedVars = value;
	piUnlock (RELOJ_KEY);
}
