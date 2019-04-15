#include <windows.h>											
#include <gl.h>												
#include <glu.h>												
#include <glaux.h>											
#include <stdio.h>												
#include "NeHeGL.h"												

#pragma comment( lib, "opengl32.lib" )							
#pragma comment( lib, "glu32.lib" )								
#pragma comment( lib, "glaux.lib" )								

#ifndef CDS_FULLSCREEN											
#define CDS_FULLSCREEN 4										
#endif															



#define MESH_RESOLUTION 4.0f									
#define MESH_HEIGHTSCALE 1.0f									



#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_STATIC_DRAW_ARB 0x88E4
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, int size, const GLvoid *data, GLenum usage);


PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;					
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;					
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;					
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;			

class CVert														
{
public:
	float x;													
	float y;													
	float z;													
};
typedef CVert CVec;												

class CTexCoord													
{
public:
	float u;													
	float v;													
};

class CMesh
{
public:
	
	int				m_nVertexCount;								
	CVert*			m_pVertices;								
	CTexCoord*		m_pTexCoords;								
	unsigned int	m_nTextureId;								

	
	unsigned int	m_nVBOVertices;								
	unsigned int	m_nVBOTexCoords;							

	
	AUX_RGBImageRec* m_pTextureImage;							

public:
	CMesh();													
	~CMesh();													

	
	bool LoadHeightmap( char* szPath, float flHeightScale, float flResolution );
	
	float PtHeight( int nX, int nY );
	
	void BuildVBOs();
};

bool		g_fVBOSupported = false;							
CMesh*		g_pMesh = NULL;										
float		g_flYRot = 0.0f;									
int			g_nFPS = 0, g_nFrames = 0;							
DWORD		g_dwLastFPS = 0;									


GL_Window*	g_window;
Keys*		g_keys;



bool IsExtensionSupported( char* szTargetExtension )
{
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	
	pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
	if( pszWhere || *szTargetExtension == '\0' )
		return false;

	
	pszExtensions = glGetString( GL_EXTENSIONS );

	
	pszStart = pszExtensions;
	for(;;)
	{
		pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
		if( !pszWhere )
			break;
		pszTerminator = pszWhere + strlen( szTargetExtension );
		if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
			if( *pszTerminator == ' ' || *pszTerminator == '\0' )
				return true;
		pszStart = pszTerminator;
	}
	return false;
}


BOOL Initialize (GL_Window* window, Keys* keys)					
{
	g_window	= window;
	g_keys		= keys;

	
	
	g_pMesh = new CMesh();										
	if( !g_pMesh->LoadHeightmap( "terrain.bmp",					
								MESH_HEIGHTSCALE,
								MESH_RESOLUTION ) )
	{
		MessageBox( NULL, "Error Loading Heightmap", "Error", MB_OK );
		return false;
	}

	
#ifndef NO_VBOS
	g_fVBOSupported = IsExtensionSupported( "GL_ARB_vertex_buffer_object" );
	if( g_fVBOSupported )
	{
		
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress("glGenBuffersARB");
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress("glBindBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress("glBufferDataARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress("glDeleteBuffersARB");
		
		g_pMesh->BuildVBOs();									
	}
#else  

	g_fVBOSupported = false;
#endif
	
	
	
	glClearColor (0.0f, 0.0f, 0.0f, 0.5f);						
	glClearDepth (1.0f);										
	glDepthFunc (GL_LEQUAL);									
	glEnable (GL_DEPTH_TEST);									
	glShadeModel (GL_SMOOTH);									
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			
	glEnable( GL_TEXTURE_2D );									
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );						

	return TRUE;												
}

void Deinitialize (void)										
{
	if( g_pMesh )												
		delete g_pMesh;											
	g_pMesh = NULL;
}

void Update (DWORD milliseconds)								
{
	g_flYRot += (float) ( milliseconds ) / 1000.0f * 25.0f;		

	if (g_keys->keyDown [VK_ESCAPE] == TRUE)					
	{
		TerminateApplication (g_window);						
	}

	if (g_keys->keyDown [VK_F1] == TRUE)						
	{
		ToggleFullscreen (g_window);							
	}
}

void Draw (void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
	glLoadIdentity ();											

	
	if( GetTickCount() - g_dwLastFPS >= 1000 )					
	{
		g_dwLastFPS = GetTickCount();							
		g_nFPS = g_nFrames;										
		g_nFrames = 0;											

		char szTitle[256]={0};									
		sprintf( szTitle, "Lesson 45: NeHe & Paul Frazee's VBO Tut - %d Triangles, %d FPS", g_pMesh->m_nVertexCount / 3, g_nFPS );
		if( g_fVBOSupported )									
			strcat( szTitle, ", Using VBOs" );
		else
			strcat( szTitle, ", Not Using VBOs" );
		SetWindowText( g_window->hWnd, szTitle );				
	}
	g_nFrames++;												
	
	
	glTranslatef( 0.0f, -220.0f, 0.0f );						
	glRotatef( 10.0f, 1.0f, 0.0f, 0.0f );						
	glRotatef( g_flYRot, 0.0f, 1.0f, 0.0f );					

	
	glEnableClientState( GL_VERTEX_ARRAY );						
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );				

	
	if( g_fVBOSupported )
	{
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, g_pMesh->m_nVBOVertices );
		glVertexPointer( 3, GL_FLOAT, 0, (char *) NULL );		
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, g_pMesh->m_nVBOTexCoords );
		glTexCoordPointer( 2, GL_FLOAT, 0, (char *) NULL );		
	} else
	{
		glVertexPointer( 3, GL_FLOAT, 0, g_pMesh->m_pVertices ); 
		glTexCoordPointer( 2, GL_FLOAT, 0, g_pMesh->m_pTexCoords ); 
	}

	
	glDrawArrays( GL_TRIANGLES, 0, g_pMesh->m_nVertexCount );	

	
	glDisableClientState( GL_VERTEX_ARRAY );					
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );				
}

CMesh :: CMesh()
{
	
	m_pTextureImage = NULL;
	m_pVertices = NULL;
	m_pTexCoords = NULL;
	m_nVertexCount = 0;
	m_nVBOVertices = m_nVBOTexCoords = m_nTextureId = 0;
}

CMesh :: ~CMesh()
{
	
	if( g_fVBOSupported )
	{
		unsigned int nBuffers[2] = { m_nVBOVertices, m_nVBOTexCoords };
		glDeleteBuffersARB( 2, nBuffers );						
	}
	
	if( m_pVertices )											
		delete [] m_pVertices;
	m_pVertices = NULL;
	if( m_pTexCoords )											
		delete [] m_pTexCoords;
	m_pTexCoords = NULL;
}

bool CMesh :: LoadHeightmap( char* szPath, float flHeightScale, float flResolution )
{
	
	FILE* fTest = fopen( szPath, "r" );							
	if( !fTest )												
		return false;											
	fclose( fTest );											

	
	m_pTextureImage = auxDIBImageLoad( szPath );				

	
	m_nVertexCount = (int) ( m_pTextureImage->sizeX * m_pTextureImage->sizeY * 6 / ( flResolution * flResolution ) );
	m_pVertices = new CVec[m_nVertexCount];						
	m_pTexCoords = new CTexCoord[m_nVertexCount];				
	int nX, nZ, nTri, nIndex=0;									
	float flX, flZ;
	for( nZ = 0; nZ < m_pTextureImage->sizeY; nZ += (int) flResolution )
	{
		for( nX = 0; nX < m_pTextureImage->sizeX; nX += (int) flResolution )
		{
			for( nTri = 0; nTri < 6; nTri++ )
			{
				
				flX = (float) nX + ( ( nTri == 1 || nTri == 2 || nTri == 5 ) ? flResolution : 0.0f );
				flZ = (float) nZ + ( ( nTri == 2 || nTri == 4 || nTri == 5 ) ? flResolution : 0.0f );

				
				m_pVertices[nIndex].x = flX - ( m_pTextureImage->sizeX / 2 );
				m_pVertices[nIndex].y = PtHeight( (int) flX, (int) flZ ) *  flHeightScale;
				m_pVertices[nIndex].z = flZ - ( m_pTextureImage->sizeY / 2 );

				
				m_pTexCoords[nIndex].u = flX / m_pTextureImage->sizeX;
				m_pTexCoords[nIndex].v = flZ / m_pTextureImage->sizeY;

				
				nIndex++;
			}
		}
	}

	
	glGenTextures( 1, &m_nTextureId );							
	glBindTexture( GL_TEXTURE_2D, m_nTextureId );				
	glTexImage2D( GL_TEXTURE_2D, 0, 3, m_pTextureImage->sizeX, m_pTextureImage->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTextureImage->data );
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	
	if( m_pTextureImage )
	{
		if( m_pTextureImage->data )
			free( m_pTextureImage->data );
		free( m_pTextureImage );
	}
	return true;
}

float CMesh :: PtHeight( int nX, int nY )
{
	
	int nPos = ( ( nX % m_pTextureImage->sizeX )  + ( ( nY % m_pTextureImage->sizeY ) * m_pTextureImage->sizeX ) ) * 3;
	float flR = (float) m_pTextureImage->data[ nPos ];			
	float flG = (float) m_pTextureImage->data[ nPos + 1 ];		
	float flB = (float) m_pTextureImage->data[ nPos + 2 ];		
	return ( 0.299f * flR + 0.587f * flG + 0.114f * flB );		
}

void CMesh :: BuildVBOs()
{
	
	glGenBuffersARB( 1, &m_nVBOVertices );							
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );			
	
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, m_nVertexCount*3*sizeof(float), m_pVertices, GL_STATIC_DRAW_ARB );

	
	glGenBuffersARB( 1, &m_nVBOTexCoords );							
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOTexCoords );		
	
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, m_nVertexCount*2*sizeof(float), m_pTexCoords, GL_STATIC_DRAW_ARB );

	
	delete [] m_pVertices; m_pVertices = NULL;
	delete [] m_pTexCoords; m_pTexCoords = NULL;
