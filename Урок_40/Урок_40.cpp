#include <windows.h>														
#include <gl.h>															
#include <glu.h>															
#include "NeHeGL.h"															

#include "Physics2.h"														

#pragma comment( lib, "opengl32.lib" )										
#pragma comment( lib, "glu32.lib" )											

#ifndef CDS_FULLSCREEN														
#define CDS_FULLSCREEN 4													
#endif																		

GL_Window*	g_window;
Keys*		g_keys;

 

RopeSimulation* ropeSimulation = new RopeSimulation(
													80,						
													0.05f,					
													10000.0f,				
													0.05f,					
													0.2f,					
													Vector3D(0, -9.81f, 0), 
													0.02f,					
													100.0f,					
													0.2f,					
													2.0f,					
													-1.5f);					

BOOL Initialize (GL_Window* window, Keys* keys)								
{
	g_window	= window;
	g_keys		= keys;

	ropeSimulation->getMass(ropeSimulation->numOfMasses - 1)->vel.z = 10.0f;

	glClearColor (0.0f, 0.0f, 0.0f, 0.5f);									
	glClearDepth (1.0f);													
	glShadeModel (GL_SMOOTH);												
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);						

	return TRUE;															
}

void Deinitialize (void)													
{
	ropeSimulation->release();												
	delete(ropeSimulation);													
	ropeSimulation = NULL;
}

void Update (DWORD milliseconds)											
{
	if (g_keys->keyDown [VK_ESCAPE] == TRUE)								
		TerminateApplication (g_window);									

	if (g_keys->keyDown [VK_F1] == TRUE)									
		ToggleFullscreen (g_window);										

	Vector3D ropeConnectionVel;												

	
	if (g_keys->keyDown [VK_RIGHT] == TRUE)									
		ropeConnectionVel.x += 3.0f;										

	if (g_keys->keyDown [VK_LEFT] == TRUE)									
		ropeConnectionVel.x -= 3.0f;										

	if (g_keys->keyDown [VK_UP] == TRUE)									
		ropeConnectionVel.z -= 3.0f;										

	if (g_keys->keyDown [VK_DOWN] == TRUE)									
		ropeConnectionVel.z += 3.0f;										

	if (g_keys->keyDown [VK_HOME] == TRUE)									
		ropeConnectionVel.y += 3.0f;										

	if (g_keys->keyDown [VK_END] == TRUE)									
		ropeConnectionVel.y -= 3.0f;										

	ropeSimulation->setRopeConnectionVel(ropeConnectionVel);				

	float dt = milliseconds / 1000.0f;										

	float maxPossible_dt = 0.002f;											
																			

  	int numOfIterations = (int)(dt / maxPossible_dt) + 1;					
	if (numOfIterations != 0)												
		dt = dt / numOfIterations;											

	for (int a = 0; a < numOfIterations; ++a)								
		ropeSimulation->operate(dt);
}

void Draw (void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity ();														
	
	
	
	gluLookAt(0, 0, 4, 0, 0, 0, 0, 1, 0);						

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);					

	
	glBegin(GL_QUADS);
		glColor3ub(0, 0, 255);												
		glVertex3f(20, ropeSimulation->groundHeight, 20);
		glVertex3f(-20, ropeSimulation->groundHeight, 20);
		glColor3ub(0, 0, 0);												
		glVertex3f(-20, ropeSimulation->groundHeight, -20);
		glVertex3f(20, ropeSimulation->groundHeight, -20);
	glEnd();
	
	
	glColor3ub(0, 0, 0);													
	for (int a = 0; a < ropeSimulation->numOfMasses - 1; ++a)
	{
		Mass* mass1 = ropeSimulation->getMass(a);
		Vector3D* pos1 = &mass1->pos;

		Mass* mass2 = ropeSimulation->getMass(a + 1);
		Vector3D* pos2 = &mass2->pos;

		glLineWidth(2);
		glBegin(GL_LINES);
			glVertex3f(pos1->x, ropeSimulation->groundHeight, pos1->z);		
			glVertex3f(pos2->x, ropeSimulation->groundHeight, pos2->z);		
		glEnd();
	}
	

	
	glColor3ub(255, 255, 0);												
	for (a = 0; a < ropeSimulation->numOfMasses - 1; ++a)
	{
		Mass* mass1 = ropeSimulation->getMass(a);
		Vector3D* pos1 = &mass1->pos;

		Mass* mass2 = ropeSimulation->getMass(a + 1);
		Vector3D* pos2 = &mass2->pos;

		glLineWidth(4);
		glBegin(GL_LINES);
			glVertex3f(pos1->x, pos1->y, pos1->z);
			glVertex3f(pos2->x, pos2->y, pos2->z);
		glEnd();
	}
	
	
	glFlush ();																
}
