#pragma once

#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#define TAM 200
#define MAX 256
#define SHM_NAME _T("fmMsgSpace")				//nome da memoria partilhada
#define SHM_NAME_TAB _T("shmTab")
#define MUTEX_NAME _T("RWMUTEX")				//nome do mutex
#define EVENT_NAME _T("EVENT")					//nome do evento
#define EVENT_TAB_NAME _T("EventTab")			//nome do evento Tab
#define SEMAPHORE_WRITE_NAME _T("HSEMWRITE")	//nome do semaforo de escrita
#define SEMAPHORE_READ_NAME _T("HSEMREAD")		//nome do semaforo de leitura
#define BUFFERSIZE 9							//tamanho do buffer
#define SHMEMSIZE 4096 

typedef struct _BufferCell {
	unsigned char letra;
	unsigned int vals[10];
} BufferCell;

typedef struct sharedMem {
	BufferCell buffer[BUFFERSIZE];
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
	HANDLE hEventTab;				//evento de aviso do tabuleiro
} ControlData;

BOOL initMemAndSync(ControlData* cdata);

DWORD WINAPI consome(LPVOID p);

DWORD WINAPI correAgua(LPVOID p);

DWORD WINAPI mostraTab(LPVOID p);

DWORD WINAPI recebeComandos(LPVOID p);

DWORD WINAPI mostraTab(LPVOID p);

DWORD WINAPI produz(LPVOID p);

void changeChar(int i, int j, ControlData* cdata);