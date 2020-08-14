#include <windows.h>
#include <stdint.h>

#define internal static
#define global static
#define local_persist static

#define assert(expression) if(!(expression)) {*(s32*)0 = 0;}

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

global bool gAppIsRunning;

struct Framebuffer
{
  BITMAPINFO bitmapInfo;
  void *memory;
  s32 width;
  s32 height;
  s32 stride;
  s32 bytesPerPixel;
};

global Framebuffer gFramebuffer;

struct Window_dim
{
  s32 width;
  s32 height;
};

internal Window_dim
getWindowDim(HWND window)
{
  Window_dim result = {};
  RECT rect;
  GetClientRect(window, &rect);
  result.width  = rect.right -  rect.left;
  result.height = rect.bottom - rect.top;
  
  return result;
}

internal void
createFramebuffer(Framebuffer *framebuffer, s32 width, s32 height)
{
  if(framebuffer->memory)
    {
      VirtualFree(framebuffer->memory, 0, MEM_RELEASE);
    }
  
  framebuffer->width  = width;  
  framebuffer->height = height;
  framebuffer->bytesPerPixel = 4;
  
  framebuffer->bitmapInfo.bmiHeader = {};  
  
  framebuffer->bitmapInfo.bmiHeader.biSize = sizeof(framebuffer->bitmapInfo.bmiHeader);
  framebuffer->bitmapInfo.bmiHeader.biWidth = framebuffer->width;
  framebuffer->bitmapInfo.bmiHeader.biHeight = -framebuffer->height;
  framebuffer->bitmapInfo.bmiHeader.biPlanes = 1;
  framebuffer->bitmapInfo.bmiHeader.biBitCount = 32;
  framebuffer->bitmapInfo.bmiHeader.biCompression = BI_RGB;
  
  s32 framebufferSize = framebuffer->width * framebuffer->height * framebuffer->bytesPerPixel;
  
  framebuffer->memory = VirtualAlloc(0,(size_t)framebufferSize,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);
  framebuffer->stride = framebuffer->width * framebuffer->bytesPerPixel;
  
  s8 *destRow = (s8*)framebuffer->memory;
  
  for(u32 y = 0; y < framebuffer->height; y++)
    {
      s8 *pixel = destRow;
      for(u32 x = 0; x < framebuffer->width; x++)
	{
	  *pixel++ = 255;
	  *pixel++ = 0;
	  *pixel++ = 255;
	  *pixel++;
	}
      destRow += framebuffer->stride;
    }
}


internal void
blitBufferToScreen(Framebuffer *framebuffer, HDC deviceContext,s32 width,s32 height)
{
  
  StretchDIBits(
		deviceContext,
		0,0, width, height,
		0,0, framebuffer->width, framebuffer->height,
		framebuffer->memory,
		&framebuffer->bitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
		);
}

LRESULT handlingWindowMessages(HWND    window,
			       UINT    message,
			       WPARAM  wParam,
			       LPARAM  lParam
			       )
{
  LRESULT result = 0;
  
  switch(message)
    {
    case WM_CREATE:
      {
	OutputDebugStringA("WM_CREATE\n");
      }break;
    case WM_PAINT:
      {
	PAINTSTRUCT ps;
	HDC deviceContext = BeginPaint(window, &ps);

	Window_dim windowDim = getWindowDim(window);
	blitBufferToScreen(&gFramebuffer, deviceContext,windowDim.width,windowDim.height);
	
	EndPaint(window, &ps);
      }break;
    case WM_DESTROY:
      {
	PostQuitMessage(0);
	gAppIsRunning = false;
	OutputDebugStringA("WM_DESTROY\n");
      }break;
    case WM_QUIT:
      {
	gAppIsRunning = false;
	OutputDebugStringA("WM_QUIT\n");
      }break;
    case WM_KEYUP:
    case WM_KEYDOWN:
      {
	assert("Must not happen!");
      }break;
    default:
      {
	return DefWindowProc(window, message, wParam, lParam);
      }break;
    }
  
  return result;
}

internal void
handleInputMessages()
{
  MSG message;
  while(PeekMessageA(&message,0,0,0,PM_REMOVE))
    {
      switch(message.message)
	{
	case WM_KEYUP:
	case WM_KEYDOWN:
	  {
	    bool keyIsDown  = ((message.lParam & (1 << 31)) == 0);
	    bool keyWasDown = ((message.lParam & (1 << 30)) == 1);
	    if(keyIsDown != keyWasDown)
	      {
		switch(message.wParam)
		  {
		  case 'W':
		    {
		      OutputDebugStringA("W\n");
		    }break;
		  case 'A':
		    {
		      OutputDebugStringA("A\n");
		    }break;
		  case 'S':
		    {
		      OutputDebugStringA("S\n");
		    }break;
		  case 'D':
		    {
		      OutputDebugStringA("D\n");
		    }break;
		  }
	      }break;
	  }
	default:
	  {
	    TranslateMessage(&message);
	    DispatchMessage(&message);
	  }break;
	}	  
    }
}


int  WinMain(
	     HINSTANCE hInstance,
	     HINSTANCE hPrevInstance,
	     LPSTR     lpCmdLine,
	     int       nShowCmd
	     )
{
  createFramebuffer(&gFramebuffer, 1280, 980);
  
  char *className = "appClassName";
  
  WNDCLASS windowClass = {};
  windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
  windowClass.lpfnWndProc = &handlingWindowMessages;
  windowClass.hInstance = hInstance;
  windowClass.lpszClassName = className;

  RegisterClassA(&windowClass);
  
  HWND window = CreateWindowExA(0,
				className,
				"app",
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				hInstance,
				0);
  if(window)
    {
      MSG msg;
      gAppIsRunning = true;
      HDC deviceContext = GetDC(window);
      while(gAppIsRunning)
	{
	  handleInputMessages();
	  Window_dim windowDim = getWindowDim(window);
	  blitBufferToScreen(&gFramebuffer,deviceContext,windowDim.width,windowDim.height);
	}
    }
  return 0;
}
