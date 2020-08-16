#include <windows.h>
#include <stdint.h>
#include <gl/gl.h>
#include "mario.h"
#include "mario.cpp"

global bool gAppIsRunning;
#define SOFTWARE_RENDERER 0

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
}

internal void
createOpenglContext(HWND window, HDC deviceContext)
{
  PIXELFORMATDESCRIPTOR pixelFormat = {};

  pixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pixelFormat.nVersion = 1;
  pixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pixelFormat.iPixelType = PFD_TYPE_RGBA;
  pixelFormat.cColorBits = 24;
  pixelFormat.cStencilBits = 8;
  pixelFormat.iLayerType = PFD_MAIN_PLANE;

  s32 pixelFormatIndex = ChoosePixelFormat(deviceContext, &pixelFormat);
  if(SetPixelFormat(deviceContext,pixelFormatIndex,&pixelFormat))
    {
      HGLRC renderingContext = wglCreateContext(deviceContext);
      if(renderingContext)
	{
	  // NOTE(shvayko): SUCCESS
	  bool result = wglMakeCurrent(deviceContext, renderingContext);
	}
      else
	{
	  
	}
    }
  else
    {
      
    }
}

internal void
blitBufferToScreen(Framebuffer *framebuffer, HDC deviceContext,s32 width,s32 height)
{
#if SOFTWARE_RENDERER 
  StretchDIBits(
		deviceContext,
		0,0, width, height,
		0,0, framebuffer->width, framebuffer->height,
		framebuffer->memory,
		&framebuffer->bitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY
		);
#else
  
  SwapBuffers(deviceContext);
#endif
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
			        1280,
				980,
				0,
				0,
				hInstance,
				0);
  if(window)
    {
      MSG msg;
      gAppIsRunning = true;
      HDC deviceContext = GetDC(window);
      createOpenglContext(window,deviceContext);
      while(gAppIsRunning)
	{
	  handleInputMessages();

	  Game_framebuffer gameFramebuffer = {};
	  gameFramebuffer.width = gFramebuffer.width;
	  gameFramebuffer.height = gFramebuffer.height;
	  gameFramebuffer.stride = gFramebuffer.stride;
	  gameFramebuffer.bytesPerPixel = gFramebuffer.bytesPerPixel;
	  gameFramebuffer.memory = gFramebuffer.memory;
	  gameUpdateAndRender(&gameFramebuffer);
	  
	  Window_dim windowDim = getWindowDim(window);
	  blitBufferToScreen(&gFramebuffer,deviceContext,windowDim.width,windowDim.height);
	}      
    }
  return 0;
}
