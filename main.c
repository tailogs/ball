#include <windows.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>

int   BALL_RADIUS   = 20;
int   SHADOW_OFFSET = 8;
float ballDX        = 400.0f;
float ballDY        = 300.0f;
int   ignoreTaskbar = 1;
int   enableSound   = 1;
int   ballColor     = 0xFFFFFFFF;
int   volumePercent = 30;

#define MAX_PATH_LEN 260
char soundPath[MAX_PATH_LEN] = "sounds\\collision.wav";

#define WIN_W (BALL_RADIUS*2 + SHADOW_OFFSET*2)
#define WIN_H (BALL_RADIUS*2 + SHADOW_OFFSET*2)

float ballX = 500, ballY = 300;

HWND hwnd;
HDC hdcMem;
HBITMAP hBitmap;
BLENDFUNCTION blend;

int screenW, screenH;

NOTIFYICONDATA nid = {0};
#define WM_TRAYICON (WM_USER + 1)

void LoadConfig(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return;
    char key[64], val[128];
    while (fscanf(f, "%63[^=]=%127[^\n]\n", key, val) == 2) {
        if (strcmp(key, "ball_radius") == 0) BALL_RADIUS = atoi(val);
        else if (strcmp(key, "ball_speed_x") == 0) ballDX = (float)atof(val);
        else if (strcmp(key, "ball_speed_y") == 0) ballDY = (float)atof(val);
        else if (strcmp(key, "ignore_taskbar") == 0) ignoreTaskbar = atoi(val);
        else if (strcmp(key, "enable_sound") == 0) enableSound = atoi(val);
        else if (strcmp(key, "ball_color") == 0) ballColor = (int)strtol(val, NULL, 16);
        else if (strcmp(key, "sound_path") == 0) strncpy(soundPath, val, MAX_PATH_LEN-1);
        else if (strcmp(key, "volume") == 0) volumePercent = atoi(val);
    }
    fclose(f);
    if(volumePercent < 0) volumePercent = 0;
    if(volumePercent > 100) volumePercent = 100;
}

void SetVolume(int percent) {
    DWORD vol = (DWORD)((percent * 0xFFFF) / 100);
    DWORD volAll = (vol & 0xFFFF) | (vol << 16);
    waveOutSetVolume(0, volAll);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            if(LOWORD(wParam) == 1) {
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            return 0;
        case WM_TRAYICON:
            if(lParam == WM_RBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 1, "Exit");
                SetForegroundWindow(hwnd);
                TrackPopupMenu(hMenu, TPM_BOTTOMALIGN|TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
            return 0;
    }
    return DefWindowProc(hwnd,msg,wParam,lParam);
}

void DrawBall() {
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = WIN_W;
    bmi.bmiHeader.biHeight = -WIN_H;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    uint32_t *pixels = (uint32_t*)malloc(WIN_W*WIN_H*sizeof(uint32_t));
    for(int i=0;i<WIN_W*WIN_H;i++) pixels[i]=0x00000000;

    // тень
    for(int y=0;y<WIN_H;y++) {
        for(int x=0;x<WIN_W;x++) {
            float dx0 = x - WIN_W/2;
            float dy0 = y - WIN_H/2 + SHADOW_OFFSET;
            if(dx0*dx0 + dy0*dy0 <= BALL_RADIUS*BALL_RADIUS)
                pixels[y*WIN_W + x] = 0x80000000;
        }
    }

    // мяч
    for(int y=0;y<WIN_H;y++) {
        for(int x=0;x<WIN_W;x++) {
            float dx0 = x - WIN_W/2;
            float dy0 = y - WIN_H/2;
            if(dx0*dx0 + dy0*dy0 <= BALL_RADIUS*BALL_RADIUS)
                pixels[y*WIN_W + x] = ballColor;
        }
    }

    SetDIBits(hdcMem,hBitmap,0,WIN_H,pixels,&bmi,DIB_RGB_COLORS);
    free(pixels);
}

void UpdateLayered() {
    SIZE size = {WIN_W, WIN_H};
    POINT ptSrc = {0,0};
    POINT ptDst = {(LONG)(ballX - WIN_W/2), (LONG)(ballY - WIN_H/2)};
    UpdateLayeredWindow(hwnd,NULL,&ptDst,&size,hdcMem,&ptSrc,0,&blend,ULW_ALPHA);
    SetWindowPos(hwnd, HWND_TOPMOST, (LONG)(ballX - WIN_W/2), (LONG)(ballY - WIN_H/2), WIN_W, WIN_H, SWP_NOACTIVATE);
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrev,LPSTR lpCmdLine,int nShowCmd) {
    LoadConfig("config.ini");
    SetVolume(volumePercent);

    screenW = GetSystemMetrics(SM_CXSCREEN);
    screenH = GetSystemMetrics(SM_CYSCREEN);

    WNDCLASS wc={0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "BallLayer";
    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST,
        "BallLayer","",WS_POPUP,
        (int)(ballX - WIN_W/2),(int)(ballY - WIN_H/2),
        WIN_W,WIN_H,
        NULL,NULL,hInstance,NULL);

    ShowWindow(hwnd,SW_SHOW);

    HDC hdcScreen = GetDC(NULL);
    hdcMem = CreateCompatibleDC(hdcScreen);
    hBitmap = CreateCompatibleBitmap(hdcScreen,WIN_W,WIN_H);
    SelectObject(hdcMem,hBitmap);
    ReleaseDC(NULL,hdcScreen);

    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    lstrcpy(nid.szTip, "Bouncing Ball");
    Shell_NotifyIcon(NIM_ADD, &nid);

    LARGE_INTEGER freq,lastTime,nowTime;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&lastTime);

    MSG msg;
    while(1) {
        while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
            if(msg.message==WM_QUIT) return msg.wParam;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        QueryPerformanceCounter(&nowTime);
        float dt = (float)(nowTime.QuadPart - lastTime.QuadPart)/freq.QuadPart;
        lastTime = nowTime;

        ballX += ballDX*dt;
        ballY += ballDY*dt;

        int collided = 0;
        if(ballX - BALL_RADIUS <0){ ballX = BALL_RADIUS; ballDX=-ballDX; collided=1; }
        if(ballX + BALL_RADIUS >screenW){ ballX=screenW-BALL_RADIUS; ballDX=-ballDX; collided=1; }
        if(ballY - BALL_RADIUS <0){ ballY = BALL_RADIUS; ballDY=-ballDY; collided=1; }
        if(ballY + BALL_RADIUS >screenH){ ballY=screenH-BALL_RADIUS; ballDY=-ballDY; collided=1; }

        if(collided && enableSound){
            PlaySound(TEXT("sounds\\collision.wav"),NULL,SND_FILENAME|SND_ASYNC);
        }

        DrawBall();
        UpdateLayered();
        Sleep(1);
    }

    return 0;
}
