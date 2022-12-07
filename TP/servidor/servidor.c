#include "header.h"

BOOL initMemAndSync(ControlData* cdata) {
	BOOL firstProcess = FALSE;

	//se já existir, abre
	cdata->hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHM_NAME);

	//se não, cria
	if (cdata->hMapFile == NULL) {
		firstProcess = TRUE;
		cdata->hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHMEMSIZE, SHM_NAME);	//ultimo -> nome comum

		if (cdata->hMapFile == NULL) {
			_tprintf(_T("ERRO\n"));
			return FALSE;
		}
	}
	else {
		_tprintf(_T("Já existe uma instância do Servidor a correr!\n"));
		return FALSE;
	}

	cdata->sharedMem = (SharedMem*)MapViewOfFile(cdata->hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

	cdata->hMapTab = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Tabuleiro), SHM_NAME_TAB);

	if (cdata->hMapTab == NULL) {
		_tprintf(_T("ERRO\n"));
		return FALSE;
	}

	cdata->tabuleiro = (Tabuleiro*)MapViewOfFile(cdata->hMapTab, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

	if (firstProcess) {
		cdata->sharedMem->wP = 0;
		cdata->sharedMem->rP = 0;
		cdata->sharedMem->agua_correr = TRUE;
		cdata->sharedMem->random_pecas = FALSE;
		cdata->sharedMem->pausa = FALSE;
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

	if (cdata->hEvent == NULL) {
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
	

	cdata->hEventJogada = CreateEvent(NULL, TRUE, FALSE, EVENT_JOG_NAME);

	if (cdata->hEventTab == NULL) {
		UnmapViewOfFile(cdata->sharedMem);
		UnmapViewOfFile(cdata->tabuleiro);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hRWMutex);
		CloseHandle(cdata->hSemRead);
		CloseHandle(cdata->hSemWrite);
		CloseHandle(cdata->hEvent);
		CloseHandle(cdata->hEventTab);
		return FALSE;
	}

	cdata->hEventEnvia = CreateEvent(NULL, TRUE, FALSE, EVENT_ENVIA);

	if (cdata->hEventEnvia == NULL) {
		UnmapViewOfFile(cdata->sharedMem);
		UnmapViewOfFile(cdata->tabuleiro);
		CloseHandle(cdata->hMapFile);
		CloseHandle(cdata->hRWMutex);
		CloseHandle(cdata->hSemRead);
		CloseHandle(cdata->hSemWrite);
		CloseHandle(cdata->hEvent);
		CloseHandle(cdata->hEventTab);
		CloseHandle(cdata->hEventJogada);
		return FALSE;
	}

	return TRUE;
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

void colocaPecas(int x, int y, ControlData* cdata) {

	char pecas[6] = { 'a', 'b', 'c', 'd', 'e', 'f' };
	DWORD n;

	if (cdata->sharedMem->random_pecas == FALSE) {
		WaitForSingleObject(cdata->hRWMutex, INFINITE);
		if (cdata->tablocal[x][y] == '_')
			cdata->tablocal[x][y] = 'a';
		else if (cdata->tablocal[x][y] == 'a')
			cdata->tablocal[x][y] = 'b';
		else if (cdata->tablocal[x][y] == 'b')
			cdata->tablocal[x][y] = 'c';
		else if (cdata->tablocal[x][y] == 'c')
			cdata->tablocal[x][y] = 'd';
		else if (cdata->tablocal[x][y] == 'd')
			cdata->tablocal[x][y] = 'e';
		else if (cdata->tablocal[x][y] == 'e')
			cdata->tablocal[x][y] = 'f';
		else if (cdata->tablocal[x][y] == 'f')
			cdata->tablocal[x][y] = 'a';
		ReleaseMutex(cdata->hRWMutex);
	} else {
		srand(time(NULL));
		int r = rand() % 6;

		cdata->tablocal[x][y] = pecas[r];

		WriteFile(cdata->hPipe, &cdata->tablocal, sizeof(cdata->tablocal), &n, NULL);
	}
}

DWORD WINAPI consome(LPVOID p) {
	ControlData* cdata = (ControlData*)p;
	SharedMem shMem;
	TCHAR msg[20];
	int tempo;
	BufferCell cell;
	char toCliente[3];
	DWORD n;
	BOOL ret;

	do {
		WaitForSingleObject(cdata->hSemRead, INFINITE);
		WaitForSingleObject(cdata->hRWMutex, INFINITE);

		CopyMemory(&cell, &(cdata->sharedMem->buffer[(cdata->sharedMem->rP)++]), sizeof(BufferCell));

		ReleaseMutex(cdata->hRWMutex);
		ReleaseSemaphore(cdata->hSemWrite, 1, NULL);

		if (cdata->sharedMem->rP == BUFFERSIZE)
			cdata->sharedMem->rP = 0;

		if (cell.letra == 'f') {

			WaitForSingleObject(cdata->hRWMutex, INFINITE);
			cdata->sharedMem->agua_correr = FALSE;
			ReleaseMutex(cdata->hRWMutex);

			_tprintf(_T("A água vai parar de fluir durante %d segundos!\n"), cell.vals[0]);

			Sleep(cell.vals[0] * 1000);

			WaitForSingleObject(cdata->hRWMutex, INFINITE);

			SetEvent(cdata->hEvent);								//avisa que a agua pode continuar
			Sleep(500);
			ResetEvent(cdata->hEvent);								//faz reset ao evento, para não ficar em loop

			_tprintf(_T("\nA água voltou a fluir!"));

			cdata->sharedMem->agua_correr = TRUE;

			ReleaseMutex(cdata->hRWMutex);

		}
		else if (cell.letra == 'a') {
			_tprintf(_T("Vai ser adicionado um bloco na posição (%d,%d)!\n"), cell.vals[0], cell.vals[1]);

			int linha = cell.vals[0] - 1;
			int col = cell.vals[1] - 1;

			//WaitForSingleObject(cdata->hRWMutex, INFINITE);

			if (cdata->tablocal[linha][col] == '_')
				cdata->tablocal[linha][col] = 'B';

			toCliente[0] = 'a';
			toCliente[1] = linha;
			toCliente[2] = col;

			ret = WriteFile(cdata->hPipe, &toCliente, sizeof(toCliente), &n, NULL);

			if (!ret)
				_tprintf(_T("\nErro no envio da mensagem ao Cliente!"));

			//ReleaseMutex(cdata->hRWMutex);
		}
		else if (cell.letra == 'r') {
			//WaitForSingleObject(cdata->hRWMutex, INFINITE);

			if (cdata->sharedMem->random_pecas == TRUE) {
				_tprintf(_T("Tubos aleatórios: OFF\n"));
				cdata->sharedMem->random_pecas = FALSE;
			}
			else {
				_tprintf(_T("Tubos aleatórios: ON\n"));
				cdata->sharedMem->random_pecas = TRUE;
			}

			toCliente[0] = 'r';

			ret = WriteFile(cdata->hPipe, &toCliente, sizeof(toCliente), &n, NULL);

			if (!ret)
				_tprintf(_T("\nErro no envio da mensagem ao Cliente!"));

			//ReleaseMutex(cdata->hRWMutex);

		}
		else if (cell.letra == 's') {
			break;
		}
		else {
			_tprintf(_T("\nERRO"));
		}

	} while (cdata->sharedMem->estadoJogo == 'u' && cdata->sharedMem->shutdown != 1);

	return 0;

}

DWORD WINAPI correAgua(LPVOID p) {
	ControlData* cdata = (ControlData*)p;
	int i = cdata->sharedMem->inicio, j = 0;

	do {

		if (cdata->sharedMem->agua_correr == FALSE)			//Se a torneira tiver fechada
			WaitForSingleObject(cdata->hEvent, INFINITE);	//Espera pelo evento

		if (cdata->sharedMem->pausa == TRUE) {				//Se o jogo estiver em pausa
			WaitForSingleObject(cdata->hEvent, INFINITE);	//Espera pelo evento
		}

		WaitForSingleObject(cdata->hRWMutex, INFINITE);

		char num = cdata->tablocal[i][j];

		switch (num)
		{
		case 'a':
			cdata->tablocal[i][j] = 'g';

			if (cdata->tablocal[i][j + 1] == 'a' || cdata->tablocal[i][j + 1] == 'd' || cdata->tablocal[i][j + 1] == 'e') {
				j++;
			}
			else if (cdata->tablocal[i][j - 1] == 'a' || cdata->tablocal[i][j - 1] == 'c' || cdata->tablocal[i][j - 1] == 'f') {
				j--;
			}
			else {
				cdata->sharedMem->estadoJogo = 'p';
			}

			if (cdata->tablocal[i][j - 1] == 'F' || cdata->tablocal[i][j + 1] == 'F')
				cdata->sharedMem->estadoJogo = 'g';

			break;
		case 'b':
			cdata->tablocal[i][j] = 'h';

			if (cdata->tablocal[i + 1][j] == 'b' || cdata->tablocal[i + 1][j] == 'e' || cdata->tablocal[i + 1][j] == 'f') {
				i++;
			}
			else if (cdata->tablocal[i - 1][j] == 'b' || cdata->tablocal[i - 1][j] == 'c' || cdata->tablocal[i - 1][j] == 'd') {
				i--;
			}
			else {
				cdata->sharedMem->estadoJogo = 'p';
			}

			if (cdata->tablocal[i + 1][j] == 'F' || cdata->tablocal[i - 1][j] == 'F')
				cdata->sharedMem->estadoJogo = 'g';

			break;
		case 'c':
			cdata->tablocal[i][j] = 'i';

			if (cdata->tablocal[i][j + 1] == 'a' || cdata->tablocal[i][j + 1] == 'd' || cdata->tablocal[i][j + 1] == 'e') {
				j++;
			}
			else if (cdata->tablocal[i + 1][j] == 'b' || cdata->tablocal[i + 1][j] == 'e' || cdata->tablocal[i + 1][j] == 'f') {
				i++;
			}
			else {
				cdata->sharedMem->estadoJogo = 'p';
			}

			if (cdata->tablocal[i][j + 1] == 'F' || cdata->tablocal[i + 1][j] == 'F')
				cdata->sharedMem->estadoJogo = 'g';

			break;
		case 'd':
			cdata->tablocal[i][j] = 'j';

			if (cdata->tablocal[i + 1][j] == 'b' || cdata->tablocal[i + 1][j] == 'e' || cdata->tablocal[i + 1][j] == 'f') {
				i++;
			}
			else if (cdata->tablocal[i][j - 1] == 'a' || cdata->tablocal[i][j - 1] == 'c' || cdata->tablocal[i][j - 1] == 'f'){
				j--;
			}else{
				cdata->sharedMem->estadoJogo = 'p';
			}

			if (cdata->tablocal[i + 1][j] == 'F' || cdata->tablocal[i][j - 1] == 'F') {
				cdata->sharedMem->estadoJogo = 'g';
				break;
			}

			break;

		case 'e':
			cdata->tablocal[i][j] = 'k';

			if (cdata->tablocal[i][j - 1] == 'a' || cdata->tablocal[i][j - 1] == 'c' || cdata->tablocal[i][j - 1] == 'f') {
				j--;
			}
			else if (cdata->tablocal[i - 1][j] == 'b' || cdata->tablocal[i - 1][j] == 'c' || cdata->tablocal[i - 1][j] == 'd') {
				i--;
			}
			else {
				cdata->sharedMem->estadoJogo = 'p';
			}

			if (cdata->tablocal[i - 1][j] == 'F' || cdata->tablocal[i][j - 1] == 'F') {
				cdata->sharedMem->estadoJogo = 'g';
				break;
			}

			break;
		case 'f':
			cdata->tablocal[i][j] = 'l';

			if (cdata->tablocal[i][j + 1] == 'a' || cdata->tablocal[i][j + 1] == 'd' || cdata->tablocal[i][j + 1] == 'e') {
				j++;
			}
			else if (cdata->tablocal[i - 1][j] == 'b' || cdata->tablocal[i - 1][j] == 'c' || cdata->tablocal[i - 1][j] == 'd') {
				i--;
			}
			else {
				cdata->sharedMem->estadoJogo = 'p';
			}

			if (cdata->tablocal[i - 1][j] == 'F' || cdata->tablocal[i][j + 1] == 'F')
				cdata->sharedMem->estadoJogo = 'g';

			break;
		case 'I':
			if (cdata->tablocal[i][j + 1] == 'a' || cdata->tablocal[i][j + 1] == 'd' || cdata->tablocal[i][j + 1] == 'e') {
				j++;
			}else if (cdata->tablocal[i + 1][j] == 'b' || cdata->tablocal[i + 1][j] == 'f') {
				i++;
			}else if (cdata->tablocal[i - 1][j] == 'b' || cdata->tablocal[i - 1][j] == 'c') {
				i--;
			}else {
				cdata->sharedMem->estadoJogo = 'p';
			}
		default:
			break;
		}

		ReleaseMutex(cdata->hRWMutex);

		SetEvent(cdata->hEventEnvia);
		Sleep(500);
		ResetEvent(cdata->hEventEnvia);							//faz reset ao evento, para não ficar em loop

		Sleep(5000);
	} while (cdata->sharedMem->estadoJogo == 'u' && cdata->sharedMem->shutdown != 1);

	//ReleaseMutex(cdata->hRWMutex);

	if (cdata->sharedMem->shutdown != 1) {

		WaitForSingleObject(cdata->hRWMutex, INFINITE);

		cdata->sharedMem->shutdown = 1;

		if (cdata->sharedMem->estadoJogo == 'g')
			_tprintf(_T("\O JOGADOR GANHOU!"));
		else
			_tprintf(_T("\O JOGADOR PERDEU!"));

		ReleaseMutex(cdata->hRWMutex);
	}

	return 0;
}

DWORD WINAPI recebeComandos(LPVOID p) {
	ControlData* cdata = (ControlData*)p;

	TCHAR comando[20];

	_tprintf(_T("Introduza comandos: "));

	do {
		_getts_s(comando, 20);

		WaitForSingleObject(cdata->hRWMutex, INFINITE);

		if (_tcscmp(comando, _T("pausa")) == 0) {
			if (cdata->sharedMem->pausa == FALSE) {
				cdata->sharedMem->pausa = TRUE;
				_tprintf(_T("\nJogo colocado em pausa!\n"));
			}
			else {
				_tprintf(_T("\nO jogo já se encontra em pausa!\n"));
			}
		}
		else if (_tcscmp(comando, _T("continuar")) == 0) {
			if (cdata->sharedMem->pausa == TRUE) {
				cdata->sharedMem->pausa = FALSE;

				SetEvent(cdata->hEvent);								//avisa que a agua pode continuar
				Sleep(500);
				ResetEvent(cdata->hEvent);								//faz reset ao evento, para não ficar em loop

				_tprintf(_T("\nJogo retomado!\n"));
			}
			else {
				_tprintf(_T("\nO jogo já está em andamento!\n"));
			}
		}
		else if (_tcscmp(comando, _T("sair")) == 0) {
			cdata->sharedMem->shutdown = 1;
		}
		else {
			_tprintf(_T("\nComando não identificado!\n"));
		}

		ReleaseMutex(cdata->hRWMutex);
	} while (cdata->sharedMem->shutdown != 1 && cdata->sharedMem->estadoJogo == 'u');

	return 0;
}

DWORD WINAPI mostraTab(LPVOID p) {
	ControlData* cdata = (ControlData*)p;

	Sleep(1000);
	do {
		if (cdata->sharedMem->pausa == TRUE) {				//Se o jogo estiver em pausa
			WaitForSingleObject(cdata->hEvent, INFINITE);	//Espera pelo evento
		}

		CopyMemory(&cdata->tabuleiro->tab, &cdata->tablocal, sizeof(cdata->tablocal));
		SetEvent(cdata->hEventTab);								//avisa que foi escrito na memoria
		Sleep(500);
		ResetEvent(cdata->hEventTab);							//faz reset ao evento, para não ficar em loop

		WaitForSingleObject(cdata->hRWMutex, INFINITE);

		_tprintf(_T("\n\n"));
		for (int i = 0; i < cdata->sharedMem->lin_col; i++) {
			for (int j = 0; j < cdata->sharedMem->lin_col; j++) {
				changeChar(i, j, cdata);
			}
			_puttchar(_T('\n'));
		}

		ReleaseMutex(cdata->hRWMutex);
		Sleep(5000);
	} while (cdata->sharedMem->estadoJogo == 'u' && cdata->sharedMem->shutdown != 1);

	if (cdata->sharedMem->shutdown != 1) {
		WaitForSingleObject(cdata->hRWMutex, INFINITE);

		_tprintf(_T("\n\n"));
		for (int i = 0; i < cdata->sharedMem->lin_col; i++) {
			for (int j = 0; j < cdata->sharedMem->lin_col; j++) {
				changeChar(i, j, cdata);
			}
			_puttchar(_T('\n'));
		}

		ReleaseMutex(cdata->hRWMutex);
	}

	return 0;

}

DWORD WINAPI recebeCliente(LPVOID p) {
	ControlData* cdata = (ControlData*)p;
	TCHAR buf[256];
	int n, i, ret, x, y;

	do {
		ret = ReadFile(cdata->hPipe, &y, sizeof(int), &n, NULL);			//Tem de se receber ao contrário do envio
		if (!ret || !n) {
			_tprintf(_T("[ERRO] - ReadFile\n"), ret, n);
			break;
		}

		ret = ReadFile(cdata->hPipe, &x, sizeof(int), &n, NULL);
		if (!ret || !n) {
			break;
		}


		colocaPecas(x-1, y-1, cdata);
		WriteFile(cdata->hPipe, cdata->tablocal, sizeof(cdata->tablocal), &n, NULL);
	} while (cdata->sharedMem->shutdown != 1);

	if (!DisconnectNamedPipe(cdata->hPipe)) {
		_tprintf(_T("[ERRO] Desligar o pipe! (DisconnectNamedPipe)"));
		exit(-1);
	}

	return 0;
}

/*DWORD WINAPI enviaCliente(LPVOID p) {
	ControlData* cdata = (ControlData*)p;
	DWORD n;
	
	do{
		//WaitForSingleObject(cdata->hEventEnvia, INFINITE);
		//WriteFile(cdata->hPipe, &cdata->tablocal, sizeof(cdata->tablocal), &n, NULL);
	} while (cdata->sharedMem->shutdown != 1);


}*/

int _tmain(int argc, TCHAR* argv[]) {
	HKEY hKey;
	TCHAR caminhoCompletoComChave[TAM] = _T("Software\\TP\\vars"), parValor[TAM], parValor2[TAM];
	int resposta, op, i;
	ControlData cdata;
	HANDLE hThread, hThreadAgua, hThreadComandos, hThreadMostra, waitAgua, hThreadComunica, hEventTemp, hThreadEnvia, hPipe;
	TCHAR command[100], buf[256];

	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
		_setmode(_fileno(stderr), _O_WTEXT);
	#endif

	if (!initMemAndSync(&cdata)) {
		_tprintf(_T("Erro ao criar/abrir a memória partilhada"));
		exit(1);
	}	

	if (RegCreateKeyEx(HKEY_CURRENT_USER, caminhoCompletoComChave, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS
		, NULL, &hKey, &resposta) != ERROR_SUCCESS) {
		_tprintf(_T("\nOcorreu um erro!"));
		exit(-1);
	}

	if (argc == 1) {

		DWORD dwDataSize = sizeof(DWORD);
		RegQueryValueEx(hKey, _T("n_linhas_colunas"), NULL, NULL, (LPBYTE)&cdata.sharedMem->lin_col, &dwDataSize);
		RegQueryValueEx(hKey, _T("tempo_água"), NULL, NULL, (LPBYTE)&cdata.sharedMem->tempo_agua, &dwDataSize);
	}
	else if (argc == 3) {
		cdata.sharedMem->lin_col = _tstoi(argv[1]);
		cdata.sharedMem->tempo_agua = _tstoi(argv[2]);

		if (cdata.sharedMem->lin_col > 20 || cdata.sharedMem->lin_col < 0) {
			_tprintf(_T("Tamanho do tabuleiro de jogo inválido! [0,20]\n"));
			return 0;
		}

		RegSetValueEx(hKey, _T("n_linhas_colunas"), 0, REG_DWORD, (const BYTE*)&cdata.sharedMem->lin_col, sizeof(cdata.sharedMem->lin_col));
		RegSetValueEx(hKey, _T("tempo_água"), 0, REG_DWORD, (const BYTE*)&cdata.sharedMem->tempo_agua, sizeof(cdata.sharedMem->tempo_agua));
	}
	else {
		_tprintf(_T("ERRO\n"));
		return 0;
	}

	_tprintf(_T("\nBEM-VINDO AO JOGO DOS TUBOS!\n\n"));
	_tprintf(_T("Número de linhas e colunas: %d\nTempo que a água começará a fluir: %d"), cdata.sharedMem->lin_col, cdata.sharedMem->tempo_agua);

	cdata.hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_OUTBOUND | PIPE_ACCESS_INBOUND, PIPE_WAIT |
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1,
		sizeof(buf), sizeof(buf), 1000, NULL);

	if (cdata.hPipe == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
		exit(-1);
	}

	/*ZeroMemory(&cdata.hPipes[i].overlap, sizeof(cdata.hPipes[i].overlap));
	cdata.hPipes[i].hInstancia = hPipe;
	cdata.hPipes[i].overlap.hEvent = hEventTemp;
	cdata.hEvents[i] = hEventTemp;
	cdata.hPipes[i].ativo = FALSE;*/

	_tprintf(TEXT("\nÀ espera de um jogador...\n"));
	if (!ConnectNamedPipe(cdata.hPipe, NULL)) {
		_tprintf(TEXT("[ERRO] Ligação ao leitor! (ConnectNamedPipe\n"));
		exit(-1);
	}

	//Calcular posição de inicio e fim				
	srand(time(NULL));
	cdata.sharedMem->inicio = rand() % cdata.sharedMem->lin_col;

	_tprintf(_T("\nPosição inicial = %d\n"), cdata.sharedMem->inicio);

	cdata.sharedMem->final = cdata.sharedMem->lin_col - (cdata.sharedMem->inicio + 1);

	waitAgua = CreateWaitableTimer(NULL, FALSE, NULL);

	cdata.sharedMem->shutdown = 0;	//trinco
	cdata.sharedMem->estadoJogo = 'u';

	/*cdata.tablocal = malloc(sizeof(char*) * cdata.sharedMem->lin_col);

	if (cdata.tablocal == NULL) {
		_tprintf(_T("Erro na alocação de memória!\n"));
		return 0;
	}

	for (int i = 0; i < cdata.sharedMem->lin_col; i++) {
		cdata.tablocal[i] = malloc(sizeof(char) * cdata.sharedMem->lin_col);
	}*/

	for (int i = 0; i < cdata.sharedMem->lin_col; i++) {
		for (int j = 0; j < cdata.sharedMem->lin_col; j++) {
			cdata.tablocal[i][j] = _T('_');
		}
	}

	_tprintf(_T("\n\n"));
	for (int i = 0; i < cdata.sharedMem->lin_col; i++) {
		for (int j = 0; j < cdata.sharedMem->lin_col; j++) {
			changeChar(i, j, &cdata);
		}
		_puttchar(_T('\n'));
	}

	cdata.tablocal[cdata.sharedMem->inicio][0] = 'I';
	cdata.tablocal[cdata.sharedMem->final][cdata.sharedMem->lin_col-1] = 'F';

	_tprintf(_T("\n\n"));
	for (int i = 0; i < cdata.sharedMem->lin_col; i++) {
		for (int j = 0; j < cdata.sharedMem->lin_col; j++) {
			changeChar(i, j, &cdata);
		}
		_puttchar(_T('\n'));
	}

	CopyMemory(&cdata.tabuleiro->tab, &cdata.tablocal, sizeof(cdata.tablocal));

	//Enviar dados ao Cliente
	DWORD n;
	WriteFile(cdata.hPipe, &cdata.sharedMem->lin_col, sizeof(cdata.sharedMem->lin_col), &n, NULL);
	WriteFile(cdata.hPipe, &cdata.sharedMem->tempo_agua, sizeof(cdata.sharedMem->tempo_agua), &n, NULL);
	WriteFile(cdata.hPipe, &cdata.sharedMem->inicio, sizeof(cdata.sharedMem->inicio), &n, NULL);
	WriteFile(cdata.hPipe, &cdata.sharedMem->final, sizeof(cdata.sharedMem->final), &n, NULL);
	WriteFile(cdata.hPipe, &cdata.tablocal, sizeof(cdata.tablocal), &n, NULL);

	SetEvent(cdata.hEventTab);								//avisa que a agua pode continuar
	Sleep(500);
	ResetEvent(cdata.hEventTab);							//faz reset ao evento, para não ficar em loop

	hThread = CreateThread(NULL, 0, consome, &cdata, 0, NULL);
	hThreadAgua = CreateThread(NULL, 0, correAgua, &cdata, CREATE_SUSPENDED, NULL);
	hThreadComandos = CreateThread(NULL, 0, recebeComandos, &cdata, 0, NULL);
	hThreadMostra = CreateThread(NULL, 0, mostraTab, &cdata, 0, NULL);
	hThreadComunica = CreateThread(NULL, 0, recebeCliente, &cdata, 0, NULL);
	//hThreadEnvia = CreateThread(NULL, 0, enviaCliente, &cdata, 0, NULL);


	/*cdata.tablocal[2][1] = 'a';
	cdata.tablocal[2][2] = 'd';
	cdata.tablocal[3][2] = 'b';
	cdata.tablocal[4][2] = 'f';
	cdata.tablocal[4][3] = 'a';
	cdata.tablocal[4][4] = 'a';
	cdata.tablocal[4][5] = 'a';
	cdata.tablocal[4][6] = 'a';
	cdata.tablocal[4][7] = 'd';*/

	/*WaitForSingleObject(cdata.hRWMutex, INFINITE);

	_tprintf(_T("\n\n"));
	for (int i = 0; i < cdata.sharedMem->lin_col; i++) {
		for (int j = 0; j < cdata.sharedMem->lin_col; j++) {
			_tprintf(_T(" %c "), cdata.tablocal[i][j]);
		}
		_puttchar(_T('\n'));
	}

	ReleaseMutex(cdata.hRWMutex);*/

	//DÁ ERRO, NÃO SEI PORQUÊ (PERGUNTAR AO STOR)
	//SetWaitableTimer(waitAgua, cdata.sharedMem->tempo_agua * 1000, 0, NULL, NULL, TRUE);	//esperar o tempo para a agua começar a fluir
	//WaitForSingleObject(waitAgua, INFINITE);								

	Sleep(cdata.sharedMem->tempo_agua * 1000);
	ResumeThread(hThreadAgua);

	_tprintf(_T("\n\n"));
	for (int i = 0; i < cdata.sharedMem->lin_col; i++) {
		for (int j = 0; j < cdata.sharedMem->lin_col; j++) {
			changeChar(i, j, &cdata);
		}
		_puttchar(_T('\n'));
	}

	WaitForSingleObject(hThread, INFINITE);
	WaitForSingleObject(hThreadAgua, INFINITE);
	WaitForSingleObject(hThreadComandos, INFINITE);
	WaitForSingleObject(hThreadMostra, INFINITE);
	WaitForSingleObject(hThreadComunica, INFINITE);

	RegCloseKey(hKey);
	UnmapViewOfFile(cdata.sharedMem);		//tira o ficheiro da memória
	UnmapViewOfFile(cdata.tabuleiro);
	CloseHandle(cdata.hMapFile);
	CloseHandle(cdata.hRWMutex);
	CloseHandle(cdata.hSemWrite);
	CloseHandle(cdata.hSemRead);

	return 0;
}