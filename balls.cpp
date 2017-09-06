#include <windows.h>
#include <windowsx.h>
#include <cstdio>
#include <cstdlib>
#include <queue>
using namespace std;

#define ID_TIMER1 0
#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

struct GrCircle {
   int x, y, r;
   GrCircle(int x_i, int y_i, int r_i): x(x_i), y(y_i), r(r_i) {}
};

queue<GrCircle*> balls;
queue<void*> objects;

struct StaticBall {
   int x, y, r, color;
   HWND hwnd;
   StaticBall(HWND hwnd_i, int x_i, int y_i, unsigned r_i, int color_i): x(x_i), y(y_i), r(r_i), color(color_i), hwnd(hwnd_i) {}
   virtual ~StaticBall() {}
   void Draw() {
      balls.push(new GrCircle(x, y, r));
   }
};

struct MainBall: public StaticBall {
   int dx, dy;  //velocity vector
   MainBall(HWND hwnd_i, int x_i, int y_i, unsigned r_i, int color_i, int dx_i, int dy_i): StaticBall(hwnd_i, x_i, y_i, r_i, color_i), dx(dx_i), dy(dy_i) {
      Draw();
      InvalidateRect(hwnd, NULL, FALSE);
      objects.push(this);
   }
   virtual ~MainBall() {}
   void Move(int xlimit, int ylimit) {
      balls.push(new GrCircle(x, y, r));
      /*for (int i = 0; i < objects.size(); i++) {
	 StaticBall *p = (StaticBall*) objects.front();
	 objects.pop();
	 if (p == this) {
	    objects.push(p);
	    continue;
	 }
         objects.push(p);
      }*/
      x += dx;
      y += dy;
      if (x - r < 0 || x + r > xlimit) {
	 dx = -dx;
	 if (x - r < 0)
	    x = r;
	 else
	    x = xlimit - r;
      }
      if (y + r > ylimit || y - r < 0) {
	 dy = -dy;
	 if (y - r < 0)
	    y = r;
	 else
	    y = ylimit - r;
      }
   }
} *pb;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   LPSTR szCmdLine, int iCmdShow) {
    static char szAppName[] = "balls";
    HWND        hwnd;
    MSG         msg;
    WNDCLASSEX  wndclass = {};

    wndclass.cbSize         = sizeof(wndclass);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wndclass.lpszClassName  = szAppName;
    wndclass.lpszMenuName   = NULL;

    RegisterClassEx(&wndclass);

    hwnd = CreateWindow(szAppName, "Balls!",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, hInstance, NULL);
    
    pb = new MainBall(hwnd, 100, 100, 40, BLACK_BRUSH, 7, 1);
    
    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);
    
    while ( GetMessage(&msg, NULL, 0, 0) ) {
	TranslateMessage(&msg);    /*  for certain keyboard messages  */
	DispatchMessage(&msg);     /*  send message to WndProc        */
    } 

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC         hdc;
    RECT        s;

    switch (iMsg) {
    case WM_CREATE:
       SetTimer(hwnd, ID_TIMER1, 20, NULL);
       return 0;

    case WM_TIMER:
       switch (wParam) { 
          case ID_TIMER1: 
	     GetClientRect(hwnd, &s);
             pb->Move(s.right, s.bottom);
	     InvalidateRect(hwnd, NULL, FALSE);
             return 0; 
    }
    break;

    case WM_PAINT:
       hdc = BeginPaint(hwnd, &ps);
       SelectObject(hdc, GetStockObject(WHITE_PEN));
       SelectObject(ps.hdc, GetStockObject(WHITE_PEN));
       while (!balls.empty()) {
          GrCircle *b = balls.front();
	  balls.pop();
          Ellipse(ps.hdc, b->x - b->r, b->y - b->r, b->x + b->r, b->y + b->r);
          delete b;
       }
       for (int i = 0; i < objects.size(); i++) {
	  StaticBall *p = (StaticBall*) objects.front();
	  objects.pop();
	  objects.push(p);
	  SelectObject(ps.hdc, GetStockObject(BLACK_BRUSH));
	  Ellipse(ps.hdc, p->x - p->r, p->y - p->r, p->x + p->r, p->y + p->r);
       }
       EndPaint(hwnd, &ps);
       return 0;

    case WM_LBUTTONDOWN: {
       objects.push(new StaticBall(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 20, BLACK_BRUSH));
       return 0;
    }
    
    case WM_DESTROY:
       KillTimer(hwnd, ID_TIMER1);
       PostQuitMessage(0);
       return 0;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
