#include <windows.h>											
#include <gl.h>												
#include <glu.h>												
#include <stdio.h>												

HDC			hDC=NULL;											
HGLRC		hRC=NULL;											
HWND		hWnd=NULL;											
HINSTANCE	hInstance = NULL;									

bool		keys[256];											
bool		active=TRUE;										
bool		fullscreen=TRUE;									

DEVMODE		DMsaved;											

GLfloat		xrot;												
GLfloat		yrot;												
GLfloat		zrot;												

GLuint		texture[1];											

typedef struct
{
	int width;													
	int height;													
	int format;													
	unsigned char *data;										
} TEXTURE_IMAGE;

typedef TEXTURE_IMAGE *P_TEXTURE_IMAGE;							

P_TEXTURE_IMAGE t1;												
P_TEXTURE_IMAGE t2;												

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);			


P_TEXTURE_IMAGE AllocateTextureBuffer( GLint w, GLint h, GLint f)
{
    P_TEXTURE_IMAGE ti=NULL;									
    unsigned char *c=NULL;										

    ti = (P_TEXTURE_IMAGE)malloc(sizeof(TEXTURE_IMAGE));		
    
    if( ti != NULL ) {
        ti->width  = w;											
        ti->height = h;											
        ti->format = f;											
        c = (unsigned char *)malloc( w * h * f);
        if ( c != NULL ) {
            ti->data = c;
        }
        else {
			MessageBox(NULL,"Could Not Allocate Memory For A Texture Buffer","BUFFER ERROR",MB_OK | MB_ICONINFORMATION);
            return NULL;
        }
    }
    else
	{
		MessageBox(NULL,"Could Not Allocate An Image Structure","IMAGE STRUCTURE ERROR",MB_OK | MB_ICONINFORMATION);
		return NULL;
    }
	return ti;													
}


void DeallocateTexture( P_TEXTURE_IMAGE t )
{
	if(t)
	{
		if(t->data)
		{
			free(t->data);
		}

		free(t);
	}
}



int ReadTextureData ( char *filename, P_TEXTURE_IMAGE buffer)
{
	FILE *f;
	int i,j,k,done=0;
	int stride = buffer->width * buffer->format;				
	unsigned char *p = NULL;

    f = fopen(filename, "rb");									
    if( f != NULL )												
    {
		for( i = buffer->height-1; i >= 0 ; i-- )				
		{
			p = buffer->data + (i * stride );					
			for ( j = 0; j < buffer->width ; j++ )				
			{
				for ( k = 0 ; k < buffer->format-1 ; k++, p++, done++ )
				{
					*p = fgetc(f);								
				}
				*p = 255; p++;									
			}
		}
		fclose(f);												
	}
	else														
	{
		MessageBox(NULL,"Unable To Open Image File","IMAGE ERROR",MB_OK | MB_ICONINFORMATION);
    }
	return done;												
}

void BuildTexture (P_TEXTURE_IMAGE tex)
{
	glGenTextures(1, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tex->width, tex->height, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
}

void Blit( P_TEXTURE_IMAGE src, P_TEXTURE_IMAGE dst, int src_xstart, int src_ystart, int src_width, int src_height,
           int dst_xstart, int dst_ystart, int blend, int alpha)
{
	int i,j,k;
	unsigned char *s, *d;										

	
    if( alpha > 255 ) alpha = 255;
    if( alpha < 0 ) alpha = 0;

	
    if( blend < 0 ) blend = 0;
    if( blend > 1 ) blend = 1;

    d = dst->data + (dst_ystart * dst->width * dst->format);    
    s = src->data + (src_ystart * src->width * src->format);    

    for (i = 0 ; i < src_height ; i++ )							
    {
        s = s + (src_xstart * src->format);						
        d = d + (dst_xstart * dst->format);						
        for (j = 0 ; j < src_width ; j++ )						
        {
            for( k = 0 ; k < src->format ; k++, d++, s++)		
            {
                if (blend)										
                    *d = ( (*s * alpha) + (*d * (255-alpha)) ) >> 8; 
                else											
                    *d = *s;									
            }
        }
        d = d + (dst->width - (src_width + dst_xstart))*dst->format;	
        s = s + (src->width - (src_width + src_xstart))*src->format;	
    }
}

int InitGL(GLvoid)												
{
    t1 = AllocateTextureBuffer( 256, 256, 4 );					
    if (ReadTextureData("Data/Monitor.raw",t1)==0)				
	{															
		MessageBox(NULL,"Could Not Read 'Monitor.raw' Image Data","TEXTURE ERROR",MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

    t2 = AllocateTextureBuffer( 256, 256, 4 );					
	if (ReadTextureData("Data/GL.raw",t2)==0)					
	{															
		MessageBox(NULL,"Could Not Read 'GL.raw' Image Data","TEXTURE ERROR",MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }

	
    Blit(t2,t1,127,127,128,128,64,64,1,127);					

    BuildTexture (t1);											

    DeallocateTexture( t1 );									
    DeallocateTexture( t2 );									

	glEnable(GL_TEXTURE_2D);									

	glShadeModel(GL_SMOOTH);									
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);						
	glClearDepth(1.0);											
	glEnable(GL_DEPTH_TEST);									
	glDepthFunc(GL_LESS);										

	return TRUE;
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

	
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							
	glLoadIdentity();									
}

GLvoid DrawGLScene(GLvoid)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	glLoadIdentity();										
	glTranslatef(0.0f,0.0f,-5.0f);

	glRotatef(xrot,1.0f,0.0f,0.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);
	glRotatef(zrot,0.0f,0.0f,1.0f);

	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glBegin(GL_QUADS);
		
		glNormal3f( 0.0f, 0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		
		glNormal3f( 0.0f, 0.0f,-1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		
		glNormal3f( 0.0f, 1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		
		glNormal3f( 0.0f,-1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		
		glNormal3f( 1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
		
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
	glEnd();

	xrot+=0.3f;
	yrot+=0.2f;
	zrot+=0.4f;
}

GLvoid KillGLWindow(GLvoid)								
{
	if (fullscreen)										
	{
		if (!ChangeDisplaySettings(NULL,CDS_TEST)) {	
			ChangeDisplaySettings(NULL,CDS_RESET);		
			ChangeDisplaySettings(&DMsaved,CDS_RESET);	
		}
		else											
		{
			ChangeDisplaySettings(NULL,CDS_RESET);		
		}
			
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

	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DMsaved); 

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

	if (!RegisterClass(&wc))							
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;									
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

	
	if (!CreateGLWindow("Andreas Löffler, Rob Fletcher & NeHe's Blitter & Raw Image Loading Tutorial", 640, 480, 32, fullscreen))
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

		if (!active)									
		{
			WaitMessage();								
		}

		if (keys[VK_ESCAPE])							
		{
			done=TRUE;									
		}

		if (keys[VK_F1])								
		{
			keys[VK_F1]=FALSE;							
			KillGLWindow();								
			fullscreen=!fullscreen;						
			
			if (!CreateGLWindow("Andreas Löffler, Rob Fletcher & NeHe's Blitter & Raw Image Loading Tutorial",640,480,16,fullscreen))
			{
				return 0;								
			}
		}

		DrawGLScene();									
		SwapBuffers(hDC);								
	}

	
	KillGLWindow();										
	return (msg.wParam);								
}
