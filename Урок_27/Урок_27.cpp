#include <windows.h>									
#include <gl.h>										
#include <glu.h>										
#include <glaux.h>									
#include "3dobject.h"									

typedef float GLvector4f[4];							
typedef float GLmatrix16f[16];							

HDC			hDC=NULL;									
HGLRC		hRC=NULL;									
HWND		hWnd=NULL;									
HINSTANCE	hInstance=NULL;								

bool keys[256], keypressed[256];						
bool active=TRUE;										
bool fullscreen=TRUE;									

glObject	obj;										
GLfloat		xrot=0, xspeed=0;							
GLfloat		yrot=0, yspeed=0;							

float LightPos[] = { 0.0f, 5.0f,-4.0f, 1.0f};			
float LightAmb[] = { 0.2f, 0.2f, 0.2f, 1.0f};			
float LightDif[] = { 0.6f, 0.6f, 0.6f, 1.0f};			
float LightSpc[] = {-0.2f, -0.2f, -0.2f, 1.0f};			

float MatAmb[] = {0.4f, 0.4f, 0.4f, 1.0f};				
float MatDif[] = {0.2f, 0.6f, 0.9f, 1.0f};				
float MatSpc[] = {0.0f, 0.0f, 0.0f, 1.0f};				
float MatShn[] = {0.0f};								

float ObjPos[] = {-2.0f,-2.0f,-5.0f};					

GLUquadricObj	*q;										
float SpherePos[] = {-4.0f,-5.0f,-6.0f};

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	

void VMatMult(GLmatrix16f M, GLvector4f v)
{
	GLfloat res[4];										
	res[0]=M[ 0]*v[0]+M[ 4]*v[1]+M[ 8]*v[2]+M[12]*v[3];
	res[1]=M[ 1]*v[0]+M[ 5]*v[1]+M[ 9]*v[2]+M[13]*v[3];
	res[2]=M[ 2]*v[0]+M[ 6]*v[1]+M[10]*v[2]+M[14]*v[3];
	res[3]=M[ 3]*v[0]+M[ 7]*v[1]+M[11]*v[2]+M[15]*v[3];
	v[0]=res[0];										
	v[1]=res[1];
	v[2]=res[2];
	v[3]=res[3];										
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		
{
	if (height==0)										
	{
		height=1;										
	}

	glViewport(0,0,width,height);						

	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();									

	
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.001f,100.0f);

	glMatrixMode(GL_MODELVIEW);							
	glLoadIdentity();									
}

int InitGLObjects()										
{
	if (!ReadObject("Data/Object2.txt", &obj))			
	{
		return FALSE;									
	}

	SetConnectivity(&obj);								

	for (unsigned int i=0;i<obj.nPlanes;i++)			
		CalcPlane(obj, &(obj.planes[i]));				

	return TRUE;										
}

int InitGL(GLvoid)										
{
	if (!InitGLObjects()) return FALSE;					
	glShadeModel(GL_SMOOTH);							
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				
	glClearDepth(1.0f);									
	glClearStencil(0);									
	glEnable(GL_DEPTH_TEST);							
	glDepthFunc(GL_LEQUAL);								
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	

	glLightfv(GL_LIGHT1, GL_POSITION, LightPos);		
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmb);			
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDif);			
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpc);		
	glEnable(GL_LIGHT1);								
	glEnable(GL_LIGHTING);								

	glMaterialfv(GL_FRONT, GL_AMBIENT, MatAmb);			
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MatDif);			
	glMaterialfv(GL_FRONT, GL_SPECULAR, MatSpc);		
	glMaterialfv(GL_FRONT, GL_SHININESS, MatShn);		

	glCullFace(GL_BACK);								
	glEnable(GL_CULL_FACE);								
	glClearColor(0.1f, 1.0f, 0.5f, 1.0f);				

	q = gluNewQuadric();								
	gluQuadricNormals(q, GL_SMOOTH);					
	gluQuadricTexture(q, GL_FALSE);						

	return TRUE;										
}

void DrawGLRoom()										
{
	glBegin(GL_QUADS);									
		
		glNormal3f(0.0f, 1.0f, 0.0f);					
		glVertex3f(-10.0f,-10.0f,-20.0f);				
		glVertex3f(-10.0f,-10.0f, 20.0f);				
		glVertex3f( 10.0f,-10.0f, 20.0f);				
		glVertex3f( 10.0f,-10.0f,-20.0f);				
		
		glNormal3f(0.0f,-1.0f, 0.0f);					
		glVertex3f(-10.0f, 10.0f, 20.0f);				
		glVertex3f(-10.0f, 10.0f,-20.0f);				
		glVertex3f( 10.0f, 10.0f,-20.0f);				
		glVertex3f( 10.0f, 10.0f, 20.0f);				
		
		glNormal3f(0.0f, 0.0f, 1.0f);					
		glVertex3f(-10.0f, 10.0f,-20.0f);				
		glVertex3f(-10.0f,-10.0f,-20.0f);				
		glVertex3f( 10.0f,-10.0f,-20.0f);				
		glVertex3f( 10.0f, 10.0f,-20.0f);				
		
		glNormal3f(0.0f, 0.0f,-1.0f);					
		glVertex3f( 10.0f, 10.0f, 20.0f);				
		glVertex3f( 10.0f,-10.0f, 20.0f);				
		glVertex3f(-10.0f,-10.0f, 20.0f);				
		glVertex3f(-10.0f, 10.0f, 20.0f);				
		
		glNormal3f(1.0f, 0.0f, 0.0f);					
		glVertex3f(-10.0f, 10.0f, 20.0f);				
		glVertex3f(-10.0f,-10.0f, 20.0f);				
		glVertex3f(-10.0f,-10.0f,-20.0f);				
		glVertex3f(-10.0f, 10.0f,-20.0f);				
		
		glNormal3f(-1.0f, 0.0f, 0.0f);					
		glVertex3f( 10.0f, 10.0f,-20.0f);				
		glVertex3f( 10.0f,-10.0f,-20.0f);				
		glVertex3f( 10.0f,-10.0f, 20.0f);				
		glVertex3f( 10.0f, 10.0f, 20.0f);				
	glEnd();											
}

int DrawGLScene(GLvoid)									
{
	GLmatrix16f Minv;
	GLvector4f wlp, lp;

	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	glLoadIdentity();									
	glTranslatef(0.0f, 0.0f, -20.0f);					
	glLightfv(GL_LIGHT1, GL_POSITION, LightPos);		
	glTranslatef(SpherePos[0], SpherePos[1], SpherePos[2]);	
	gluSphere(q, 1.5f, 32, 16);							

	
	
	

	
	
	glLoadIdentity();									
	glRotatef(-yrot, 0.0f, 1.0f, 0.0f);					
	glRotatef(-xrot, 1.0f, 0.0f, 0.0f);					
	glGetFloatv(GL_MODELVIEW_MATRIX,Minv);				
	lp[0] = LightPos[0];								
	lp[1] = LightPos[1];								
	lp[2] = LightPos[2];								
	lp[3] = LightPos[3];								
	VMatMult(Minv, lp);									
	glTranslatef(-ObjPos[0], -ObjPos[1], -ObjPos[2]);	
	glGetFloatv(GL_MODELVIEW_MATRIX,Minv);				
	wlp[0] = 0.0f;										
	wlp[1] = 0.0f;										
	wlp[2] = 0.0f;										
	wlp[3] = 1.0f;
	VMatMult(Minv, wlp);								
														
	lp[0] += wlp[0];									
	lp[1] += wlp[1];									
	lp[2] += wlp[2];									

	glColor4f(0.7f, 0.4f, 0.0f, 1.0f);					
	glLoadIdentity();									
	glTranslatef(0.0f, 0.0f, -20.0f);					
	DrawGLRoom();										
	glTranslatef(ObjPos[0], ObjPos[1], ObjPos[2]);		
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);					
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);					
	DrawGLObject(obj);									
	CastShadow(&obj, lp);								

	glColor4f(0.7f, 0.4f, 0.0f, 1.0f);					
	glDisable(GL_LIGHTING);								
	glDepthMask(GL_FALSE);								
	glTranslatef(lp[0], lp[1], lp[2]);					
														
	gluSphere(q, 0.2f, 16, 8);							
	glEnable(GL_LIGHTING);								
	glDepthMask(GL_TRUE);								

	xrot += xspeed;										
	yrot += yspeed;										

	glFlush();											
	return TRUE;										
}

void ProcessKeyboard()									
{
	
	if (keys[VK_LEFT])	yspeed -= 0.1f;					
	if (keys[VK_RIGHT])	yspeed += 0.1f;					
	if (keys[VK_UP])	xspeed -= 0.1f;					
	if (keys[VK_DOWN])	xspeed += 0.1f;					

	
	if (keys['L']) LightPos[0] += 0.05f;				
	if (keys['J']) LightPos[0] -= 0.05f;				

	if (keys['I']) LightPos[1] += 0.05f;				
	if (keys['K']) LightPos[1] -= 0.05f;				

	if (keys['O']) LightPos[2] += 0.05f;				
	if (keys['U']) LightPos[2] -= 0.05f;				

	
	if (keys[VK_NUMPAD6]) ObjPos[0] += 0.05f;			
	if (keys[VK_NUMPAD4]) ObjPos[0] -= 0.05f;			

	if (keys[VK_NUMPAD8]) ObjPos[1] += 0.05f;			
	if (keys[VK_NUMPAD5]) ObjPos[1] -= 0.05f;			

	if (keys[VK_NUMPAD9]) ObjPos[2] += 0.05f;			
	if (keys[VK_NUMPAD7]) ObjPos[2] -= 0.05f;			

	
	if (keys['D']) SpherePos[0] += 0.05f;				
	if (keys['A']) SpherePos[0] -= 0.05f;				

	if (keys['W']) SpherePos[1] += 0.05f;				
	if (keys['S']) SpherePos[1] -= 0.05f;				

	if (keys['E']) SpherePos[2] += 0.05f;				
	if (keys['Q']) SpherePos[2] -= 0.05f;				
}


GLvoid KillGLWindow(GLvoid)								
{
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

	if (fullscreen)										
	{
		ChangeDisplaySettings(NULL,0);					
		ShowCursor(TRUE);								
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
		dwStyle=WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;	
		ShowCursor(FALSE);								
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;	
		dwStyle=WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;	
	}

	
	if (!(hWnd=CreateWindowEx(	dwExStyle,				
								"OpenGL",				
								title,					
								dwStyle,				
								0, 0,					
								width, height,			
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
		1,												
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

	
	if (!CreateGLWindow("Banu Octavian & NeHe's Shadow Casting Tutorial",800,600,32,fullscreen))
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
			
			if (active && keys[VK_ESCAPE])				
			{
				done=TRUE;								
			}
			else										
			{
				DrawGLScene();							
				SwapBuffers(hDC);						
				ProcessKeyboard();						
			}
		}
	}

	
	KillGLWindow();										
	return (msg.wParam);								
}
