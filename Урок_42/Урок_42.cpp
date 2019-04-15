#include <windows.h>													
#include <gl.h>														
#include <glu.h>														

#include "NeHeGL.h"														

#pragma comment( lib, "opengl32.lib" )									
#pragma comment( lib, "glu32.lib" )										

GL_Window*	g_window;													
Keys*		g_keys;														


int	mx,my;																

const	width	= 128;													
const	height	= 128;													

BOOL	done;															
BOOL	sp;																

BYTE	r[4], g[4], b[4];												
BYTE	*tex_data;														

GLfloat	xrot, yrot, zrot;												

GLUquadricObj *quadric;													

void UpdateTex(int dmx, int dmy)										
{
	tex_data[0+((dmx+(width*dmy))*3)]=255;								
	tex_data[1+((dmx+(width*dmy))*3)]=255;								
	tex_data[2+((dmx+(width*dmy))*3)]=255;								
}

void Reset (void)														
{
	ZeroMemory(tex_data, width * height *3);							

	srand(GetTickCount());												

	for (int loop=0; loop<4; loop++)									
	{
		r[loop]=rand()%128+128;											
		g[loop]=rand()%128+128;											
		b[loop]=rand()%128+128;											
	}

	mx=int(rand()%(width/2))*2;											
	my=int(rand()%(height/2))*2;										
}

BOOL Initialize (GL_Window* window, Keys* keys)							
{
	tex_data=new BYTE[width*height*3];									

	g_window	= window;												
	g_keys		= keys;													

	Reset();															

	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);

	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);								
	glClearDepth (1.0f);												

	glDepthFunc (GL_LEQUAL);											
	glEnable (GL_DEPTH_TEST);											

	glEnable(GL_COLOR_MATERIAL);										

	glEnable(GL_TEXTURE_2D);											

	quadric=gluNewQuadric();											
	gluQuadricNormals(quadric, GLU_SMOOTH);								
	gluQuadricTexture(quadric, GL_TRUE);								

	glEnable(GL_LIGHT0);												

	return TRUE;														
}

void Deinitialize (void)												
{
	delete [] tex_data;													
}

void Update (float milliseconds)										
{
	int		dir;														

	if (g_keys->keyDown [VK_ESCAPE])									
		TerminateApplication (g_window);								

	if (g_keys->keyDown [VK_F1])										
		ToggleFullscreen (g_window);									

	if (g_keys->keyDown [' '] && !sp)									
	{
		sp=TRUE;														
		Reset();														
	}

	if (!g_keys->keyDown [' '])											
		sp=FALSE;														

	xrot+=(float)(milliseconds)*0.02f;									
	yrot+=(float)(milliseconds)*0.03f;									
	zrot+=(float)(milliseconds)*0.015f;									

	done=TRUE;															
	for (int x=0; x<width; x+=2)										
	{
		for (int y=0; y<height; y+=2)									
		{
			if (tex_data[((x+(width*y))*3)]==0)							
				done=FALSE;												
		}
	}

	if (done)															
	{
		
		SetWindowText(g_window->hWnd,"Lesson 42: Multiple Viewports... 2003 NeHe Productions... Maze Complete!");
		Sleep(5000);
		SetWindowText(g_window->hWnd,"Lesson 42: Multiple Viewports... 2003 NeHe Productions... Building Maze!");
		Reset();
	}

	
	if (((mx>(width-4) || tex_data[(((mx+2)+(width*my))*3)]==255)) && ((mx<2 || tex_data[(((mx-2)+(width*my))*3)]==255)) &&
		((my>(height-4) || tex_data[((mx+(width*(my+2)))*3)]==255)) && ((my<2 || tex_data[((mx+(width*(my-2)))*3)]==255)))
	{
		do																
		{
			mx=int(rand()%(width/2))*2;									
			my=int(rand()%(height/2))*2;								
		}
		while (tex_data[((mx+(width*my))*3)]==0);						
	}																	

	dir=int(rand()%4);													

	if ((dir==0) && (mx<=(width-4)))									
	{
		if (tex_data[(((mx+2)+(width*my))*3)]==0)						
		{
			UpdateTex(mx+1,my);											
			mx+=2;														
		}
	}

	if ((dir==1) && (my<=(height-4)))									
	{
		if (tex_data[((mx+(width*(my+2)))*3)]==0)						
		{
			UpdateTex(mx,my+1);											
			my+=2;														
		}
	}

	if ((dir==2) && (mx>=2))											
	{
		if (tex_data[(((mx-2)+(width*my))*3)]==0)						
		{
			UpdateTex(mx-1,my);											
			mx-=2;														
		}
	}

	if ((dir==3) && (my>=2))											
	{
		if (tex_data[((mx+(width*(my-2)))*3)]==0)						
		{
			UpdateTex(mx,my-1);											
			my-=2;														
		}
	}

	UpdateTex(mx,my);													
}

void Draw (void)														
{
	RECT	rect;														

	GetClientRect(g_window->hWnd, &rect);								
	int window_width=rect.right-rect.left;								
	int window_height=rect.bottom-rect.top;								

	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, tex_data);

	glClear (GL_COLOR_BUFFER_BIT);										

	for (int loop=0; loop<4; loop++)									
	{
		glColor3ub(r[loop],g[loop],b[loop]);							

		if (loop==0)													
		{
			
			glViewport (0, window_height/2, window_width/2, window_height/2);
			glMatrixMode (GL_PROJECTION);								
			glLoadIdentity ();											
			
			gluOrtho2D(0, window_width/2, window_height/2, 0);
		}

		if (loop==1)													
		{
			
			glViewport (window_width/2, window_height/2, window_width/2, window_height/2);
			glMatrixMode (GL_PROJECTION);								
			glLoadIdentity ();											
			
			gluPerspective( 45.0, (GLfloat)(width)/(GLfloat)(height), 0.1f, 500.0 ); 
		}

		if (loop==2)													
		{
			
			glViewport (window_width/2, 0, window_width/2, window_height/2);
			glMatrixMode (GL_PROJECTION);								
			glLoadIdentity ();											
			
			gluPerspective( 45.0, (GLfloat)(width)/(GLfloat)(height), 0.1f, 500.0 ); 
		}

		if (loop==3)													
		{
			
			glViewport (0, 0, window_width/2, window_height/2);
			glMatrixMode (GL_PROJECTION);								
			glLoadIdentity ();											
			
			gluPerspective( 45.0, (GLfloat)(width)/(GLfloat)(height), 0.1f, 500.0 ); 
		}

		glMatrixMode (GL_MODELVIEW);									
		glLoadIdentity ();												

		glClear (GL_DEPTH_BUFFER_BIT);									

		if (loop==0)													
		{
			glBegin(GL_QUADS);											
				
				glTexCoord2f(1.0f, 0.0f); glVertex2i(window_width/2, 0              );
				glTexCoord2f(0.0f, 0.0f); glVertex2i(0,              0              );
				glTexCoord2f(0.0f, 1.0f); glVertex2i(0,              window_height/2);
				glTexCoord2f(1.0f, 1.0f); glVertex2i(window_width/2, window_height/2);
			glEnd();													
		}

		if (loop==1)													
		{
			glTranslatef(0.0f,0.0f,-14.0f);								

			glRotatef(xrot,1.0f,0.0f,0.0f);								
			glRotatef(yrot,0.0f,1.0f,0.0f);								
			glRotatef(zrot,0.0f,0.0f,1.0f);								

			glEnable(GL_LIGHTING);										
			gluSphere(quadric,4.0f,32,32);								
			glDisable(GL_LIGHTING);										
		}
		
		if (loop==2)													
		{
			glTranslatef(0.0f,0.0f,-2.0f);								
			glRotatef(-45.0f,1.0f,0.0f,0.0f);							
			glRotatef(zrot/1.5f,0.0f,0.0f,1.0f);						

			glBegin(GL_QUADS);											
				glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, 0.0f);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, 0.0f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, 0.0f);
			glEnd();													
		}

		if (loop==3)													
		{
			glTranslatef(0.0f,0.0f,-7.0f);								
			glRotatef(-xrot/2,1.0f,0.0f,0.0f);							
			glRotatef(-yrot/2,0.0f,1.0f,0.0f);							
			glRotatef(-zrot/2,0.0f,0.0f,1.0f);							

			glEnable(GL_LIGHTING);										
			glTranslatef(0.0f,0.0f,-2.0f);								
			gluCylinder(quadric,1.5f,1.5f,4.0f,32,16);					
			glDisable(GL_LIGHTING);										
		}
	}

	glFlush ();															
}
