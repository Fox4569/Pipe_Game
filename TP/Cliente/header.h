#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windowsx.h>

#define TAM 200
#define MAX 256
#define SHM_NAME _T("fmMsgSpace")				//nome da memoria partilhada
#define SHM_NAME_TAB _T("shmTab")
#define MUTEX_NAME _T("RWMUTEX")				//nome do mutex
#define EVENT_NAME _T("EVENT")					//nome do evento
#define SEMAPHORE_WRITE_NAME _T("HSEMWRITE")	//nome do semaforo de escrita
#define SEMAPHORE_READ_NAME _T("HSEMREAD")		//nome do semaforo de leitura
#define BUFFERSIZE 9							//tamanho do buffer
#define SHMEMSIZE 4096 

#define PIPE_NAME _T("\\\\.\\pipe\\jogo")

typedef struct sharedMem {
	unsigned int shutdown;			//trinco
	TCHAR estadoJogo;
	int lin_col;
	int tempo_agua;
	int inicio;
	int final;
	int wP;
	int rP;
	BOOL agua_correr;
	BOOL random_pecas;
	BOOL pausa;
} SharedMem;

typedef struct tabuleiro {
	char** tab;
}Tabuleiro;

typedef struct ControlData {
	HANDLE hMapFile;				//ficheiro de memoria
	HANDLE hMapTab;
	SharedMem* sharedMem;			//memoria partilhada
	Tabuleiro* tabuleiro;
	char tablocal[20][20];
	HANDLE hRWMutex;				//mutex
	HANDLE hSemWrite;				//semaforo de escrita
	HANDLE hSemRead;				//semaforo de leitura
	HANDLE hEvent;					//evento
	HANDLE hPipe;
} ControlData;
