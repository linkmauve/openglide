//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                    Buffer functions
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//**************************************************************

#include <stdio.h>

#include "glogl.h"

// extern variables
extern __int64          InitialTick,
                        FinalTick;
extern DWORD            Frame;
extern double           Fps, 
                        FpsAux, 
                        ClockFreq;
extern HDC              hDC;


// extern functions
void ConvertColorF( GrColor_t GlideColor, float &R, float &G, float &B, float &A );


//*************************************************
//* Clear all the Buffers
//*************************************************
DLLEXPORT void __stdcall
grBufferClear( GrColor_t color, GrAlpha_t alpha, FxU16 depth )
{
#ifdef OGL_CRITICAL
    GlideMsg( "grBufferClear( %d, %d, %d )\n", color, alpha, depth );
#endif
    static GrColor_t    old_color = 0;
    static float        BR = 0.0f, 
                        BG = 0.0f, 
                        BB = 0.0f, 
                        BA = 0.0f;

    if ( ( Glide.State.ClipMinX == 0 ) && 
         ( Glide.State.ClipMinY == 0 ) && 
         ( Glide.State.ClipMaxX == (FxU32) Glide.WindowWidth ) &&
         ( Glide.State.ClipMaxY == (FxU32) Glide.WindowHeight ) )
    {
        static unsigned int Bits;
        
        Bits = 0;
        
        RenderDrawTriangles( );
        
        if ( OpenGL.ColorMask )
        {
            Bits = GL_COLOR_BUFFER_BIT;
            if ( color != old_color )
            {
                old_color = color;
                ConvertColorF( color, BR, BG, BB, BA );
            }
            glClearColor( BR, BG, BB, BA );
        }
        
        if ( Glide.State.DepthBufferWritting )
        {
            glClearDepth( depth * D1OVER65536 );
            Bits |= GL_DEPTH_BUFFER_BIT;
        }
        
        glClear( Bits );
    }
    else
    {
        static GLboolean    alpha_test;
        static FxU32        oldDepth = 0x10000;
        static GLuint       clearList = glGenLists( 1 );  

        if ( depth != oldDepth )
        {
            oldDepth = depth;

            glNewList( clearList, GL_COMPILE );

                glDisable( GL_TEXTURE_2D );
                glDisable( GL_ALPHA_TEST );
                glDisable( GL_BLEND );
                glDisable( GL_DEPTH_TEST );
                glDisable( GL_CULL_FACE );

                glBegin( GL_TRIANGLE_STRIP );
                    glVertex3f( 0.0f,                      0.0f,                       depth * D1OVER65536 );
                    glVertex3f( (float) Glide.WindowWidth, 0.0f,                       depth * D1OVER65536 );
                    glVertex3f( (float) Glide.WindowWidth, (float) Glide.WindowHeight, depth * D1OVER65536 );
                    glVertex3f( 0.0f,                      (float) Glide.WindowHeight, depth * D1OVER65536 );
                glEnd( );

            glEndList( );
        }

        if ( color != old_color )
        {
            old_color = color;
            ConvertColorF( color, BR, BG, BB, BA );
        }

       /*
        * Remember alpha-test state because it is
        * unclear how it relates to the stored
        * Glide state
        */
        alpha_test = glIsEnabled( GL_ALPHA_TEST );

        glColor3f( BR, BG, BB );
        glCallList( clearList );

        if ( alpha_test )
        {
            glEnable( GL_ALPHA_TEST );
        }
        if ( Glide.State.DepthBufferMode != GR_DEPTHBUFFER_DISABLE )
        {
            glEnable( GL_DEPTH_TEST );
        }
        if ( Glide.State.CullMode != GR_CULL_DISABLE )
        {
            glEnable( GL_CULL_FACE );
        }
    }

#ifdef OPENGL_DEBUG
    GLErro( "grBufferClear" );
#endif
}

//*************************************************
//* Swaps Front and Back Buffers
//*************************************************
DLLEXPORT void __stdcall
grBufferSwap( int swap_interval )
{
#ifdef OGL_CRITICAL
    GlideMsg( "grBufferSwap( %d )\n", swap_interval );
#endif

    RenderDrawTriangles( );
    glFlush( );

#ifdef OGL_DEBUG
    static float    Temp = 1.0f;

    if ( OGLRender.FrameTriangles > OGLRender.MaxTriangles )
    {
        OGLRender.MaxTriangles = OGLRender.FrameTriangles;
    }
    OGLRender.FrameTriangles = 0;
#endif

    SwapBuffers( hDC );

#ifdef OGL_DEBUG
    RDTSC( FinalTick );
    Temp = (float)(FinalTick - InitialTick);
    FpsAux += Temp;
    Frame++;
    RDTSC( InitialTick );
#endif

#ifdef OPENGL_DEBUG
    GLErro( "grBufferSwap" );
#endif
}

//*************************************************
//* Return the number of queued buffer swap requests
//* Always 0, never pending
//*************************************************
DLLEXPORT int __stdcall
grBufferNumPending( void )
{
#ifdef OGL_DONE
    GlideMsg("grBufferNumPending()\n");
#endif

    return 0; 
}

//*************************************************
//* Defines the Buffer to Render
//*************************************************
DLLEXPORT void __stdcall
grRenderBuffer(GrBuffer_t dwBuffer)
{
#ifdef OGL_DONE
    GlideMsg("grRenderBuffer( %d )\n", dwBuffer);
#endif

    RenderDrawTriangles();

    Glide.State.RenderBuffer = dwBuffer;

    switch ( dwBuffer )
    {
    case GR_BUFFER_FRONTBUFFER:
        OpenGL.RenderBuffer = GL_FRONT;
        break;
    case GR_BUFFER_BACKBUFFER:
        OpenGL.RenderBuffer = GL_BACK;
        break;
    case GR_BUFFER_AUXBUFFER:
    case GR_BUFFER_DEPTHBUFFER:
    case GR_BUFFER_ALPHABUFFER:
    case GR_BUFFER_TRIPLEBUFFER:
        Error("Unsupported parameter in grRenderBuffer();\n");
        return;
    }

    glDrawBuffer( OpenGL.RenderBuffer );

#ifdef OPENGL_DEBUG
    GLErro( "grRenderBuffer" );
#endif
}

