#pragma once
#include <cmath>
#define OnMenuExit				1
#define OnRules					2
#define AboutProg			    3
#define SetEase					4
#define SetMedium			    5
#define SetHard				    6
#define SetCustom			    7
#define OnButtonCSBOKClick      8
#define OnButtonCSBCancelClick  9
#define TIMER_ID                10
#define OnNewGame               11
#define OnButtonSaveOKClick     12
#define OnButtonSaveCancelClick 13
#define OnButtonStaticsOKClick  14
#define OnStatics               15


ATOM MyStartWindowClass(HINSTANCE hInstance);
ATOM CSBRegisterClass(HINSTANCE hInstance);
ATOM TileRegisterClass(HINSTANCE hInstance);
ATOM SaveRegisterClass(HINSTANCE hInstance);
ATOM StaticsRegisterClass(HINSTANCE hInstance);

LRESULT CALLBACK    SoftMainProcedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    CSBWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    TileWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    SaveWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    StaticsWndProc(HWND, UINT, WPARAM, LPARAM);

void MainWndAddMenu(HWND hWnd);
void CustomSizeBoxAddWidgets(HWND hWnd);
void SaveBoxAddWidgets(HWND hWnd);
void StaticsBoxFunctions(HWND hWnd);

void SaveData(LPCSTR);
void LoadData(LPCSTR);

RECT clientWindow, timerRect, mineCountRect;
DWORD lastTick = 0;
HWND parent,CSBhWnd;
HWND edit1;
HWND edit2;
HWND edit3;
HWND SaveWnd;
HWND editSave;
HWND staticStatistics;
HWND StaticsWnd;

BOOL InitInstance(HINSTANCE, int);
BOOL InitCustomSizeBox(HINSTANCE, int, HWND);
BOOL InitTile(HINSTANCE, int, HWND, int, int);
BOOL InitSaveBox(HINSTANCE, int, HWND);
BOOL InitStaticsBox(HINSTANCE, int, HWND);

bool tilesInitialized = false, timerSet = false, isDebug = false, isWon = false, isEnd = false;
HINSTANCE hInst;
  
const int upperPanelSize = 30;
const int defaultMines = 10;//кол-во стартовых мин
const int defaultCelsX = 10;//кол-во стартовых €чеек по ’
const int defaultCelsY = 10;//кол-во стартовых €чеек по ”
const int CellSideSize = 15;//длина стороны шестиугольника-половина €чейки
const int TileDistance = 2;
const int tileSize = 2 * CellSideSize;//длина €чейки
int activeSizeX = defaultCelsX, activeSizeY = defaultCelsY, activeMines = defaultMines, tileCount = defaultCelsX * defaultCelsY;
const int minMines = 10;
const int minCelsY = 9;
const int maxCelsY = 20;
const int minCelsX = 9;
const int maxCelsX = 30;
int activeWindowSizeX ;
int activeWindowSizeY ;
int oldActiveSizeX = defaultCelsX, oldActiveSizeY = defaultCelsY;
int tilesCovered;
int realMinesLeft, flagMinesLeft;
int lvlType;
int time1;
int time2;
HWND hwnds[maxCelsX][maxCelsY];


enum TileType
{
    Free = 0,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,

    Mine
};
struct TileData
{
    int x, y;
    bool isCovered = true;
    bool isFlagged = false;
    TileType tileType = TileType::Free;
}**tileData;

const COLORREF colourTable[] = {
    RGB(0, 0, 255),
    RGB(0, 127 + 31, 0),
    RGB(255 - 63, 0, 0),
    RGB(0, 0, 127 + 31),
    RGB(127, 0, 0),
    RGB(0, 127 + 31, 127 + 31),
    RGB(0, 0, 0),
    RGB(127, 127, 127)
};