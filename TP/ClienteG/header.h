#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windowsx.h>

#define TAM 200
#define MAX 256
#define MUTEX_NAME _T("RWMUTEX")				//nome do mutex
#define EVENT_NAME _T("EVENT")					//nome do evento
#define BUFFERSIZE 9							//tamanho do buffer
#define SHMEMSIZE 4096 

#define PIPE_NAME _T("\\\\.\\pipe\\jogo")

typedef struct ControlData {
	HANDLE hRWMutex;				//mutex
	HANDLE hEvent;					//evento
	HANDLE hPipe;
	int lin_col;
	TCHAR estadoJogo;
	int tempo_agua;
	int inicio;
	int final;
	BOOL agua_correr;
	BOOL random_pecas;
	BOOL pausa;
	char tablocal[20][20];
	HWND hWnd;
} ControlData;
