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

	return TRUE;
}

int _tmain(int argc, TCHAR* argv[]) {
	HKEY hKey;
	int resposta, op, i, n, ret;
	ControlData cdata;
	HANDLE hPipe;
	TCHAR command[100], buf[256];
	int x, y;

	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
		_setmode(_fileno(stderr), _O_WTEXT);
	#endif

	if (!initMemAndSync(&cdata)) {
		//_tprintf(_T("Error creating/opening shared memory"));
		exit(1);
	}

	_tprintf(TEXT("[LEITOR] Esperar pelo pipe '%s' (WaitNamedPipe)\n"),
		PIPE_NAME);
	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_NAME);
		exit(-1);
	}
	_tprintf(TEXT("[LEITOR] Ligação ao pipe do escritor... (CreateFile)\n"));
	cdata.hPipe = CreateFile(PIPE_NAME, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (cdata.hPipe == NULL) {
		_tprintf(_T("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		exit(-1);
	}
	_tprintf(_T("[LEITOR] Liguei-me...\n"));

	do{
		_tprintf(_T("X: "));
		//_fgetts(buf, 256, stdin);
		//buf[_tcslen(buf) - 1] = '\0';
		_tscanf_s(_T("%d"), &x);

		_tprintf(_T("Y: "));
		_tscanf_s(_T("%d"), &y);
		if (!WriteFile(cdata.hPipe, &x, sizeof(int), &n, NULL)) {
			_tprintf(_T("[ERRO] Escrever no pipe! (WriteFile)\n"));
			exit(-1);
		}
		if (!WriteFile(cdata.hPipe, &y, sizeof(int), &n, NULL)) {
			_tprintf(_T("[ERRO] Escrever no pipe! (WriteFile)\n"));
			exit(-1);
		}
		_tprintf(_T("[ESCRITOR] Enviei %d bytes ao leitor... (WriteFile)\n"), n);
	} while (x != -1);

		UnmapViewOfFile(cdata.sharedMem);		//tira o ficheiro da memória
		UnmapViewOfFile(cdata.tabuleiro);
		CloseHandle(cdata.hMapFile);
		CloseHandle(cdata.hRWMutex);
		CloseHandle(cdata.hSemWrite);
		CloseHandle(cdata.hSemRead);
		CloseHandle(cdata.hPipe);
}