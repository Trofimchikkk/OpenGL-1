#include <windows.h>												
#include <gl.h>													
#include <glu.h>													
#include <stdio.h>													
#include <mmsystem.h>												
#include "glFont.h"													
#include "glCamera.h"												

HDC			hDC=NULL;												
HGLRC		hRC=NULL;												
HWND		hWnd=NULL;												
HINSTANCE	hInstance;												


bool		keys[256];												
bool		active=TRUE;											
bool		fullscreen=TRUE;										
bool		infoOn=FALSE;
int			gFrames=0;
DWORD		gStartTime;
DWORD		gCurrentTime;
GLfloat		gFPS;
glFont		gFont;
glCamera	gCamera;





GLUquadricObj *qobj;			
GLint						cylList;


LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);				
void DrawGLInfo(void);
void CheckKeys(void);

bool LoadTexture(LPTSTR szFileName, GLuint &texid)					
{
	HBITMAP hBMP;													
	BITMAP	BMP;													

	glGenTextures(1, &texid);										
	hBMP=(HBITMAP)LoadImage(GetModuleHandle(NULL), szFileName, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE );

	if (!hBMP)														
		return FALSE;												

	GetObject(hBMP, sizeof(BMP), &BMP);								
																	
																	
																	

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);							

	
	glBindTexture(GL_TEXTURE_2D, texid);								
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D, 0, 3, BMP.bmWidth, BMP.bmHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);

	DeleteObject(hBMP);												

	return TRUE;													
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)					
{
	gCamera.m_WindowHeight = height;								
	gCamera.m_WindowWidth = width;									

	if (height==0)													
	{
		height=1;													
	}

	glViewport(0,0,width,height);									

	glMatrixMode(GL_PROJECTION);									
	glLoadIdentity();												

	
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,1.0f,1000.0f);

	glMatrixMode(GL_MODELVIEW);										
	glLoadIdentity();												
}

int InitGL(GLvoid)													
{
	GLuint tex=0;

	glShadeModel(GL_SMOOTH);										
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);							
	glClearDepth(1.0f);												
	glEnable(GL_DEPTH_TEST);										
	glDepthFunc(GL_LEQUAL);											
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);				

	LoadTexture("Art/Font.bmp", tex);								
	if(tex != 0)													
	{
		gFont.SetFontTexture(tex);									
		gFont.SetWindowSize(1024, 768);								
		gFont.BuildFont(1.0f);					                    
	}
	else
	{
		MessageBox(NULL,											
					"Failed to load font texture.",					
					"Error", 
					MB_OK);
	}

	gCamera.m_MaxHeadingRate = 1.0f;								
	gCamera.m_MaxPitchRate = 1.0f;									
	gCamera.m_HeadingDegrees = 0.0f;								

	
	LoadTexture("Art/HardGlow2.bmp", gCamera.m_GlowTexture);
	if(gCamera.m_GlowTexture == 0) {
		MessageBox(NULL, "Failed to load Hard Glow texture.", "Error", MB_OK);
		return(FALSE);
	}
	
	
	LoadTexture("Art/BigGlow3.bmp", gCamera.m_BigGlowTexture);
	if(gCamera.m_BigGlowTexture == 0) {
		MessageBox(NULL, "Failed to load Big Glow texture.", "Error", MB_OK);
		return(FALSE);
	}
	
	
	LoadTexture("Art/Halo3.bmp", gCamera.m_HaloTexture);
	if(gCamera.m_HaloTexture == 0) {
		MessageBox(NULL, "Failed to load Halo texture.", "Error", MB_OK);
		return(FALSE);
	}
	
	
	LoadTexture("Art/Streaks4.bmp", gCamera.m_StreakTexture);
	if(gCamera.m_StreakTexture == 0) {
		MessageBox(NULL, "Failed to load Streaks texture.", "Error", MB_OK);
		return(FALSE);
	}



	
	cylList = glGenLists(1);
	qobj = gluNewQuadric();
	gluQuadricDrawStyle(qobj, GLU_FILL); 
	gluQuadricNormals(qobj, GLU_SMOOTH);
	glNewList(cylList, GL_COMPILE);
		glEnable(GL_COLOR_MATERIAL);
		glColor3f(0.0f, 0.0f, 1.0f);
		glEnable(GL_LIGHT0);
		glEnable(GL_LIGHTING);
		glTranslatef(0.0f,0.0f,-2.0f);
		gluCylinder(qobj, 0.5, 0.5, 4.0, 15, 5);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
		glDisable(GL_COLOR_MATERIAL);
	glEndList();

	gStartTime = timeGetTime();										

	return TRUE;													
}

int DrawGLScene(GLvoid)												
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				
	glLoadIdentity();												

	
	
	
	
	
	gCamera.m_LightSourcePos.z = gCamera.m_Position.z - 50.0f;

	
	
	
	
	glPushMatrix();
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, -20.0f);
		glRotatef(timeGetTime() / 50.0f, 0.3f, 0.0f, 0.0f);
		glRotatef(timeGetTime() / 50.0f, 0.0f, 0.5f, 0.0f);
		glCallList(cylList);
	glPopMatrix();

	gCamera.SetPrespective();										
	gCamera.RenderLensFlare();										
	gCamera.UpdateFrustumFaster();									
	
	if(infoOn == TRUE) {											
		DrawGLInfo();												
	}

	CheckKeys();													

	return TRUE;
}

GLvoid KillGLWindow(GLvoid)											
{
	if (fullscreen)													
	{
		ChangeDisplaySettings(NULL,0);								
		ShowCursor(TRUE);											
	}

	if (hRC)														
	{
		if (!wglMakeCurrent(NULL,NULL))								
		{
			MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))									
		{
			MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;													
	}

	if (hDC && !ReleaseDC(hWnd,hDC))								
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hDC=NULL;													
	}

	if (hWnd && !DestroyWindow(hWnd))								
	{
		MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hWnd=NULL;													
	}

	if (!UnregisterClass("OpenGL",hInstance))						
	{
		MessageBox(NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		hInstance=NULL;												
	}
}

 

 
BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;									
	WNDCLASS	wc;												
	DWORD		dwExStyle;										
	DWORD		dwStyle;										
	RECT		WindowRect;										
	WindowRect.left=(long)0;									
	WindowRect.right=(long)width;								
	WindowRect.top=(long)0;										
	WindowRect.bottom=(long)height;								

	fullscreen=fullscreenflag;									

	hInstance			= GetModuleHandle(NULL);				
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	
	wc.lpfnWndProc		= (WNDPROC) WndProc;					
	wc.cbClsExtra		= 0;									
	wc.cbWndExtra		= 0;									
	wc.hInstance		= hInstance;							
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			
	wc.hbrBackground	= NULL;									
	wc.lpszMenuName		= NULL;									
	wc.lpszClassName	= "OpenGL";								

	if (!RegisterClass(&wc))									
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											
	}
	
	if (fullscreen)												
	{
		DEVMODE dmScreenSettings;								
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		
		dmScreenSettings.dmPelsWidth	= width;				
		dmScreenSettings.dmPelsHeight	= height;				
		dmScreenSettings.dmBitsPerPel	= bits;					
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			
			if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				fullscreen=FALSE;								
			}
			else
			{
				
				MessageBox(NULL,"Program Will Now Close.","ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									
			}
		}
	}

	if (fullscreen)												
	{
		dwExStyle=WS_EX_APPWINDOW;								
		dwStyle=WS_POPUP;										
		ShowCursor(FALSE);										
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			
		dwStyle=WS_OVERLAPPEDWINDOW;							
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);	

	
	if (!(hWnd=CreateWindowEx(	dwExStyle,							
								"OpenGL",							
								title,								
								dwStyle |							
								WS_CLIPSIBLINGS |					
								WS_CLIPCHILDREN,					
								0, 0,								
								WindowRect.right-WindowRect.left,	
								WindowRect.bottom-WindowRect.top,	
								NULL,								
								NULL,								
								hInstance,							
								NULL)))								
	{
		KillGLWindow();											
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											
	}

	static	PIXELFORMATDESCRIPTOR pfd=							
	{
		sizeof(PIXELFORMATDESCRIPTOR),							
		1,														
		PFD_DRAW_TO_WINDOW |									
		PFD_SUPPORT_OPENGL |									
		PFD_DOUBLEBUFFER,										
		PFD_TYPE_RGBA,											
		bits,													
		0, 0, 0, 0, 0, 0,										
		0,														
		0,														
		0,														
		0, 0, 0, 0,												
		16,														
		0,														
		0,														
		PFD_MAIN_PLANE,											
		0,														
		0, 0, 0													
	};
	
	if (!(hDC=GetDC(hWnd)))										
	{
		KillGLWindow();											
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))				
	{
		KillGLWindow();											
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))					
	{
		KillGLWindow();											
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											
	}

	if (!(hRC=wglCreateContext(hDC)))							
	{
		KillGLWindow();											
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											
	}

	if(!wglMakeCurrent(hDC,hRC))								
	{
		KillGLWindow();											
		MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											
	}

	ShowWindow(hWnd,SW_SHOW);									
	SetForegroundWindow(hWnd);									
	SetFocus(hWnd);												
	ReSizeGLScene(width, height);								

	if (!InitGL())												
	{
		KillGLWindow();											
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											
	}

	return TRUE;												
}

LRESULT CALLBACK WndProc(	HWND	hWnd,						
							UINT	uMsg,						
							WPARAM	wParam,						
							LPARAM	lParam)						
{
	switch (uMsg)												
	{
		case WM_ACTIVATE:										
		{
			if (!HIWORD(wParam))								
			{
				active=TRUE;									
			}
			else
			{
				active=FALSE;									
			}

			return 0;											
		}

		case WM_SYSCOMMAND:										
		{
			switch (wParam)										
			{
				case SC_SCREENSAVE:								
				case SC_MONITORPOWER:							
				return 0;										
			}
			break;												
		}

		case WM_CLOSE:											
		{
			PostQuitMessage(0);									
			return 0;											
		}

		case WM_KEYDOWN:										
		{
			keys[wParam] = TRUE;								
			return 0;											
		}

		case WM_KEYUP:											
		{
			keys[wParam] = FALSE;								
			return 0;											
		}

		case WM_SIZE:											
		{
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));		
			return 0;											
		}
	}

	
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance,						
					HINSTANCE	hPrevInstance,					
					LPSTR		lpCmdLine,						
					int			nCmdShow)						
{
	MSG		msg;												
	BOOL	done=FALSE;											

	
	if (MessageBox(NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNO|MB_ICONQUESTION)==IDNO)
	{
		fullscreen=FALSE;										
	}

	
	if (!CreateGLWindow("Lens Flare Tutorial",640,480,32,fullscreen))
	{
		return 0;												
	}

	while(!done)												
	{
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))				
		{
			if (msg.message==WM_QUIT)							
			{
				done=TRUE;										
			}
			else												
			{
				TranslateMessage(&msg);							
				DispatchMessage(&msg);							
			}
		}
		else													
		{
			
			if (active)											
			{
				if (keys[VK_ESCAPE])							
				{
					done=TRUE;									
				}
				else											
				{
					DrawGLScene();								
					SwapBuffers(hDC);							
				}
			}

			if (keys[VK_F1])									
			{
				keys[VK_F1]=FALSE;								
				KillGLWindow();									
				fullscreen=!fullscreen;							
				
				if (!CreateGLWindow("NeHe's OpenGL Framework",640,480,16,fullscreen))
				{
					return 0;									
				}
			}
		}
	}


	
	

	gluDeleteQuadric(qobj);							
	glDeleteLists(cylList,1);						


	
	KillGLWindow();												
	return (msg.wParam);										
}

void DrawGLInfo(void)
{
	GLfloat modelMatrix[16];									
	GLfloat projMatrix[16];										
	GLfloat DiffTime;											
	char String[64];											
																

	glGetFloatv(GL_PROJECTION_MATRIX, projMatrix);				
	glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);				

	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	sprintf(String, "m_Position............. = %.02f, %.02f, %.02f", gCamera.m_Position.x, gCamera.m_Position.y, gCamera.m_Position.z);
	gFont.glPrintf(10, 720, 1, String);
	
	
	sprintf(String, "m_DirectionVector...... = %.02f, %.02f, %.02f", gCamera.m_DirectionVector.i, gCamera.m_DirectionVector.j, gCamera.m_DirectionVector.k);
	gFont.glPrintf(10, 700, 1, String);
	
	
	sprintf(String, "m_LightSourcePos....... = %.02f, %.02f, %.02f", gCamera.m_LightSourcePos.x, gCamera.m_LightSourcePos.y, gCamera.m_LightSourcePos.z);
	gFont.glPrintf(10, 680, 1, String);

	
	sprintf(String, "ptIntersect............ = %.02f, %.02f, %.02f", gCamera.ptIntersect.x, gCamera.ptIntersect.y, gCamera.ptIntersect.x);
	gFont.glPrintf(10, 660, 1, String);

	
	sprintf(String, "vLightSourceToCamera... = %.02f, %.02f, %.02f", gCamera.vLightSourceToCamera.i, gCamera.vLightSourceToCamera.j, gCamera.vLightSourceToCamera.k);
	gFont.glPrintf(10, 640, 1, String);

	
	sprintf(String, "vLightSourceToIntersect = %.02f, %.02f, %.02f", gCamera.vLightSourceToIntersect.i, gCamera.vLightSourceToIntersect.j, gCamera.vLightSourceToIntersect.k);
	gFont.glPrintf(10, 620, 1, String);

	
	sprintf(String, "GL_MODELVIEW_MATRIX");
	gFont.glPrintf(10, 580, 1, String);
	
	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", modelMatrix[0], modelMatrix[1], modelMatrix[2], modelMatrix[3]);
	gFont.glPrintf(10, 560, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", modelMatrix[4], modelMatrix[5], modelMatrix[6], modelMatrix[7]);
	gFont.glPrintf(10, 540, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", modelMatrix[8], modelMatrix[9], modelMatrix[10], modelMatrix[11]);
	gFont.glPrintf(10, 520, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", modelMatrix[12], modelMatrix[13], modelMatrix[14], modelMatrix[15]);
	gFont.glPrintf(10, 500, 1, String);

	
	sprintf(String, "GL_PROJECTION_MATRIX");
	gFont.glPrintf(10, 460, 1, String);
	
	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", projMatrix[0], projMatrix[1], projMatrix[2], projMatrix[3]);
	gFont.glPrintf(10, 440, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", projMatrix[4], projMatrix[5], projMatrix[6], projMatrix[7]);
	gFont.glPrintf(10, 420, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.03f, %.03f", projMatrix[8], projMatrix[9], projMatrix[10], projMatrix[11]);
	gFont.glPrintf(10, 400, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.03f, %.03f", projMatrix[12], projMatrix[13], projMatrix[14], projMatrix[15]);
	gFont.glPrintf(10, 380, 1, String);

	
	gFont.glPrintf(10, 320, 1, "FRUSTUM CLIPPING PLANES");

	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", gCamera.m_Frustum[0][0], gCamera.m_Frustum[0][1], gCamera.m_Frustum[0][2], gCamera.m_Frustum[0][3]);
	gFont.glPrintf(10, 300, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", gCamera.m_Frustum[1][0], gCamera.m_Frustum[1][1], gCamera.m_Frustum[1][2], gCamera.m_Frustum[1][3]);
	gFont.glPrintf(10, 280, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", gCamera.m_Frustum[2][0], gCamera.m_Frustum[2][1], gCamera.m_Frustum[2][2], gCamera.m_Frustum[2][3]);
	gFont.glPrintf(10, 260, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", gCamera.m_Frustum[3][0], gCamera.m_Frustum[3][1], gCamera.m_Frustum[3][2], gCamera.m_Frustum[3][3]);
	gFont.glPrintf(10, 240, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", gCamera.m_Frustum[4][0], gCamera.m_Frustum[4][1], gCamera.m_Frustum[4][2], gCamera.m_Frustum[4][3]);
	gFont.glPrintf(10, 220, 1, String);

	
	sprintf(String, "%.02f, %.02f, %.02f, %.02f", gCamera.m_Frustum[5][0], gCamera.m_Frustum[5][1], gCamera.m_Frustum[5][2], gCamera.m_Frustum[5][3]);
	gFont.glPrintf(10, 200, 1, String);

	if(gFrames >= 100)											
	{
		gCurrentTime = timeGetTime();							
		DiffTime = GLfloat(gCurrentTime - gStartTime);			
		gFPS = (gFrames / DiffTime) * 1000.0f;					
		gStartTime = gCurrentTime;								
		gFrames = 1;											
	}
	else
	{
		gFrames++;												
	}
	
	
	sprintf(String, "FPS %.02f", gFPS);
	gFont.glPrintf(10, 160, 1, String);
}

void CheckKeys(void)
{
	if(keys['W'] == TRUE)										
	{
		gCamera.ChangePitch(-0.2f);								
	}

	if(keys['S'] == TRUE)										
	{
		gCamera.ChangePitch(0.2f);								
	}
	
	if(keys['D'] == TRUE)										
	{
		gCamera.ChangeHeading(0.2f);							
	}
	
	if(keys['A'] == TRUE)										
	{
		gCamera.ChangeHeading(-0.2f);							
	}
	
	if(keys['Z'] == TRUE)										
	{
		gCamera.m_ForwardVelocity = 0.01f;						
	}
	
	if(keys['C'] == TRUE)										
	{
		gCamera.m_ForwardVelocity = -0.01f;						
	}
	
	if(keys['X'] == TRUE)										
	{
		gCamera.m_ForwardVelocity = 0.0f;						
	}

	if(keys['1'] == TRUE)										
	{
		infoOn = TRUE;											
	}
	
	if(keys['2'] == TRUE)										
	{
		infoOn = FALSE;											
      }
}
