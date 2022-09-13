#include <Windows.h>
#include "Functions.h"
#include <math.h> //для формы ячеек
#include <stdio.h>


void InitConfig()
{
	flagMinesLeft = activeMines;
	realMinesLeft = activeMines;
	tilesCovered = tileCount = activeSizeX * activeSizeY;
	activeWindowSizeX = activeSizeX * (CellSideSize * 18 / 10) + (activeSizeX - 1) * TileDistance;;
	activeWindowSizeY = activeSizeY * tileSize + (activeSizeY - 1) * TileDistance + upperPanelSize + tileSize / 2;
	clientWindow.left = GetSystemMetrics(SM_CXSCREEN) / 2 - activeWindowSizeX / 2;
	clientWindow.top = GetSystemMetrics(SM_CYSCREEN) / 2 - activeWindowSizeY / 2;
	clientWindow.right = clientWindow.left + activeWindowSizeX;
	clientWindow.bottom = clientWindow.top + activeWindowSizeY;
	AdjustWindowRect(&clientWindow, WS_OVERLAPPEDWINDOW, TRUE);
}
void CreateTiles(HINSTANCE hInst, int nCmd, HWND hWnd)
{
	int maxActiveSizeX = max(activeSizeX, oldActiveSizeX);
	int maxActiveSizeY = max(activeSizeY, oldActiveSizeY);

	tileData = new TileData * [activeSizeX];
	for (int i = 0; i < maxActiveSizeX; i++)
	{
		if (i < activeSizeX)tileData[i] = new TileData[activeSizeY];
		for (int j = 0; j < maxActiveSizeY; j++)
		{
			if (i < oldActiveSizeX && j < oldActiveSizeY) DestroyWindow(hwnds[i][j]);
			if (i < activeSizeX && j < activeSizeY) InitTile(hInst, nCmd, hWnd, i, j);
		}
	}
	oldActiveSizeX = activeSizeX;
	oldActiveSizeY = activeSizeY;
} 
void Propagate(int x, int y)
{
	if (x >= 0 && x < activeSizeX && y >= 0 && y < activeSizeY && tileData[x][y].isCovered == true)
	{
		tilesCovered--;
		tileData[x][y].isCovered = false;

		InvalidateRect(hwnds[x][y], NULL, TRUE);
		InvalidateRect(parent, &mineCountRect, TRUE);
		if (x % 2 != 0) {
			if (tileData[x][y].tileType == TileType::Free)
			{
				Propagate(x - 1, y);
				Propagate(x + 1, y);
				Propagate(x, y - 1);
				Propagate(x, y + 1);
				Propagate(x - 1, y + 1);
				Propagate(x + 1, y + 1);
			}
		}
		else {
			if (tileData[x][y].tileType == TileType::Free)
			{
				Propagate(x - 1, y);
				Propagate(x + 1, y);
				Propagate(x, y - 1);
				Propagate(x, y + 1);
				Propagate(x - 1, y - 1);
				Propagate(x + 1, y - 1);
			}
		}
	}
}
void adjustTiles(int x, int y)
{
	if (x >= 0 && x < activeSizeX && y >= 0 && y < activeSizeY)
		if (tileData[x][y].tileType != TileType::Mine)
			tileData[x][y].tileType = (TileType)(tileData[x][y].tileType + 1);
}
void InitTileTypes()
{
	int x, y;
	for (int i = 0; i < activeMines; i++)
	{
		do
		{
			x = rand() % activeSizeX;
			y = rand() % activeSizeY;
		} 
		while (tileData[x][y].tileType == TileType::Mine);

		tileData[x][y].tileType = TileType::Mine;
		if (x % 2 != 0) {
			adjustTiles(x, --y);
			adjustTiles(++x, ++y);
			adjustTiles(x, ++y);
			adjustTiles(--x, y);
			adjustTiles(--x, y);
			adjustTiles(x, --y);
		}
		else
		{
			adjustTiles(x, ++y);
			adjustTiles(++x, --y);
			adjustTiles(x, --y);
			adjustTiles(--x, y);
			adjustTiles(--x, y);
			adjustTiles(x, ++y);
		}
	}
	for (int i = 0; i < activeSizeX; i++)
		for (int j = 0; j < activeSizeY; j++)
		{
			SetPropA(hwnds[i][j], "tileData", &tileData[i][j]);
		}
	tilesInitialized = true;
}
void DeleteTileTypes()
{
	for (int i = 0; i < oldActiveSizeX; i++)
		delete[] tileData[i];
	delete[] tileData;
}
void NewGame()
{
	if (tilesInitialized == true)
		DeleteTileTypes();
	tilesInitialized = false;

	KillTimer(parent, TIMER_ID);
	timerSet = false;
	lastTick = 0;
	isWon = false;
	isEnd = false;
	InitConfig();
	MoveWindow(parent, clientWindow.left, clientWindow.top, clientWindow.right - clientWindow.left, clientWindow.bottom - clientWindow.top, TRUE);
	CreateTiles(hInst, TRUE, parent);
	InitTileTypes();

	InvalidateRect(parent, &timerRect, TRUE);
	InvalidateRect(parent, &mineCountRect, TRUE);
	for (int i = 0; i < activeSizeX; i++)
		for (int j = 0; j < activeSizeY; j++)
		{
			InvalidateRect(hwnds[i][j], NULL, TRUE);
		}

}
void gameEnd()
{
	isEnd = true;
	for (int i = 0; i < activeSizeX; i++)
		for (int j = 0; j < activeSizeY; j++)
		{
			tileData[i][j].isCovered = false;
		}
	KillTimer(parent, TIMER_ID);
	if (isWon) {
		MessageBox(parent, L"Поле разминировано!\nВы выиграли!", L"Победа!", MB_ICONASTERISK | MB_OK);
		if (MessageBox(parent, L"Хотите ли сохранить результат?", L"Вопрос", MB_YESNO) == IDYES) {
			InitSaveBox(hInst, TRUE, parent);
		};
	}
	else if (MessageBox(parent, L"Ба-бах!!!\nВы проиграли!\nХотите начать заново?", L"Неудача!", MB_ICONERROR | MB_YESNO) == IDYES) {
		NewGame();
	};
}



int WINAPI  WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,int nCmdShow) {
	
	MyStartWindowClass(hInstance);
	CSBRegisterClass(hInstance);
	TileRegisterClass(hInstance);
	SaveRegisterClass(hInstance);
	StaticsRegisterClass(hInstance);
	 
	if (!InitInstance(hInstance, nCmdShow)) { return -1; }
	NewGame();
	lvlType = 1;
	ShowWindow(parent, nCmdShow);
	MSG SoftwareMainMessage;

	
	while (GetMessage(&SoftwareMainMessage, NULL, 0, 0)) {
		TranslateMessage(&SoftwareMainMessage);
		DispatchMessageW(&SoftwareMainMessage);
	}
	return 0;
}

ATOM MyStartWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = SoftMainProcedure;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_QUESTION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"parentWnd";
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_QUESTION);

	return RegisterClassExW(&wcex);
}
ATOM CSBRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CSBWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_QUESTION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"CSBChild";
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_QUESTION);

	return RegisterClassExW(&wcex);
}
ATOM TileRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = TileWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(0, 150, 250)));
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Child";
	wcex.hIconSm = NULL;

	return RegisterClassExW(&wcex);
}
ATOM SaveRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = SaveWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_QUESTION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"SaveChild";
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_QUESTION);

	return RegisterClassExW(&wcex);
}
ATOM StaticsRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = StaticsWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_QUESTION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"StaticsChild";
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_QUESTION);

	return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	parent = CreateWindowW(L"parentWnd", L"Шестигранный сапер", WS_OVERLAPPEDWINDOW  /* ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX*/ | WS_VISIBLE,
		clientWindow.left, clientWindow.top, clientWindow.right - clientWindow.left, clientWindow.bottom - clientWindow.top, 0, nullptr, nullptr, hInstance, nullptr);

	if (!parent)
	{
		return FALSE;
	}

	UpdateWindow(parent);

	return TRUE;
}
BOOL InitCustomSizeBox(HINSTANCE hInstance, int nCmdShow, HWND parentHWND) {
	hInst = hInstance;
	CSBhWnd = CreateWindowW(L"CSBChild", L"Особые настройки", WS_OVERLAPPED, (GetSystemMetrics(SM_CXSCREEN) - 400) / 2, (GetSystemMetrics(SM_CYSCREEN) - 250) / 2, 400, 250, parent, nullptr, hInst, nullptr);

	if (!CSBhWnd) {
		return FALSE;
	}
	ShowWindow(CSBhWnd, nCmdShow);
	UpdateWindow(CSBhWnd);

	return TRUE;
}
BOOL InitTile(HINSTANCE hInstance, int nCmdShow, HWND parentHWND, int i, int j)
{
	hInst = hInstance; // Store instance handle in our global variable
	
	POINT pnt[6];
	pnt[0].x = 0;
	pnt[0].y = CellSideSize ;
	pnt[1].x = CellSideSize / 2;
	pnt[1].y = pnt[0].y + (CellSideSize * 2 * (sqrt(3) / 4));
	pnt[2].x = pnt[1].x + CellSideSize;
	pnt[2].y = pnt[1].y;
	pnt[3].x = pnt[2].x + CellSideSize / 2;
	pnt[3].y = pnt[0].y;
	pnt[4].x = CellSideSize;
	pnt[4].y = -(pnt[1].y);
	pnt[5].x = CellSideSize ;
	pnt[5].y = pnt[4].y;
	HRGN rgn2 = CreatePolygonRgn(pnt, 6, ALTERNATE);
	HWND hWnd;
	if (i == 0) {
		if (i % 2 != 0) {
			hWnd = CreateWindowW(L"Child", L"Tile", WS_CHILD,
				i * (tileSize + TileDistance), j * (tileSize + TileDistance) + (tileSize / 2) + upperPanelSize, tileSize, tileSize, parentHWND, nullptr, hInstance, nullptr);
		}
		else {
			hWnd = CreateWindowW(L"Child", L"Tile", WS_CHILD,
				i * (tileSize + TileDistance), j * (tileSize + TileDistance) + upperPanelSize, tileSize, tileSize, parentHWND, nullptr, hInstance, nullptr);
		}
	}
	else {
		if (i % 2 != 0) {
			hWnd = CreateWindowW(L"Child", L"Tile", WS_CHILD,
				i * ((tileSize + TileDistance) - tileSize / 7), j * (tileSize + TileDistance) + (tileSize / 2) + upperPanelSize, tileSize, tileSize, parentHWND, nullptr, hInstance, nullptr);
		}
		else {
			hWnd = CreateWindowW(L"Child", L"Tile", WS_CHILD,
				i * ((tileSize + TileDistance) - tileSize / 7), j * (tileSize + TileDistance) + upperPanelSize, tileSize, tileSize, parentHWND, nullptr, hInstance, nullptr);
		}
	};

	if (!hWnd)
	{
		return FALSE;
	}

	hwnds[i][j] = hWnd;
	tileData[i][j].x = i;
	tileData[i][j].y = j;

	SetWindowRgn(hWnd, rgn2, true);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}
BOOL InitSaveBox(HINSTANCE hInstance, int nCmdShow, HWND parentHWND) {
	hInst = hInstance;
	SaveWnd = CreateWindowW(L"SaveChild", L"Сохранение результата", WS_OVERLAPPED, (GetSystemMetrics(SM_CXSCREEN) - 280) / 2, (GetSystemMetrics(SM_CYSCREEN) - 150) / 2, 280, 150, parentHWND, nullptr, hInst, nullptr);


	if (!SaveWnd) {
		return FALSE;
	}

	ShowWindow(SaveWnd, nCmdShow);
	UpdateWindow(SaveWnd);

	return TRUE;
}
BOOL InitStaticsBox(HINSTANCE hInstance, int nCmdShow, HWND parentHWND) {
	hInst = hInstance;
	StaticsWnd = CreateWindowW(L"StaticsChild", L"Статистика", WS_OVERLAPPED, (GetSystemMetrics(SM_CXSCREEN) - 500) / 2, (GetSystemMetrics(SM_CYSCREEN) - 350) / 2, 500, 350, parentHWND, nullptr, hInst, nullptr);
	if (!StaticsWnd) {
		return FALSE;
	}

	ShowWindow(StaticsWnd, nCmdShow);
	UpdateWindow(StaticsWnd);
}

LRESULT CALLBACK SoftMainProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg)
	{
	case WM_COMMAND:
		switch (wp)
		{
		case OnMenuExit:
			DestroyWindow(hWnd);
			break;
		case OnRules:
			MessageBoxA(hWnd, "В начале игры на поле спрятаны мины. Цель игры — определить (или угадать), где находятся мины.\n\nДля начала игры нажмите на любую неоткрытую ячейку, вместе с этим запускается таймер. Если Вам повезло, и Вы не наткнулись на мину, то увидите в ячейке(-ах) число(-а). Число в ячейке обозначает число окружающих её мин (сюда входят все 6 ячеек, граничащих с ней). На основании этого можно определить, в каких ячейках находятся мины.\n\nЧтобы отметить ячейку, как содержащую мину, нажмите на неё правой кнопкой мыши. Игра завершается, как только будут правильно отмечены все мины или открыты все ячейки, не содержащие мины.",
				"Правила игры", MB_OK);
			//RuleBoxAddWidgets(hWnd);
			//MessageBoxW(hWnd,)
			break;
		case AboutProg:
			MessageBox(hWnd, L"Игра Шестигранный сапер\nСоздано студентом ИУК5-42Б\nЗавитаевым Яном", L"Об игре <<Шестигранный сапер>>!", MB_OK);
			break;
		case OnStatics:
			InitStaticsBox(hInst, TRUE, hWnd);
			break;
		case SetEase:
			lvlType = 1;
			activeMines = 10;
			activeSizeX = 10;
			activeSizeY = 10;

			NewGame();
			break;
		case SetMedium:
			lvlType = 2;
			activeMines = 40;
			activeSizeX = 16;
			activeSizeY = 16;

			NewGame();
			break;
		case SetHard:
			lvlType = 3;
			activeMines = 99;
			activeSizeX = 30;
			activeSizeY = 20;
			NewGame();
			break;
		case SetCustom:
			lvlType = 4;
			InitCustomSizeBox(hInst, TRUE, parent);
			NewGame();
			break;
		case OnNewGame:
			NewGame();
			break;
		default:
			break;
		}

		break;
	case WM_CREATE:
		MainWndAddMenu(hWnd);
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		HFONT hFont;
		char s1[] = "0000.0";
		char s2[] = "0000";
		RECT rc;
		GetClientRect(hWnd, &rc);
		timerRect.left = rc.left;
		timerRect.right = rc.left + (rc.right - rc.left) / 2;
		timerRect.top = rc.top;
		timerRect.bottom = rc.top + upperPanelSize;
		mineCountRect.left = rc.left + (rc.right - rc.left) / 2;
		mineCountRect.right = rc.right;
		mineCountRect.top = rc.top;
		mineCountRect.bottom = rc.top + upperPanelSize;

		if (lastTick != 0) time1 = GetTickCount64() - lastTick;
		snprintf(s1, strlen(s1) + 1, "%04d.%d", time1 / 1000, time1 % 1000 / 100);
		snprintf(s2, strlen(s2) + 1, "%04d", flagMinesLeft);

		time2 = time1;


		hFont = CreateFont(25, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
		SelectObject(hdc, hFont);
		SetTextColor(hdc, RGB(0, 0, 0));


		DrawTextExA(hdc, s1, strlen(s1), &timerRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE, NULL);
		DrawTextExA(hdc, s2, strlen(s2), &mineCountRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE, NULL);

		DeleteObject(hFont);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
	break;
	case WM_TIMER:
		InvalidateRect(parent, &timerRect, TRUE);
		break;
	case WM_CLOSE:
		DeleteTileTypes();
		KillTimer(parent, TIMER_ID);
		DestroyWindow(parent);
		break;
	default:
		return DefWindowProc(hWnd, msg, wp, lp);
		break;
	}
}
LRESULT CALLBACK TileWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TileData* tileData;
	switch (message)
	{
	case WM_PAINT:
	{
		tileData = (TileData*)GetPropA(hWnd, "tileData");
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		HBRUSH mineBrush = CreateSolidBrush(RGB(0, 0, 0));
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, mineBrush);

		if (tilesInitialized)
		{
			if (tileData->isFlagged == false && tileData->isCovered == false)
			{
				RECT rc;
				GetClientRect(hWnd, &rc);

				FillRect(hdc, &rc, CreateSolidBrush(RGB(150, 255, 190)));

				if (tileData->tileType == TileType::Mine)
					Ellipse(hdc, 5, 3, tileSize-5 , tileSize - 5);
				if (tileData->tileType >= 1 && tileData->tileType <= 6)
				{
					HFONT hFont;
					hFont = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
					SelectObject(hdc, hFont);

					char cNum[2];
					int iNum = tileData->tileType;
					snprintf(cNum, 2, "%d", iNum);
					SetTextColor(hdc, colourTable[iNum - 1]);
					SetBkColor(hdc, RGB(150, 255, 190));

					DrawTextExA(hdc, cNum, 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE, NULL);
				}
			}
			else if (tileData->isFlagged == true)
			{
				RECT rc;
				GetClientRect(hWnd, &rc);
				FillRect(hdc, &rc, CreateSolidBrush(RGB(250, 0, 0)));
			}
		}
		SelectObject(hdc, &oldBrush);

		DeleteObject(mineBrush);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_LBUTTONDOWN:
		tileData = (TileData*)GetPropA(hWnd, "tileData");
		if (isEnd == true || tileData->isFlagged == true) break;
		if (timerSet == false)
		{
			timerSet = true;
			SetTimer(parent, TIMER_ID, 100, NULL);
			lastTick = GetTickCount64();
		}
		if (tileData->isCovered == true)
		{
			Propagate(tileData->x, tileData->y);
			if (tilesCovered == activeMines)
			{
				isWon = true;
				gameEnd();
			}
			if (tileData->tileType == TileType::Mine)
				gameEnd();
		}
		break;
	case WM_RBUTTONDOWN:
		if (isEnd == true) break;
		tileData = (TileData*)GetPropA(hWnd, "tileData");
		if (timerSet == false)
		{
			timerSet = true;
			SetTimer(parent, TIMER_ID, 100, NULL);
			lastTick = GetTickCount64();
		}
		if (tileData->isCovered == true)
		{
			if (tileData->isFlagged == true)
			{
				tileData->isFlagged = false;
				flagMinesLeft++;
				if (tileData->tileType == TileType::Mine)
					realMinesLeft++;
			}
			else
			{
				tileData->isFlagged = true;
				flagMinesLeft--;
				if (tileData->tileType == TileType::Mine)
					realMinesLeft--;
			}
			InvalidateRect(hWnd, NULL, TRUE);
			InvalidateRect(parent, &mineCountRect, TRUE);
			if (realMinesLeft == 0 && flagMinesLeft == 0)
			{
				isWon = true;
				gameEnd();
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
LRESULT CALLBACK CSBWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId;
	char textBuf[5];
	switch (message)
	{

	case WM_CREATE:
		CustomSizeBoxAddWidgets(hWnd);
		break;
	case WM_COMMAND:

		wmId = LOWORD(wParam);

		switch (wmId) {
		case OnButtonCSBOKClick:
			int sizeX, sizeY, mines;
			GetWindowTextA(edit1, textBuf, 5);
			sizeX = atoi(textBuf);
			GetWindowTextA(edit2, textBuf, 5);
			sizeY = atoi(textBuf);
			GetWindowTextA(edit3, textBuf, 5);
			mines = atoi(textBuf);

			if (sizeX < minCelsX || sizeX > maxCelsX || sizeY < minCelsY || sizeY > maxCelsY || mines < minMines || mines > sizeX * sizeY)
				break;

			activeMines = mines;
			activeSizeX = sizeX;
			activeSizeY = sizeY;

			NewGame();
			DestroyWindow(hWnd);
			break;
		case OnButtonCSBCancelClick:
			DestroyWindow(hWnd);
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
LRESULT CALLBACK SaveWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId;

	switch (message)
	{
	case WM_CREATE:
		SaveBoxAddWidgets(hWnd);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		switch (wmId) {
		case OnButtonSaveOKClick:
			SaveData("StatisticsMinesweeper.dat");
			DestroyWindow(hWnd);
			break;
		case OnButtonSaveCancelClick:
			DestroyWindow(hWnd);
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
LRESULT CALLBACK StaticsWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId;

	switch (message)
	{
	case WM_CREATE:
		StaticsBoxFunctions(hWnd);
		LoadData("StatisticsMinesweeper.dat");
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		switch (wmId) {
		case OnButtonStaticsOKClick:
			DestroyWindow(hWnd);
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


void MainWndAddMenu(HWND hWnd) {
	HMENU RootMenu = CreateMenu();
	HMENU SubMenuFile = CreateMenu();
	HMENU SubMenuHelp = CreateMenu();
	HMENU SubMenuChooseLVL = CreateMenu();

	AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenuFile, L"Игра");
	AppendMenu(RootMenu, MF_POPUP, (UINT_PTR)SubMenuHelp, L"Справка");

	AppendMenu(SubMenuFile, MF_STRING, OnNewGame, L"Новая игра");
	AppendMenu(SubMenuFile, MF_POPUP, (UINT_PTR)SubMenuChooseLVL, L"Выбор уровня");

	AppendMenu(SubMenuChooseLVL, MF_STRING, SetEase, L"Легкий");
	AppendMenu(SubMenuChooseLVL, MF_STRING, SetMedium, L"Средний");
	AppendMenu(SubMenuChooseLVL, MF_STRING, SetHard, L"Сложный");
	AppendMenu(SubMenuChooseLVL, MF_SEPARATOR, NULL, NULL);
	AppendMenu(SubMenuChooseLVL, MF_STRING, SetCustom, L"Особый");

	AppendMenu(SubMenuFile, MF_SEPARATOR, NULL, NULL);
	AppendMenu(SubMenuFile, MF_STRING, OnMenuExit, L"Выход");

	AppendMenu(SubMenuHelp, MF_STRING, OnRules, L"Правила игры");
	AppendMenu(SubMenuHelp, MF_STRING, OnStatics, L"Статистика");
	AppendMenu(SubMenuHelp, MF_STRING, AboutProg, L"О пограмме");

	SetMenu(hWnd, RootMenu);

}
void CustomSizeBoxAddWidgets(HWND hWnd) {
	CreateWindowA("button", "Подтвердить", WS_VISIBLE | WS_CHILD | ES_CENTER, 40, 160, 120, 30, hWnd, (HMENU)OnButtonCSBOKClick, nullptr, nullptr);
	CreateWindowA("button", "Отменить", WS_VISIBLE | WS_CHILD | ES_CENTER, 250, 160, 80, 30, hWnd, (HMENU)OnButtonCSBCancelClick, nullptr, nullptr);
	CreateWindowA("static", "Ширина(Ш)", WS_VISIBLE | WS_CHILD, 25, 25, 120, 30, hWnd, nullptr, nullptr, nullptr);
	CreateWindowA("static", "Высота(В)", WS_VISIBLE | WS_CHILD, 25, 65, 120, 30, hWnd, nullptr, nullptr, nullptr);
	CreateWindowA("static", "Количество мин", WS_VISIBLE | WS_CHILD, 25, 105, 120, 30, hWnd, nullptr, nullptr, nullptr);
	CreateWindowA("static", "Мин: 9, Макс: 30", WS_VISIBLE | WS_CHILD, 200, 25, 120, 30, hWnd, nullptr, nullptr, nullptr);
	CreateWindowA("static", "Мин: 9, Макс: 20", WS_VISIBLE | WS_CHILD, 200, 65, 120, 30, hWnd, nullptr, nullptr, nullptr);
	CreateWindowA("static", "Мин: 10, Макс: В*Ш", WS_VISIBLE | WS_CHILD, 200, 105, 200, 30, hWnd, nullptr, nullptr, nullptr);
	edit1 = CreateWindowA("edit", "9", WS_VISIBLE | WS_CHILD, 150, 25, 50, 30, hWnd, nullptr, nullptr, nullptr);
	edit2 = CreateWindowA("edit", "9", WS_VISIBLE | WS_CHILD, 150, 65, 50, 30, hWnd, nullptr, nullptr, nullptr);
	edit3 = CreateWindowA("edit", "10", WS_VISIBLE | WS_CHILD, 150, 105, 50, 30, hWnd, nullptr, nullptr, nullptr);
}
void SaveBoxAddWidgets(HWND hWnd) {
	CreateWindowA("static", "Введите имя: ", WS_VISIBLE | WS_CHILD, 15, 20, 110, 30, hWnd, nullptr, nullptr, nullptr);
	editSave = CreateWindowA("edit", "", WS_VISIBLE | WS_CHILD, 130, 20, 110, 20, hWnd, nullptr, nullptr, nullptr);
	CreateWindowA("button", "Сохранить", WS_VISIBLE | WS_CHILD | ES_CENTER, 30, 60, 90, 30, hWnd, (HMENU)OnButtonSaveOKClick, nullptr, nullptr);
	CreateWindowA("button", "Отмена", WS_VISIBLE | WS_CHILD | ES_CENTER, 130, 60, 70, 30, hWnd, (HMENU)OnButtonSaveCancelClick, nullptr, nullptr);
}
void SaveData(LPCSTR path) {

	HANDLE FileToSave = CreateFileA(path, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	int saveLengthName = GetWindowTextLength(editSave) + 1;
	char* dataName = new char[saveLengthName];
	char* dataTime = new char[7];

	snprintf(dataTime, strlen(dataTime) + 1, "%04d.%d", time2 / 1000, time2 % 1000 / 100);

	GetWindowTextA(editSave, dataName, saveLengthName);

	DWORD size = GetFileSize(FileToSave, nullptr);

	if (size != 0) {
		SetFilePointer(FileToSave, -1, NULL, 2);
	}

	WriteFile(FileToSave, "Имя: ", 5, nullptr, nullptr);
	WriteFile(FileToSave, dataName, saveLengthName - 1, nullptr, nullptr);
	WriteFile(FileToSave, " Время: ", 8, nullptr, nullptr);
	WriteFile(FileToSave, dataTime, 6, nullptr, nullptr);
	WriteFile(FileToSave, "с Уровень: ", 11, nullptr, nullptr);
	switch (lvlType) {
	case 1:
		WriteFile(FileToSave, "Легкий\n\0", 8, nullptr, nullptr);
		break;
	case 2:
		WriteFile(FileToSave, "Средний\n\0", 9, nullptr, nullptr);
		break;
	case 3:
		WriteFile(FileToSave, "Сложный\n\0", 9, nullptr, nullptr);
		break;
	case 4:
		WriteFile(FileToSave, "Особый\n\0", 8, nullptr, nullptr);
		break;
	}
	CloseHandle(FileToSave);
	delete[] dataName;
	delete[] dataTime;
}

void StaticsBoxFunctions(HWND hWnd) {
	staticStatistics = CreateWindowA("static", "Ни одной записи нет", WS_VISIBLE | WS_CHILD, 10, 10, 450, 200, hWnd, nullptr, nullptr, nullptr);
	CreateWindowA("button", "OK", WS_VISIBLE | WS_CHILD, 215, 270, 60, 30, hWnd, (HMENU)OnButtonStaticsOKClick, nullptr, nullptr);
}

void LoadData(LPCSTR path) {
	HANDLE FileToLoad = CreateFileA(path, GENERIC_READ, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	DWORD size = GetFileSize(FileToLoad, nullptr);
	char* text = new char[size];
	if (size != 0) {
		ReadFile(FileToLoad, text, size + 1, nullptr, nullptr);
		SetWindowTextA(staticStatistics, text);
	}

	CloseHandle(FileToLoad);
	delete[] text;
}