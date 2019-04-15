#include <windows.h>													
#include <gl.h>														
#include <glu.h>														
#include <cg\cg.h>														
#include <cg\cggl.h>													
#include "NeHeGL.h"														
#pragma comment( lib, "opengl32.lib" )									
#pragma comment( lib, "glu32.lib" )										
#pragma comment( lib, "cg.lib" )										
#pragma comment( lib, "cggl.lib" )										
#define		TWO_PI 6.2831853071											
GL_Window*	g_window;													
Keys*		g_keys;														
#define		SIZE	64													
bool		cg_enable = TRUE, sp;										
GLfloat		mesh[SIZE][SIZE][3];										
GLfloat		wave_movement = 0.0f;										
CGcontext	cgContext;													
CGprogram	cgProgram;													
CGprofile	cgVertexProfile;											
CGparameter	position, color, modelViewMatrix, wave;						
BOOL Initialize (GL_Window* window, Keys* keys)							
{
	g_window	= window;												
	g_keys		= keys;													
	
	glClearColor (0.0f, 0.0f, 0.0f, 0.5f);								
	glClearDepth (1.0f);												
	glDepthFunc (GL_LEQUAL);											
	glEnable (GL_DEPTH_TEST);											
	glShadeModel (GL_SMOOTH);											
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);					
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);							
	
	for (int x = 0; x < SIZE; x++)
	{
		for (int z = 0; z < SIZE; z++)
		{
			mesh[x][z][0] = (float) (SIZE / 2) - x;						
			mesh[x][z][1] = 0.0f;										
			mesh[x][z][2] = (float) (SIZE / 2) - z;						
		}
	}
	
	cgContext = cgCreateContext();										
	
	if (cgContext == NULL)
	{
		MessageBox(NULL, "Failed To Create Cg Context", "Error", MB_OK);
		return FALSE;													
	}
	cgVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);				
	
	if (cgVertexProfile == CG_PROFILE_UNKNOWN)
	{
		MessageBox(NULL, "Invalid profile type", "Error", MB_OK);
		return FALSE;													
	}
	cgGLSetOptimalOptions(cgVertexProfile);								
	
	cgProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, "CG/Wave.cg", cgVertexProfile, "main", 0);
	
	if (cgProgram == NULL)
	{
		
		CGerror Error = cgGetError();
		
		MessageBox(NULL, cgGetErrorString(Error), "Error", MB_OK);
		return FALSE;													
	}
	
	cgGLLoadProgram(cgProgram);
	
	
	position		= cgGetNamedParameter(cgProgram, "IN.position");
	color			= cgGetNamedParameter(cgProgram, "IN.color");
	wave			= cgGetNamedParameter(cgProgram, "IN.wave");
	modelViewMatrix	= cgGetNamedParameter(cgProgram, "ModelViewProj");
	return TRUE;														
}
void Deinitialize (void)												
{
	
	cgDestroyContext(cgContext);
}
void Update (float milliseconds)										
{
	if (g_keys->keyDown [VK_ESCAPE])									
		TerminateApplication (g_window);								
	if (g_keys->keyDown [VK_F1])										
		ToggleFullscreen (g_window);									
	if (g_keys->keyDown [' '] && !sp)
	{
		sp=TRUE;
		cg_enable=!cg_enable;
	}
	if (!g_keys->keyDown [' '])
		sp=FALSE;
}
void Draw (void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				
	glLoadIdentity ();													
	
	gluLookAt(0.0f, 25.0f, -45.0f, 0.0f, 0.0f, 0.0f, 0, 1, 0);
	
	cgGLSetStateMatrixParameter(modelViewMatrix, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	if (cg_enable)
	{
		cgGLEnableProfile(cgVertexProfile);								
		
		cgGLBindProgram(cgProgram);
		
		cgGLSetParameter4f(color, 0.5f, 1.0f, 0.5f, 1.0f);
	}
	
	for (int x = 0; x < SIZE - 1; x++)
	{
		
		glBegin(GL_TRIANGLE_STRIP);
		for (int z = 0; z < SIZE - 1; z++)
		{
			
			cgGLSetParameter3f(wave, wave_movement, 1.0f, 1.0f);
			glVertex3f(mesh[x][z][0], mesh[x][z][1], mesh[x][z][2]);		
			glVertex3f(mesh[x+1][z][0], mesh[x+1][z][1], mesh[x+1][z][2]);	
			wave_movement += 0.00001f;									
			if (wave_movement > TWO_PI)									
				wave_movement = 0.0f;
		}
		glEnd();
	}
	if (cg_enable)
		cgGLDisableProfile(cgVertexProfile);							
	glFlush ();															
}
