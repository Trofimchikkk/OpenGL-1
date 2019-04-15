#include <windows.h>													
#include <gl.h>														
#include <glu.h>														
#include <olectl.h>														
#include <math.h>														

#include "NeHeGL.h"														

#pragma comment( lib, "opengl32.lib" )									
#pragma comment( lib, "glu32.lib" )										

#ifndef CDS_FULLSCREEN													
#define CDS_FULLSCREEN 4												
#endif																	

GL_Window*	g_window;													
Keys*		g_keys;														


GLfloat	fogColor[4] = {0.6f, 0.3f, 0.0f, 1.0f};							
GLfloat camz;															


#define GL_FOG_COORDINATE_SOURCE_EXT			0x8450					
#define GL_FOG_COORDINATE_EXT					0x8451					

typedef void (APIENTRY * PFNGLFOGCOORDFEXTPROC) (GLfloat coord);		

PFNGLFOGCOORDFEXTPROC glFogCoordfEXT = NULL;							

GLuint	texture[1];														

int BuildTexture(char *szPathName, GLuint &texid)						
{
	HDC			hdcTemp;												
	HBITMAP		hbmpTemp;												
	IPicture	*pPicture;												
	OLECHAR		wszPath[MAX_PATH+1];									
	char		szPath[MAX_PATH+1];										
	long		lWidth;													
	long		lHeight;												
	long		lWidthPixels;											
	long		lHeightPixels;											
	GLint		glMaxTexDim ;											

	if (strstr(szPathName, "http://"))									
	{
		strcpy(szPath, szPathName);										
	}
	else																
	{
		GetCurrentDirectory(MAX_PATH, szPath);							
		strcat(szPath, "\\");											
		strcat(szPath, szPathName);										
	}

	MultiByteToWideChar(CP_ACP, 0, szPath, -1, wszPath, MAX_PATH);		
	HRESULT hr = OleLoadPicturePath(wszPath, 0, 0, 0, IID_IPicture, (void**)&pPicture);

	if(FAILED(hr))														
		return FALSE;													

	hdcTemp = CreateCompatibleDC(GetDC(0));								
	if(!hdcTemp)														
	{
		pPicture->Release();											
		return FALSE;													
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);					
	
	pPicture->get_Width(&lWidth);										
	lWidthPixels	= MulDiv(lWidth, GetDeviceCaps(hdcTemp, LOGPIXELSX), 2540);
	pPicture->get_Height(&lHeight);										
	lHeightPixels	= MulDiv(lHeight, GetDeviceCaps(hdcTemp, LOGPIXELSY), 2540);

	
	if (lWidthPixels <= glMaxTexDim) 
		lWidthPixels = 1 << (int)floor((log((double)lWidthPixels)/log(2.0f)) + 0.5f); 
	else  
		lWidthPixels = glMaxTexDim;
 
	if (lHeightPixels <= glMaxTexDim) 
		lHeightPixels = 1 << (int)floor((log((double)lHeightPixels)/log(2.0f)) + 0.5f);
	else  
		lHeightPixels = glMaxTexDim;
	
	
	BITMAPINFO	bi = {0};												
	DWORD		*pBits = 0;												

	bi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);				
	bi.bmiHeader.biBitCount		= 32;									
	bi.bmiHeader.biWidth		= lWidthPixels;							
	bi.bmiHeader.biHeight		= lHeightPixels;						
	bi.bmiHeader.biCompression	= BI_RGB;								
	bi.bmiHeader.biPlanes		= 1;									

	
	hbmpTemp = CreateDIBSection(hdcTemp, &bi, DIB_RGB_COLORS, (void**)&pBits, 0, 0);
	
	if(!hbmpTemp)														
	{
		DeleteDC(hdcTemp);												
		pPicture->Release();											
		return FALSE;													
	}

	SelectObject(hdcTemp, hbmpTemp);									

	
	pPicture->Render(hdcTemp, 0, 0, lWidthPixels, lHeightPixels, 0, lHeight, lWidth, -lHeight, 0);

	
	for(long i = 0; i < lWidthPixels * lHeightPixels; i++)				
	{
		BYTE* pPixel	= (BYTE*)(&pBits[i]);							
		BYTE  temp		= pPixel[0];									
		pPixel[0]		= pPixel[2];									
		pPixel[2]		= temp;											
		pPixel[3]		= 255;											
	}

	glGenTextures(1, &texid);											

	
	glBindTexture(GL_TEXTURE_2D, texid);								
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);		
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);     

	
	glTexImage2D(GL_TEXTURE_2D, 0, 3, lWidthPixels, lHeightPixels, 0, GL_RGBA, GL_UNSIGNED_BYTE, pBits);

	DeleteObject(hbmpTemp);												
	DeleteDC(hdcTemp);													

	pPicture->Release();												

	return TRUE;														
}

int Extension_Init()
{
	char Extension_Name[] = "EXT_fog_coord";

	
	char* glextstring=(char *)malloc(strlen((char *)glGetString(GL_EXTENSIONS))+1);
	strcpy (glextstring,(char *)glGetString(GL_EXTENSIONS));			

	if (!strstr(glextstring,Extension_Name))							
		return FALSE;													

	free(glextstring);													

	
	glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC) wglGetProcAddress("glFogCoordfEXT");

	return TRUE;
}

BOOL Initialize (GL_Window* window, Keys* keys)							
{
	g_window	= window;												
	g_keys		= keys;													

	
	if (!Extension_Init())												
		return FALSE;													

	if (!BuildTexture("data/wall.bmp", texture[0]))						
		return FALSE;													

	glEnable(GL_TEXTURE_2D);											
	glClearColor (0.0f, 0.0f, 0.0f, 0.5f);								
	glClearDepth (1.0f);												
	glDepthFunc (GL_LEQUAL);											
	glEnable (GL_DEPTH_TEST);											
	glShadeModel (GL_SMOOTH);											
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);					

	
	glEnable(GL_FOG);													
	glFogi(GL_FOG_MODE, GL_LINEAR);										
	glFogfv(GL_FOG_COLOR, fogColor);									
	glFogf(GL_FOG_START,  0.0f);										
	glFogf(GL_FOG_END,    1.0f);										
	glHint(GL_FOG_HINT, GL_NICEST);										
	glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);		

	camz =	-19.0f;														

	return TRUE;														
}

void Deinitialize (void)												
{
}

void Update (DWORD milliseconds)										
{
	if (g_keys->keyDown [VK_ESCAPE])									
		TerminateApplication (g_window);								

	if (g_keys->keyDown [VK_F1])										
		ToggleFullscreen (g_window);									

	if (g_keys->keyDown [VK_UP] && camz<14.0f)							
		camz+=(float)(milliseconds)/100.0f;								

	if (g_keys->keyDown [VK_DOWN] && camz>-19.0f)						
		camz-=(float)(milliseconds)/100.0f;								
}

void Draw (void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				
	glLoadIdentity ();													

	glTranslatef(0.0f, 0.0f, camz);										
	
	glBegin(GL_QUADS);													
	 	glFogCoordfEXT( 1.0f);	glTexCoord2f(0.0f, 0.0f);	glVertex3f(-2.5f,-2.5f,-15.0f);
		glFogCoordfEXT( 1.0f);	glTexCoord2f(1.0f, 0.0f);	glVertex3f( 2.5f,-2.5f,-15.0f);
		glFogCoordfEXT( 1.0f);	glTexCoord2f(1.0f, 1.0f);	glVertex3f( 2.5f, 2.5f,-15.0f);
		glFogCoordfEXT( 1.0f);	glTexCoord2f(0.0f, 1.0f);	glVertex3f(-2.5f, 2.5f,-15.0f);
	glEnd();

	glBegin(GL_QUADS);													
	 	glFogCoordfEXT( 1.0f);	glTexCoord2f(0.0f, 0.0f);	glVertex3f(-2.5f,-2.5f,-15.0f);
		glFogCoordfEXT( 1.0f);	glTexCoord2f(1.0f, 0.0f);	glVertex3f( 2.5f,-2.5f,-15.0f);
		glFogCoordfEXT( 0.0f);	glTexCoord2f(1.0f, 1.0f);	glVertex3f( 2.5f,-2.5f, 15.0f);
		glFogCoordfEXT( 0.0f);	glTexCoord2f(0.0f, 1.0f);	glVertex3f(-2.5f,-2.5f, 15.0f);
	glEnd();

	glBegin(GL_QUADS);													
		glFogCoordfEXT( 1.0f);	glTexCoord2f(0.0f, 0.0f);	glVertex3f(-2.5f, 2.5f,-15.0f);
		glFogCoordfEXT( 1.0f);	glTexCoord2f(1.0f, 0.0f);	glVertex3f( 2.5f, 2.5f,-15.0f);
		glFogCoordfEXT( 0.0f);	glTexCoord2f(1.0f, 1.0f);	glVertex3f( 2.5f, 2.5f, 15.0f);
		glFogCoordfEXT( 0.0f);	glTexCoord2f(0.0f, 1.0f);	glVertex3f(-2.5f, 2.5f, 15.0f);
	glEnd();

	glBegin(GL_QUADS);													
		glFogCoordfEXT( 0.0f);	glTexCoord2f(0.0f, 0.0f);	glVertex3f( 2.5f,-2.5f, 15.0f);
		glFogCoordfEXT( 0.0f);	glTexCoord2f(0.0f, 1.0f);	glVertex3f( 2.5f, 2.5f, 15.0f);
		glFogCoordfEXT( 1.0f);	glTexCoord2f(1.0f, 1.0f);	glVertex3f( 2.5f, 2.5f,-15.0f);
		glFogCoordfEXT( 1.0f);	glTexCoord2f(1.0f, 0.0f);	glVertex3f( 2.5f,-2.5f,-15.0f);
	glEnd();

	glBegin(GL_QUADS);													
	 	glFogCoordfEXT( 0.0f);	glTexCoord2f(0.0f, 0.0f);	glVertex3f(-2.5f,-2.5f, 15.0f);
		glFogCoordfEXT( 0.0f);	glTexCoord2f(0.0f, 1.0f);	glVertex3f(-2.5f, 2.5f, 15.0f);
		glFogCoordfEXT( 1.0f);	glTexCoord2f(1.0f, 1.0f);	glVertex3f(-2.5f, 2.5f,-15.0f);
		glFogCoordfEXT( 1.0f);	glTexCoord2f(1.0f, 0.0f);	glVertex3f(-2.5f,-2.5f,-15.0f);
	glEnd();
 
	glFlush ();															
}
