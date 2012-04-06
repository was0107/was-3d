//
//  RenderingEngine1.cpp
//  HelloOpengl
//
//  Created by was cienet on 05/04/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#include "IRenderingEngine.hpp"

static const float RevolutioonsPerSecond = 1;


class RenderingEngine1:public IRenderingEngine
{
    public:
    
    RenderingEngine1();
    
    void Initialize(int width, int height);
    
    void Render() const; 
    
    void UpdateAnimation(float timeStep);
    
    void OnRotate(DeviceOrientation newOrientation);
    
    private:
    
    float  RotationDirection() const;
    
    private:
    
    float  m_desireAngle;
    
    float  m_currentAngle;
    
    GLuint m_frameBuffer;
    
    GLuint m_renderBuffer;
};

IRenderingEngine * CreateRender1()
{
    return new RenderingEngine1();
}

struct vertex 
{
    float Position[2];
    float Color[4];
};

const vertex verties[] = 
{
    {{-0.5, -0.866}, {1, 1, 0.5f, 1}},
    {{0.5, -0.866},  {1, 1, 0.5f, 1}},
    {{0, 1},         {1, 1, 0.5f, 1}},
    {{-0.5, -0.866}, {0.5f, 0.5f, 0.5f}},
    {{0.5, -0.866},  {0.5f, 0.5f, 0.5f}},
    {{0, -0.4f},     {0.5f, 0.5f, 0.5f}},
    
};

RenderingEngine1::RenderingEngine1()
{
    glGenRenderbuffersOES(1, &m_renderBuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_renderBuffer);
}

void RenderingEngine1::Initialize(int width, int height)
{
    glGenFramebuffersOES(1, &m_frameBuffer);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_frameBuffer);
    
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, 
                                 GL_COLOR_ATTACHMENT0_OES,
                                 GL_RENDERBUFFER_OES,
                                 m_renderBuffer);
    
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    
    const float maxX = 2; 
    const float maxY = 3;
    
    glOrthof(-maxX, +maxX, -maxY, +maxY, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    
    
    OnRotate(DeviceOrientationPortrait);
    
    m_currentAngle = m_desireAngle;
    
    
}

void RenderingEngine1::Render() const
{
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glPushMatrix();
    glRotatef(m_currentAngle, 0, 0, 1);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(2, GL_FLOAT, sizeof(vertex), &verties[0].Position[0]);
    glColorPointer(4, GL_FLOAT, sizeof(vertex), &verties[0].Color[0]);
    
    GLsizei vertexCount = sizeof(verties)/sizeof(vertex);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glPopMatrix();

}

void RenderingEngine1::OnRotate(DeviceOrientation newOrientation)
{
    float angle = 0 ; 
    switch (newOrientation) 
    {
        case DeviceOrientationLandscapeLeft:
            angle = 270;
            break;
        case DeviceOrientationPortraitUpsideDown:
            angle = 180;
            break;
        case DeviceOrientationLandscapeRight:
            angle = 90;
            break;
        default:
            break;
    }
    m_desireAngle = angle;
}

float RenderingEngine1::RotationDirection() const
{
    float delta = m_desireAngle - m_currentAngle;
    
    if (0 == delta)
    {
        return 0;
    }
    
    bool counterClockWise = ((delta > 0 && delta <= 180) || (delta < -180));
    
    return counterClockWise? +1: -1;                            
    
}

void RenderingEngine1::UpdateAnimation(float timeStep)
{
    float direction = RotationDirection();
    if (0 == direction)
    {
        return;
    }
    
    float degrees = timeStep * 360 * RevolutioonsPerSecond;
    
    m_currentAngle += degrees * direction;
    
    if (m_currentAngle >= 360)
    {
        m_currentAngle -= 360;
    }
    else if (m_currentAngle < 0)
    {
        m_currentAngle += 360;
    }
    
    if (direction != RotationDirection())
    {
        m_currentAngle = m_desireAngle;
    }
}














