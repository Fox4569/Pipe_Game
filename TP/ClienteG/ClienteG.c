#include "header.h"

BOOL initMemAndSync(ControlData* cdata) {
	//criar mutex (nome interprocessos)
	cdata->hRWMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);

	if (cdata->hRWMutex == NULL) {
		return FALSE;
	}

	cdata->hEvent = CreateEvent(NULL, TRUE, FALSE, EVENT_NAME);

	if (cdata->hEvent == NULL) {
		CloseHandle(cdata->hRWMutex);
		return FALSE;
	}

	cdata->agua_correr = TRUE;
	cdata->random_pecas = FALSE;
	cdata->pausa = FALSE;
	cdata->estadoJogo = 'u';

	return TRUE;
}

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);


TCHAR szProgName[] = TEXT("Base");

DWORD WINAPI comunicaServidor(LPVOID p) {
	ControlData* cdata = (ControlData*)p;

	char msg[3];
	DWORD n;
	int ret;

	do {
		ReadFile(cdata->hPipe, &msg, sizeof(msg), &n, NULL);

		if(msg[0] == 'a'){
			//BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpInicio, 0, 0, SRCCOPY);
		}else if (msg[0] == 'r') {
			if (cdata->random_pecas)
				cdata->random_pecas == FALSE;
			else
				cdata->random_pecas == TRUE;
		}

		/*ret = ReadFile(cdata->hPipe, cdata->tablocal, sizeof(cdata->tablocal), &n, NULL);

		if (ret == 0)
			return 0;

		InvalidateRect(cdata->hWnd, NULL, TRUE);	//chamada ao WM_PAINT*/
	} while (1);



	return 0;
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow) {
	HANDLE hThreadMsg;
	ControlData cdata;
	int x, y;
	DWORD n;
	char c;

	if (!initMemAndSync(&cdata)) {
		exit(1);
	}

	_tprintf(TEXT("[LEITOR] Esperar pelo pipe '%s' (WaitNamedPipe)\n"),
		PIPE_NAME);
	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_NAME);
		exit(-1);
	}
	_tprintf(TEXT("[LEITOR] Ligação ao pipe do escritor... (CreateFile)\n"));
	cdata.hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (cdata.hPipe == NULL) {
		_tprintf(_T("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		exit(-1);
	}

	ReadFile(cdata.hPipe, &cdata.lin_col, sizeof(cdata.lin_col), &n, NULL);
	ReadFile(cdata.hPipe, &cdata.tempo_agua, sizeof(cdata.tempo_agua), &n, NULL);
	ReadFile(cdata.hPipe, &cdata.inicio, sizeof(cdata.inicio), &n, NULL);
	ReadFile(cdata.hPipe, &cdata.final, sizeof(cdata.final), &n, NULL);
	ReadFile(cdata.hPipe, &cdata.tablocal, sizeof(cdata.tablocal), &n, NULL);
	//hThreadMsg = CreateThread(NULL, 0, comunicaServidor, &cdata, 0, NULL);

	HWND hWnd;
	MSG lpMsg;		
	WNDCLASSEX wcApp;

	// ============================================================================
	// 1. Definição das características da janela "wcApp" 
	//    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
	// ============================================================================
	wcApp.cbSize = sizeof(WNDCLASSEX);     
	wcApp.hInstance = hInst;		        
	wcApp.lpszClassName = szProgName;		 
	wcApp.lpfnWndProc = TrataEventos;        
	wcApp.style = CS_HREDRAW | CS_VREDRAW;   
	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);   
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION); 
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);	
	wcApp.lpszMenuName = NULL;			
	wcApp.cbClsExtra = 0;				
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcApp.cbWndExtra = sizeof(ControlData*);

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName,			
		TEXT("Exemplo de Janela Principal em C"),
		WS_OVERLAPPEDWINDOW,	
		CW_USEDEFAULT,		
		CW_USEDEFAULT,		
		CW_USEDEFAULT,		
		CW_USEDEFAULT,		
		(HWND)HWND_DESKTOP,	
		(HMENU)NULL,			
		(HINSTANCE)hInst,		 
		0);		

	cdata.hWnd = hWnd;	//colocar o handle da janela na estrutura
	LONG_PTR res = SetWindowLongPtr(hWnd, 0, (LONG_PTR)&cdata);

	//Mostrar a janela
	ShowWindow(hWnd, nCmdShow);	
	UpdateWindow(hWnd);		
	
	//loop de mensagens
	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);	
		DispatchMessage(&lpMsg);	
	}

	CloseHandle(cdata.hPipe);


	return((int)lpMsg.wParam);
}

typedef struct {
	TCHAR c;
	int xPos, yPos;
} PosChar;

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {

	static ControlData* pont;

	static TCHAR key = '?';
	int xPos, yPos;
	HDC hdc;
	RECT rect;
	PAINTSTRUCT ps;
	int aux = 1;
	int x, y;

	static PosChar posicoes[100];
	static int totalPos = 0;

	HBITMAP hBmp, hInicio, hFinal, hA, hB, hC, hD, hE, hF, hG, hH, hI, hJ, hK, hL, hBloco;
	static BITMAP bmp;
	static HDC bmpDC, bmpInicio, bmpFinal, bmpA, bmpB, bmpC, bmpD, bmpE, bmpF, bmpG, bmpH, bmpI, bmpJ, bmpK, bmpL, bmpBloco;
	static int xBitmap;
	static int yBitmap;
	DWORD n;

	switch (messg) {

	case WM_CREATE:


		break;

	case WM_CLOSE:
		if (MessageBox(hWnd, _T("Sair?"), _T("Deseja mesmo sair?"), MB_ICONQUESTION | MB_YESNO | MB_HELP) == IDYES) {
			DestroyWindow(hWnd);
		}
		break;

	case WM_HELP:
		MessageBox(hWnd, _T("Janela de ajuda"), _T("Sair"), MB_OK);
		break;

	case WM_LBUTTONDOWN:		//Apanhar evento do botão esquerdo do rato
		posicoes[totalPos].xPos = GET_X_LPARAM(lParam);
		posicoes[totalPos].yPos = GET_Y_LPARAM(lParam);
		posicoes[totalPos].c = key;
		totalPos++;

		x = GET_X_LPARAM(lParam)/20;
		y = GET_Y_LPARAM(lParam)/20;

		WriteFile(pont->hPipe, &x, sizeof(x), &n, NULL);
		WriteFile(pont->hPipe, &y, sizeof(y), &n, NULL);

		ReadFile(pont->hPipe, pont->tablocal, sizeof(pont->tablocal), &n, NULL);

		InvalidateRect(hWnd, NULL, TRUE);	//chamada ao WM_PAINT
		break;

	case WM_CHAR:				//Apanhar evento do teclado
		key = (TCHAR)wParam;
		break;

	case WM_PAINT:
	{
		pont = (ControlData*)GetWindowLongPtr(hWnd, 0);

			hBmp = (HBITMAP)LoadImage(NULL, TEXT("QuadradoBranco.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hBmp, sizeof(bmp), &bmp);

			hInicio = (HBITMAP)LoadImage(NULL, TEXT("Inicio.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hInicio, sizeof(bmp), &bmp);

			hFinal = (HBITMAP)LoadImage(NULL, TEXT("Final.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hFinal, sizeof(bmp), &bmp);

			hA = (HBITMAP)LoadImage(NULL, TEXT("a.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hA, sizeof(bmp), &bmp);

			hB = (HBITMAP)LoadImage(NULL, TEXT("b.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hB, sizeof(bmp), &bmp);

			hC = (HBITMAP)LoadImage(NULL, TEXT("c.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hC, sizeof(bmp), &bmp);

			hD = (HBITMAP)LoadImage(NULL, TEXT("d.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hD, sizeof(bmp), &bmp);

			hE = (HBITMAP)LoadImage(NULL, TEXT("e.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hE, sizeof(bmp), &bmp);

			hF = (HBITMAP)LoadImage(NULL, TEXT("f.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hF, sizeof(bmp), &bmp);

			hG = (HBITMAP)LoadImage(NULL, TEXT("g.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hG, sizeof(bmp), &bmp);

			hH = (HBITMAP)LoadImage(NULL, TEXT("h.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hH, sizeof(bmp), &bmp);

			hI = (HBITMAP)LoadImage(NULL, TEXT("i.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hI, sizeof(bmp), &bmp);

			hJ = (HBITMAP)LoadImage(NULL, TEXT("j.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hJ, sizeof(bmp), &bmp);

			hK = (HBITMAP)LoadImage(NULL, TEXT("k.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hK, sizeof(bmp), &bmp);

			hL = (HBITMAP)LoadImage(NULL, TEXT("l.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hL, sizeof(bmp), &bmp);

			hBloco = (HBITMAP)LoadImage(NULL, TEXT("blocoPreto.bmp"), IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE); // carrega a imagem
			GetObject(hBloco, sizeof(bmp), &bmp);

			hdc = GetDC(hWnd);

			bmpDC = CreateCompatibleDC(hdc); 
			bmpInicio = CreateCompatibleDC(hdc);
			bmpFinal = CreateCompatibleDC(hdc);
			bmpA = CreateCompatibleDC(hdc);
			bmpB = CreateCompatibleDC(hdc);
			bmpC = CreateCompatibleDC(hdc);
			bmpD = CreateCompatibleDC(hdc);
			bmpE = CreateCompatibleDC(hdc);
			bmpF = CreateCompatibleDC(hdc);
			bmpG = CreateCompatibleDC(hdc);
			bmpH = CreateCompatibleDC(hdc);
			bmpI = CreateCompatibleDC(hdc);
			bmpJ = CreateCompatibleDC(hdc);
			bmpK = CreateCompatibleDC(hdc);
			bmpL = CreateCompatibleDC(hdc);
			bmpBloco = CreateCompatibleDC(hdc);

			SelectObject(bmpDC, hBmp);
			SelectObject(bmpInicio, hInicio);
			SelectObject(bmpFinal, hFinal);
			SelectObject(bmpA, hA);
			SelectObject(bmpB, hB);
			SelectObject(bmpC, hC);
			SelectObject(bmpD, hD);
			SelectObject(bmpE, hE);
			SelectObject(bmpF, hF);
			SelectObject(bmpG, hG);
			SelectObject(bmpH, hH);
			SelectObject(bmpI, hI);
			SelectObject(bmpJ, hJ);
			SelectObject(bmpK, hK);
			SelectObject(bmpL, hL);
			SelectObject(bmpBloco, hBloco);

			ReleaseDC(hWnd, hdc);
			GetClientRect(hWnd, &rect);

			xBitmap = 20;
			yBitmap = 20;

			hdc = BeginPaint(hWnd, &ps);
			hdc = GetDC(hWnd);
			GetClientRect(hWnd, &rect);
			SetTextColor(hdc, RGB(0, 0, 0));
			SetBkMode(hdc, TRANSPARENT);

			for (int j = 0; j < pont->lin_col; j++) {
				for (int i = 0; i < pont->lin_col; i++) {
					if (pont->tablocal[j][i] == '_') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpDC, 0, 0, SRCCOPY);
						xBitmap += 20;
					} else if (pont->tablocal[j][i] == 'I'){
						//Rectangle(hdc, Lx1, Cy1, Lx2, Cy2); //esquerda, topo, direita, fundo
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpInicio, 0, 0, SRCCOPY);
						xBitmap += 20;
					} else if(pont->tablocal[j][i] == 'F'){
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpFinal, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'X') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpBloco, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'a') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpA, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'b') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpB, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'c') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpC, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'd') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpD, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'e') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpE, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'f') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpF, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'g') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpG, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'h') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpH, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'i') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpI, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'j') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpJ, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'k') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpK, 0, 0, SRCCOPY);
						xBitmap += 20;
					}
					else if (pont->tablocal[j][i] == 'l') {
						BitBlt(hdc, xBitmap, yBitmap, bmp.bmWidth, bmp.bmHeight, bmpL, 0, 0, SRCCOPY);
						xBitmap += 20;
					}

				}
				yBitmap += 20;
				xBitmap = 20;

			for (int i = 0; i < totalPos; i++) {
				rect.left = posicoes[i].xPos;
				rect.top = posicoes[i].yPos;					
			}
			EndPaint(hdc, &ps);

		}
		break;
	}

	case WM_DESTROY:			// Destruir a janela e terminar o programa
		PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}
	return(0);
}
