#include <windows.h>
#include <windowsx.h>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <cmath>
#include <ctime>
using namespace std;

//#define DOUBLE_BUFFER  //not optimized
#define TIMER_FREQ 10 //10 milliseconds, a less value means faster
#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

struct Cleaner {
   int x, y, r;
   Cleaner(int x_i, int y_i, int r_i): x(x_i), y(y_i), r(r_i) {}
};

queue<Cleaner*> cleaners;
queue<void*> objects;

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
	 double speed = sqrt(dx*dx + dy*dy), q;
	 for (q = speed; q > 0; q -= 1)
	   if (pow(x + dx*q/speed - p->x, 2) + pow(y + dy*q/speed - p->y, 2) <= (r + 1)*(r + 1) + (p->r + 1)*(p->r + 1)) {
	       cleaners.push(new Cleaner(p->x, p->y, p->r));
	       break;
	    }
         if (q <= 0) 
            objects.push(p);
      }
   }
   void Move(int xlimit, int ylimit) {
      cleaners.push(new Cleaner(x, y, r));
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
    static LPCTSTR szAppName = "balls";
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

    hwnd = CreateWindow(szAppName, (LPCTSTR)"Balls!", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
       CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    for(int i = 0; i < time(0)%777; i++) rand();  //better than srand(timer(0))
    pb = new MainBall(hwnd, 100, 100, floor(rand()*6./RAND_MAX + 5), 0, floor(rand()*10./RAND_MAX + 1), floor(rand()*10./RAND_MAX + 1));
    
    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);
    
    while ( GetMessage(&msg, NULL, 0, 0) ) {
       TranslateMessage(&msg);
       DispatchMessage(&msg);
    } 

    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;
    HDC         hdc;
    RECT        s;
    static HPEN        hPen;
    static HBRUSH      hBrush;
    const int timer_id = 0;
    
    switch (iMsg) {
    case WM_CREATE:
       SetTimer(hwnd, timer_id, TIMER_FREQ, NULL);
       hBrush = CreateSolidBrush(RGB(250, 0, 0));
       hPen = CreatePen(PS_SOLID, 1, RGB(250, 10, 5));
       return 0;

    case WM_TIMER:
       switch (wParam) { 
          case timer_id: 
	     GetClientRect(hwnd, &s);
             pb->Move(s.right, s.bottom);
	     InvalidateRect(hwnd, NULL, FALSE);
             return 0; 
    }
    break;

    case WM_PAINT: {
       char s[80];
       hdc = BeginPaint(hwnd, &ps);

#ifdef DOUBLEBUFFER
       HDC hdcMem = CreateCompatibleDC(hdc);
       HBITMAP hbmMem = CreateCompatibleBitmap(hdc, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
       HANDLE hOld = SelectObject(hdcMem, hbmMem);
       Rectangle(hdcMem, 0, 0, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
#else
       HDC hdcMem = hdc;
#endif
       
       SelectObject(hdcMem, GetStockObject(WHITE_PEN));
       Rectangle(hdcMem, 30, 1, 95, 40);  //clear transparent text area
       while (!cleaners.empty()) {
          Cleaner *b = cleaners.front();
	  cleaners.pop();
          Ellipse(hdcMem, b->x - b->r - 1, b->y - b->r - 1, b->x + b->r + 1, b->y + b->r + 1);
          delete b;
       }
       for (int i = 0; i < objects.size(); i++) {
	  StaticBall *p = (StaticBall*) objects.front();
	  objects.pop();
	  objects.push(p);
	  if (p->color == 0) {
	     SelectObject(hdcMem, GetStockObject(BLACK_BRUSH));
	     SelectObject(hdcMem, GetStockObject(BLACK_PEN));
	  }
	  else {
	     SelectObject(hdcMem, hBrush);
	     SelectObject(hdcMem, hPen);
	  }
	  Ellipse(hdcMem, p->x - p->r, p->y - p->r, p->x + p->r, p->y + p->r);
       }
       SetBkMode (hdcMem, TRANSPARENT);
       sprintf(s, "Objects = %d  ", objects.size());
       TextOut(hdcMem, 0, 0, s, strlen(s));
       sprintf(s, "Speed = %.2f  ", sqrt(pb->dx*pb->dx + pb->dy*pb->dy));
       TextOut(hdcMem, 0, 20, s, strlen(s));

#ifdef DOUBLEBUFFER
       BitBlt(hdc, 0, 0, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top, hdcMem, 0, 0, SRCCOPY);
       SelectObject(hdcMem, hOld);
       DeleteObject(hbmMem);
       DeleteDC(hdcMem);
#endif
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
       KillTimer(hwnd, timer_id);
       PostQuitMessage(0);
       return 0;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
