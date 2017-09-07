#include <windows.h>
#include <windowsx.h>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <cmath>
#include <ctime>
using namespace std;

#define ID_TIMER1 0
#define FREQ_TIMER1 10

#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

struct GrCircle {
   int x, y, r;
   GrCircle(int x_i, int y_i, int r_i): x(x_i), y(y_i), r(r_i) {}
};

queue<GrCircle*> brushes;
queue<void*> objects;
HPEN        hPen;
HBRUSH      hBrush;
HPEN pen_colors[2] = {};
HBRUSH brush_colors[2] = {};

struct StaticBall {
   double x, y, r;
   int color;
   HWND hwnd;
   StaticBall(HWND hwnd_i, double x_i, double y_i, double r_i, int color_i): x(x_i), y(y_i), r(r_i), color(color_i), hwnd(hwnd_i) {}
   virtual ~StaticBall() {}
};

struct MainBall: public StaticBall {
   double dx, dy;  //velocity vector
   MainBall(HWND hwnd_i, double x_i, double y_i, double r_i, int color_i, double dx_i, double dy_i): StaticBall(hwnd_i, x_i, y_i, r_i, color_i), dx(dx_i), dy(dy_i) {
      objects.push(this);
   }
   virtual ~MainBall() {}
   void new_velocity() {
      double r = (double)rand()*2/RAND_MAX;
      if (r < 0.5) r += 1;
      dx *= r;
      dy *= r;
      while (dx*dx + dy*dy < 1) {
	 dx *= 2;
	 dy *= 2;
      }
      while (dx*dx + dy*dy > 100) {
	 dx /= 2;
	 dy /= 2;
      }
   }
   void CollidingDetector() {
      for (int i = 0; i < objects.size(); i++) {
	 StaticBall *p = (StaticBall*) objects.front();
	 objects.pop();
	 if (p == this) {
	    objects.push(p);
	    continue;
	 }
	 double speed = sqrt(dx*dx + dy*dy);
	 int f = 0;
	 for (double i = speed; i > 0; i -= 1)
	   if (pow(x + dx*i/speed - p->x, 2) + pow(y + dy*i/speed - p->y, 2) <= (r + 1)*(r + 1) + (p->r + 1)*(p->r + 1)) {
	       f = 1;
	       break;
	    }
         if (f) 
	    brushes.push(new GrCircle(p->x, p->y, p->r));
	 else
            objects.push(p);
      }
   }
   void Move(int xlimit, int ylimit) {
      brushes.push(new GrCircle(x, y, r));
      x += dx;
      y += dy;
      CollidingDetector();
      if (x - r < 0 || x + r > xlimit) {
	 dx = -dx;
	 if (x - r < 0)
	    x = r;
	 else
	    x = xlimit - r;
	 new_velocity();
      }
      if (y + r > ylimit || y - r < 0) {
	 dy = -dy;
	 if (y - r < 0)
	    y = r;
	 else
	    y = ylimit - r;
	 new_velocity();
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
    for(int i = 0; i < time(0)%777; i++) rand();  //better than srand(timer(0))
    pb = new MainBall(hwnd, 100, 100, floor(rand()*6./RAND_MAX + 5), 0, floor(rand()*10./RAND_MAX + 1), floor(rand()*10./RAND_MAX + 1));
    
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
       SetTimer(hwnd, ID_TIMER1, FREQ_TIMER1, NULL);
       hBrush = CreateSolidBrush(RGB(250, 0, 0));
       hPen = CreatePen(PS_SOLID, 1, RGB(250, 10, 5));
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

    case WM_PAINT: {
       char s[80];
       hdc = BeginPaint(hwnd, &ps);
       SelectObject(hdc, GetStockObject(WHITE_PEN));
       SelectObject(ps.hdc, GetStockObject(WHITE_PEN));
       while (!brushes.empty()) {
          GrCircle *b = brushes.front();
	  brushes.pop();
          Ellipse(ps.hdc, b->x - b->r - 1, b->y - b->r - 1, b->x + b->r + 1, b->y + b->r + 1);
          delete b;
       }
       for (int i = 0; i < objects.size(); i++) {
	  StaticBall *p = (StaticBall*) objects.front();
	  objects.pop();
	  objects.push(p);
	  if (p->color == 0) {
	     SelectObject(ps.hdc, GetStockObject(BLACK_BRUSH));
	     SelectObject(hdc, GetStockObject(BLACK_PEN));
	  }
	  else {
	     SelectObject(ps.hdc, hBrush);
	     SelectObject(hdc, hPen);
	  }
	  Ellipse(ps.hdc, p->x - p->r, p->y - p->r, p->x + p->r, p->y + p->r);
       }
       sprintf(s, "Objects = %d  ", objects.size());
       TextOut(hdc, 0, 0, s, strlen(s));
       sprintf(s, "Speed = %.2f  ", sqrt(pb->dx*pb->dx + pb->dy*pb->dy));
       TextOut(hdc, 0, 20, s, strlen(s));       
       EndPaint(hwnd, &ps);
       return 0;
    }
    case WM_LBUTTONDOWN: {
       objects.push(new StaticBall(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), floor(96.*rand()/RAND_MAX + 5), 1));
       return 0;
    }
    
    case WM_DESTROY:
       DeleteObject(hPen);
       DeleteObject(hBrush);
       KillTimer(hwnd, ID_TIMER1);
       PostQuitMessage(0);
       return 0;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
