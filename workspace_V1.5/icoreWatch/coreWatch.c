#include "coreWatch.h"

//------------------------------------------------------
// VARIABLES GLOBALES
//------------------------------------------------------
TipoCoreWatch g_coreWatch;
static int g_flagsCoreWatch;

fsm_trans_t fsmTransCoreWatch[] ={
		{START, CompruebaSetupDone, STAND_BY, Start},
		{STAND_BY, CompruebaTimeActualizado, STAND_BY, ShowTime},
		{STAND_BY, CompruebaReset, STAND_BY, Reset},
		{STAND_BY, CompruebaSetCancelNewTime, SET_TIME, PrepareSetNewTime,},
		{SET_TIME, CompruebaSetCancelNewTime, STAND_BY, CancelSetNewTime},
		{SET_TIME, CompruebaNewTimeIsReady, STAND_BY, SetNewTime},
		{SET_TIME, CompruebaDigitoPulsado, SET_TIME, ProcesaDigitoTime},
		{-1, NULL, -1, NULL},
};

//------------------------------------------------------
// FUNCIONES PROPIAS
//------------------------------------------------------
// Wait until next_activation (absolute time)
// Necesita de la función "delay" de WiringPi.
void DelayUntil(unsigned int next) {
	unsigned int now = millis();
	if (next > now) {
		delay(next - now);
	}
}

int ConfiguraInicializaSistema (TipoCoreWatch *p_sistema){
#if VERSION >= 2
	g_flagsCoreWatch = 0;
	p_sistema->tempTime = 0;
	p_sistema->digitosGuardados = 0;
	int resultadoInicializarReloj = ConfiguraInicializaReloj(&p_sistema->reloj);
	if (resultadoInicializarReloj != 0){
		return CODIGO_ERROR;
	}
	int resultadoExploracionTeclado = piThreadCreate(ThreadExploraTecladoPC);
	if (resultadoExploracionTeclado != 0){
		return CODIGO_ERROR;
	}
	piLock (SYSTEM_KEY);
	g_flagsCoreWatch |= FLAG_SETUP_DONE;
	piUnlock (SYSTEM_KEY);
#endif
#if VERSION >= 3
	ConfiguraInicializaTeclado(&p_sistema->teclado);
	piLock (SYSTEM_KEY);
	g_flagsCoreWatch |= FLAG_SETUP_DONE;
	piUnlock (SYSTEM_KEY);
#endif
#if VERSION >= 4
	piLock (SYSTEM_KEY);
	g_flagsCoreWatch |= FLAG_SETUP_DONE;
	piUnlock (SYSTEM_KEY);
#endif

	return 0;
}

int EsNumero(char value){
	int result;
	if (value >= 48 && value <= 57){
		return result = 1;
	} else
		return result = 0;

}

//------------------------------------------------------
// FUNCIONES LIGADAS A THREADS ADICIONALES
//------------------------------------------------------

PI_THREAD(ThreadExploraTecladoPC){
	int teclaPulsada;
	while(1){
		delay(10);
		if (kbhit() != 0)
		{
			teclaPulsada = kbread();
#if VERSION >= 3
			piLock(SYSTEM_KEY);
			g_flagsCoreWatch && ~FLAG_TECLA_PULSADA;
			piUnlock(SYSTEM_KEY);
#endif
			if (teclaPulsada == TECLA_RESET){
				piLock(SYSTEM_KEY);
				g_flagsCoreWatch |= FLAG_RESET;
				piUnlock(SYSTEM_KEY);
			} else if(teclaPulsada == TECLA_SET_CANCEL_TIME){
				piLock(SYSTEM_KEY);
				g_flagsCoreWatch |= FLAG_SET_CANCEL_NEW_TIME;
				piUnlock(SYSTEM_KEY);
			} else if(EsNumero(teclaPulsada)==1){
				g_coreWatch.digitoPulsado = teclaPulsada;
				piLock(SYSTEM_KEY);
				g_flagsCoreWatch |= FLAG_DIGITO_PULSADO;
				piUnlock(SYSTEM_KEY);
			} else if(teclaPulsada == TECLA_EXIT){
				printf("Se va a salir del sistema\n");
				exit(0);
			}	else{
				printf("Tecla desconocida\n");
			}
		}
	}
}

//------------------------------------------------------
// FUNCIONES DE ENTRADA O DE TRANSICION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------
int CompruebaDigitoPulsado(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	int result;
	result = (g_flagsCoreWatch & FLAG_DIGITO_PULSADO); 	// Flag activado tras pulsar un número del teclado
	piUnlock (SYSTEM_KEY);

	return result;
}

int CompruebaNewTimeIsReady(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	int result;
	result = (g_flagsCoreWatch & FLAG_NEW_TIME_IS_READY); 	// Flag activado tras haber leído los 4 dígitos de una nueva hora
	piUnlock (SYSTEM_KEY);

	return result;
}

int CompruebaReset(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	int result;
	result = (g_flagsCoreWatch & FLAG_RESET);	// Flag activado cuando se pulsa la tecla TECLA_RESET 'F'
	piUnlock (SYSTEM_KEY);

	return result;
}

int CompruebaSetCancelNewTime(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	int result;
	result = (g_flagsCoreWatch & FLAG_SET_CANCEL_NEW_TIME);	// Flag activado cuando se pulsa la tecla TECLA_SET_CANCEL_TIME 'E'
	piUnlock (SYSTEM_KEY);

	return result;
}

int CompruebaSetupDone(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	int result;
	result = (g_flagsCoreWatch & FLAG_SETUP_DONE);	// Flag se activa al finalizar la configuración de inicialización del sistema
	piUnlock (SYSTEM_KEY);

	return result;
}

#if VERSION >=3
int CompruebaTeclaPulsada(fsm_t* p_this){
	piLock(SYSTEM_KEY);
	int result;
    result = (g_flagsCoreWatch & FLAG_SETUP_DONE);	// Flag se activa al finalizar la configuración de inicialización del sistema
    piUnlock (SYSTEM_KEY);

    return result;
}
#endif

int CompruebaTimeActualizado(fsm_t* p_this){
	int result;
	piLock(SYSTEM_KEY);
	result = (GetRelojSharedVar().flags & FLAG_TIME_ACTUALIZADO);
	piUnlock (SYSTEM_KEY);

	return result;
}

//------------------------------------------------------
// FUNCIONES DE SALIDA O DE ACCION DE LA MAQUINA DE ESTADOS
//------------------------------------------------------

void Start (fsm_t* p_this){
	piLock (SYSTEM_KEY);
	g_flagsCoreWatch &= ~FLAG_SETUP_DONE;		// preguntar como limpiar el flag
	piUnlock (SYSTEM_KEY);
}

void ShowTime (fsm_t* p_this){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	TipoRelojShared rs = GetRelojSharedVar();


	rs.flags &= ~FLAG_TIME_ACTUALIZADO; // No estoy seguro de que sea este el flag
	SetRelojSharedVars(rs);

#if VERSION <4
	piLock(RELOJ_KEY);
	printf("\n");
	piUnlock(RELOJ_KEY);
#endif
#if VERSION >=4
#endif
}

void Reset(fsm_t* p_this){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	ResetReloj(&p_sistema->reloj);
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= ~ FLAG_RESET;
	piUnlock(SYSTEM_KEY);
#if VERSION <4
	piLock(SYSTEM_KEY);
	printf("[RESET] Hora reiniciada\n");
	piUnlock(SYSTEM_KEY);
#endif
#if VERSION >=4
#endif
}

void PrepareSetNewTime(fsm_t* p_this){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	int formatoRecuperado = p_sistema->reloj.hora.formato;
	piLock(SYSTEM_KEY);
	g_flagsCoreWatch &= ~ FLAG_DIGITO_PULSADO;
	g_flagsCoreWatch &= ~ FLAG_SET_CANCEL_NEW_TIME;
	piUnlock(SYSTEM_KEY);
#if VERSION <4
	piLock(SYSTEM_KEY);
	printf("[SET_TIME] Introduzca la nueva hora en formato 0-%d\n", formatoRecuperado);
	piUnlock(SYSTEM_KEY);
#endif
#if VERSION >=4
#endif
}

void CancelSetNewTime(fsm_t* p_this){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	p_sistema->tempTime = 0;
	p_sistema->digitosGuardados = 0;
	piLock(SYSTEM_KEY);
		g_flagsCoreWatch &= ~ FLAG_SET_CANCEL_NEW_TIME;
	piUnlock(SYSTEM_KEY);
#if VERSION <4
	piLock(SYSTEM_KEY);
	printf("[SET_TIME] Operacion cancelada\n");
	piUnlock(SYSTEM_KEY);
#endif
#if VERSION >=4
#endif
}

void ProcesaDigitoTime(fsm_t* p_this){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	int recuperaTemptime = p_sistema->tempTime;
	int recuperaDigitosGuardados = p_sistema->digitosGuardados;
	int ultimoDigito = p_sistema->digitoPulsado;
	int formatoRecuperado = p_sistema->reloj.hora.formato;
	piLock(SYSTEM_KEY);
		g_flagsCoreWatch &= ~ FLAG_DIGITO_PULSADO;
	piUnlock(SYSTEM_KEY);

	if (recuperaDigitosGuardados == 0){
		if (formatoRecuperado == 12){
			ultimoDigito = MIN(1,ultimoDigito);
		} else {
			ultimoDigito = MIN(2,ultimoDigito);
		}
		recuperaTemptime=recuperaTemptime*10+ultimoDigito;
		recuperaDigitosGuardados++;
	}else if (recuperaDigitosGuardados == 1){
		if(formatoRecuperado==12){
			if(recuperaTemptime==0){
				ultimoDigito=MAX(1,ultimoDigito);
			} else{
				ultimoDigito= MIN(2,ultimoDigito);
			}
		} else
			if (recuperaTemptime==2){
			ultimoDigito=MIN(3,ultimoDigito);
			}
		recuperaTemptime=recuperaTemptime*10+ultimoDigito;
		recuperaDigitosGuardados++;
	} else if(recuperaDigitosGuardados==2){
		recuperaTemptime=recuperaTemptime*10+MIN(5,ultimoDigito);
		recuperaDigitosGuardados++;
	}else {
		recuperaTemptime=recuperaTemptime*10+ultimoDigito;
		piLock(SYSTEM_KEY);
			g_flagsCoreWatch &= ~ FLAG_DIGITO_PULSADO;		// Limpiamos el flag
			g_flagsCoreWatch |= FLAG_NEW_TIME_IS_READY;		// Activamos el flag
		piUnlock(SYSTEM_KEY);
	}
	if (recuperaDigitosGuardados < 3){
		if (recuperaTemptime > 2359){
			recuperaTemptime %= 10000;
			recuperaTemptime = 100 * MIN((int)(recuperaTemptime/100), 23) + MIN(recuperaTemptime%100, 59);
		}
	}
#if VERSION <4
	printf("[SET_TIME] Nueva hora temporal %d\n", recuperaTemptime);
#endif
#if VERSION >=4
#endif
	p_sistema->tempTime = recuperaTemptime;
	p_sistema->digitosGuardados = recuperaDigitosGuardados;
}

void SetNewTime(fsm_t* p_this){
	TipoCoreWatch *p_sistema = (TipoCoreWatch*)(p_this->user_data);
	piLock(SYSTEM_KEY);
		g_flagsCoreWatch &= ~ FLAG_NEW_TIME_IS_READY;		// Limpiamos el flag
	piUnlock(SYSTEM_KEY);
	int recuperaTemptime = p_sistema.tempTime;
	TipoReloj recuperaReloj = p_sistema.reloj;
	SetHora(recuperaTemptime, recuperaReloj);
	recuperaTemptime = 0;
	p_sistema.tempTime = recuperaTemptime;
	p_sistema.digitosGuardados = 0;

}

//------------------------------------------------------
// MAIN
//------------------------------------------------------
int main() {
	unsigned int next;

#if VERSION <=1
	TipoReloj relojPrueba;

	ConfiguraInicializaReloj(&relojPrueba);
	ActualizaFecha(&relojPrueba.calendario);
	relojPrueba.calendario.yyyy = 2000;
	relojPrueba.calendario.MM = 02;
	relojPrueba.calendario.dd = 28;
	SetHora(2358, &relojPrueba.hora);


	//printf("Son las %d:%d:%d del %d/%d/%d\n", relojPrueba.hora.hh, relojPrueba.hora.mm, relojPrueba.hora.ss, relojPrueba.calendario.dd, relojPrueba.calendario.MM, relojPrueba.calendario.yyyy);

#endif

#if VERSION >=2

	TipoCoreWatch miSistema;
	int resultadoInicializarSistema = ConfiguraInicializaSistema(&miSistema);
	if (resultadoInicializarSistema != 0){
		exit(0);
	}
#endif

	fsm_t* fsmReloj = fsm_new(WAIT_TIC, g_fsmTransReloj, &relojPrueba);
//	fsm_t* fsmCoreWatch = fsm_new(START, fsmTransCoreWatch, &relojPrueba);
	next = millis();

	while (1) {


		fsm_fire(fsmReloj);
//		fsm_fire(fsmCoreWatch);
		next += CLK_MS;
		DelayUntil(next);
		tmr_destroy(relojPrueba.tmrTic);
		fsm_destroy(fsmReloj);
		fsm_destroy(fsmCoreWatch);
	}

	return 0;
}
