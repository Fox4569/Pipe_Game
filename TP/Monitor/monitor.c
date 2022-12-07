#include "header.h"

BOOL initMemAndSync(ControlData* cdata) {
	BOOL firstProcess = FALSE;

	//abrir FileMapping
	cdata->hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHM_NAME);

	if (cdata->hMapFile == NULL) {
		_tprintf(_T("O servidor não está a correr!\n"));
		return FALSE;
	}

	cdata->sharedMem = (SharedMem*)MapViewOfFile(cdata->hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);


	cdata->hMapTab = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHM_NAME_TAB);

	if (cdata->hMapTab == NULL) {
		_tprintf(_T("ERRO!\n"));
		return FALSE;
	}

	cdata->tabuleiro = (Tabuleiro*)MapViewOfFile(cdata->hMapTab, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(Tabuleiro));

	if (cdata->tabuleiro == NULL) {
		_tprintf(_T("ERRO!\n"));
		return FALSE;
	}

	//criar mutex (nome interprocessos)
	cdata->hRWMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);

	if (cdata->hRWMutex == NULL) {
		UnmapViewOfFile(cdata->sharedMem);
		UnmapViewOfFile(cdata->tabuleiro);
		CloseHandle(cdata->hMapFile);
		return FALSE;
	}

	cdata->hSemRead = CreateSemaphore(NULL, 0, BUFFERSIZE, SEMAPHORE_READ_NAME);

	if (cdata->hSemRead == NULL) {
		UnmapViewOfFile(cdata->sharedMem);
		UnmapViewOfFile(cdata->tabuleiro);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hRWMutex);
		return FALSE;
	}

	cdata->hSemWrite = CreateSemaphore(NULL, BUFFERSIZE, BUFFERSIZE, SEMAPHORE_WRITE_NAME);

	if (cdata->hSemWrite == NULL) {
		UnmapViewOfFile(cdata->sharedMem);
		UnmapViewOfFile(cdata->tabuleiro);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hRWMutex);
		CloseHandle(cdata->hSemRead);
		return FALSE;
	}

	cdata->hEvent = CreateEvent(NULL, TRUE, FALSE, EVENT_NAME);

	if (cdata->hSemWrite == NULL) {
		UnmapViewOfFile(cdata->sharedMem);
		UnmapViewOfFile(cdata->tabuleiro);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hRWMutex);
		CloseHandle(cdata->hSemRead);
		CloseHandle(cdata->hSemWrite);
		return FALSE;
	}

	cdata->hEventTab = CreateEvent(NULL, TRUE, FALSE, EVENT_TAB_NAME);

	if (cdata->hEventTab == NULL) {
		UnmapViewOfFile(cdata->sharedMem);
		UnmapViewOfFile(cdata->tabuleiro);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hRWMutex);
		CloseHandle(cdata->hSemRead);
		CloseHandle(cdata->hSemWrite);
		CloseHandle(cdata->hEvent);
		return FALSE;
	}

	return TRUE;
}

DWORD WINAPI produz(LPVOID p) {
	ControlData* cdata = (ControlData*)p;
	TCHAR comando[20];
	TCHAR msg[100];
	BufferCell cell;

	do {
		_tprintf(_T("Introduza o comando para o servidor: "));
		_getts_s(comando, 20);

		if (_tcscmp(comando, _T("fechar torneira")) == 0) {
			_tprintf(_T("\nIntroduza o tempo, em segundos: "));
			int tempo;
			_tscanf_s(_T("%d"), &tempo);
			cell.letra = 'f';
			cell.vals[0] = tempo;

			WaitForSingleObject(cdata->hSemWrite, INFINITE);
			WaitForSingleObject(cdata->hRWMutex, INFINITE);

			CopyMemory(&(cdata->sharedMem->buffer[(cdata->sharedMem->wP)++]), &cell, sizeof(BufferCell));

			if ((cdata->sharedMem->wP) == BUFFERSIZE)
				(cdata->sharedMem->wP) = 0;

			_tprintf(_T("%c e %d"), cell.letra, cell.vals[0]);

			ReleaseMutex(cdata->hRWMutex);
			ReleaseSemaphore(cdata->hSemRead, 1, NULL);

		}
		else if (_tcscmp(comando, _T("tubos random")) == 0) {
			cell.letra = 'r';

			WaitForSingleObject(cdata->hSemWrite, INFINITE);
			WaitForSingleObject(cdata->hRWMutex, INFINITE);

			CopyMemory(&(cdata->sharedMem->buffer[(cdata->sharedMem->wP)++]), &cell, sizeof(BufferCell));

			if ((cdata->sharedMem->wP) == BUFFERSIZE)
				(cdata->sharedMem->wP) = 0;

			ReleaseMutex(cdata->hRWMutex);
			ReleaseSemaphore(cdata->hSemRead, 1, NULL);
		}
		else if (_tcscmp(comando, _T("adicionar bloco")) == 0) {
			int linha, coluna;

			do {
				_tprintf(_T("\nNº da linha: "));
				_tscanf_s(_T("%d"), &linha);
			} while (linha > cdata->sharedMem->lin_col || linha < 0);

			do {
				_tprintf(_T("\nNº da coluna: "));
				_tscanf_s(_T("%d"), &coluna);
			} while (coluna > cdata->sharedMem->lin_col || coluna < 0);

			cell.letra = 'a';
			cell.vals[0] = linha;
			cell.vals[1] = coluna;

			WaitForSingleObject(cdata->hSemWrite, INFINITE);
			WaitForSingleObject(cdata->hRWMutex, INFINITE);

			CopyMemory(&(cdata->sharedMem->buffer[(cdata->sharedMem->wP)++]), &cell, sizeof(BufferCell));

			if ((cdata->sharedMem->wP) == BUFFERSIZE)
				(cdata->sharedMem->wP) = 0;

			//_tprintf(_T("%c e %d, %d"), cell.letra, cell.vals[0], cell.vals[1]);

			ReleaseMutex(cdata->hRWMutex);
			ReleaseSemaphore(cdata->hSemRead, 1, NULL);
		}
		else if (_tcscmp(comando, _T("sair")) == 0) {
			WaitForSingleObject(cdata->hSemWrite, INFINITE);
			WaitForSingleObject(cdata->hRWMutex, INFINITE);

			cdata->sharedMem->shutdown = 1;
			cell.letra = 's';

			CopyMemory(&(cdata->sharedMem->buffer[(cdata->sharedMem->wP)++]), &cell, sizeof(BufferCell));

			if ((cdata->sharedMem->wP) == BUFFERSIZE)
				(cdata->sharedMem->wP) = 0;

			ReleaseMutex(cdata->hRWMutex);
			ReleaseSemaphore(cdata->hSemRead, 1, NULL);
		}
		else {
			_tprintf(_T("Esse comando não existe\n"));
		}

	} while (cdata->sharedMem->estadoJogo != 'u');

	return 0;
}

void changeChar(int i, int j, ControlData* cdata) {
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	if ((cdata->tablocal[i][j]) == 'a') {
		_tprintf(_T("━━━"));
	}
	else if ((cdata->tablocal[i][j]) == 'b') {
		_tprintf(_T(" ┃ "));
	}
	else if ((cdata->tablocal[i][j]) == 'c') {
		_tprintf(_T(" ┏━"));
	}
	else if ((cdata->tablocal[i][j]) == 'd') {
		_tprintf(_T("━┓ "));
	}
	else if ((cdata->tablocal[i][j]) == 'e') {
		_tprintf(_T("━┛ "));
	}
	if ((cdata->tablocal[i][j]) == 'f') {
		_tprintf(_T(" ┗━"));
	}
	else if ((cdata->tablocal[i][j]) == 'g') {
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE));
		_tprintf(_T("━━━"));
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
	}
	else if ((cdata->tablocal[i][j]) == 'h') {
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE));
		_tprintf(_T(" ┃ "));
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
	}
	else if ((cdata->tablocal[i][j]) == 'i') {
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE));
		_tprintf(_T(" ┏━"));
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
	}
	else if ((cdata->tablocal[i][j]) == 'j') {
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE));
		_tprintf(_T("━┓ "));
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
	}
	else if ((cdata->tablocal[i][j]) == 'k') {
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE));
		_tprintf(_T("━┛ "));
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
	}
	else if ((cdata->tablocal[i][j]) == 'l') {
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE));
		_tprintf(_T(" ┗━"));
		SetConsoleTextAttribute(hStdOut, (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED));
	}
	else if ((cdata->tablocal[i][j]) == 'I') {
		_tprintf(_T(" I "));
	}
	else if ((cdata->tablocal[i][j]) == '_') {
		_tprintf(_T(" _ "));
	}
	else if ((cdata->tablocal[i][j]) == 'F') {
		_tprintf(_T(" F "));
	}
	else if ((cdata->tablocal[i][j]) == 'B') {
		_tprintf(_T(" X "));
	}
	//CloseHandle(hStdOut);
}

DWORD WINAPI mostraTab(LPVOID p) {
	ControlData* cdata = (ControlData*)p;
	char tab[20][20];

	Sleep(1000);
	do {
		if (cdata->sharedMem->pausa == TRUE) {				//Se o jogo estiver em pausa
			WaitForSingleObject(cdata->hEvent, INFINITE);	//Espera pelo evento
		}

		Sleep(500); //para dar tempo para o servidor correr o ResetEvent()
		WaitForSingleObject(cdata->hEventTab, INFINITE);
		WaitForSingleObject(cdata->hRWMutex, INFINITE);

		memcpy(&cdata->tablocal, &cdata->tabuleiro->tab, 400);

		//CopyMemory(tab, cdata->tabuleiro->tab, sizeof(cdata->tabuleiro->tab));

		_tprintf(_T("\n\n"));
		for (int i = 0; i < cdata->sharedMem->lin_col; i++) {
			for (int j = 0; j < cdata->sharedMem->lin_col; j++) {
				changeChar(i, j, cdata);
				//_tprintf(_T(" %c "), cdata->tablocal[i][j]);
			}
			_puttchar(_T('\n'));
		}

		ReleaseMutex(cdata->hRWMutex);

	} while (cdata->sharedMem->estadoJogo == 'u');

	return 0;

}

int _tmain(int argc, TCHAR* argv[]) {
	HKEY hKey;
	TCHAR caminhoCompletoComChave[TAM] = _T("Software\\TP\\vars"), parValor[TAM], parValor2[TAM];
	DWORD lin_col, tempo_agua;
	ControlData cdata;
	SharedMem mem;
	char** tab;
	HANDLE hThread, hThreadMostra;
	TCHAR command[100];

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	if (!initMemAndSync(&cdata)) {
		//_tprintf(_T("Error creating/opening shared memory"));
		exit(1);
	}

	_tprintf(_T("linhas/colunas = %d\ntempo_agua = %d\n"), cdata.sharedMem->lin_col, cdata.sharedMem->tempo_agua);

	/*cdata.tabuleiro->tab = malloc(sizeof(char*) * cdata.sharedMem->lin_col);

	if (cdata.tabuleiro->tab == NULL) {
		_tprintf(_T("Erro na alocação de memória!\n"));
		return 0;
	}

	for (int i = 0; i < cdata.sharedMem->lin_col; i++) {
		cdata.tabuleiro->tab[i] = malloc(sizeof(char) * cdata.sharedMem->lin_col);
	}*/

	hThread = CreateThread(NULL, 0, produz, &cdata, 0, NULL);
	hThreadMostra = CreateThread(NULL, 0, mostraTab, &cdata, 0, NULL);

	//Sleep(20000);

	/*WaitForSingleObject(cdata.hRWMutex, INFINITE);
	cdata.shutdown = 1;		//alterar o trinco para que a thread termine
	ReleaseMutex(cdata.hRWMutex);*/

	WaitForSingleObject(hThread, INFINITE);
	WaitForSingleObject(hThreadMostra, INFINITE);

	UnmapViewOfFile(cdata.sharedMem);		//tira o ficheiro da memória
	UnmapViewOfFile(cdata.tabuleiro);
	CloseHandle(cdata.hMapFile);
	CloseHandle(cdata.hRWMutex);
	CloseHandle(cdata.hSemWrite);
	CloseHandle(cdata.hSemRead);

	return 0;
}