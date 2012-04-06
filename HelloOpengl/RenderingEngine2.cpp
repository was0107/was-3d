//
//  RenderingEngine2.cpp
//  HelloOpengl
//
//  Created by was cienet on 05/04/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//


#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <cmath>
#include <iostream>
#include "IRenderingEngine.hpp"

#include "Simple.frag"
#include "Simple.vert"

static const float RevolutionPerSecond = 1; 

class RenderingEngine2: public IRenderingEngine
{
    
public:
    
    RenderingEngine2();
    
    void Initialize(int width, int height);
    
    void Render() const; 
    
    void UpdateAnimation(float timeStep);
    
    void OnRotate(DeviceOrientation newOrientation);

private:
    
    float  RotationDirection() const;
    
    GLuint BuildShader(const char* source, GLenum shaderType) const;
    
    GLuint BuildProgram(const char* vShader, const char* fShader) const;
    
    void   ApplyOrtho(float maxX, float maxY) const;
    
    void   ApplyRotation(float degrees) const;
    
private:
    
    float  m_desireAngle;
    
    float  m_currentAngle;
    
    GLuint m_simpleProgram;
    
    GLuint m_frameBuffer;
    
    GLuint m_renderBuffer;

};

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

struct IRenderingEngine *CreateRender2()
{
    return new RenderingEngine2();
}

RenderingEngine2::RenderingEngine2()
{
    glGenRenderbuffers(1, &m_renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
}

void RenderingEngine2::Render() const
{
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    ApplyRotation(m_currentAngle);
    
    GLuint positionSlot = glGetAttribLocation(m_simpleProgram, "Position");
    GLuint colorSlot    = glGetAttribLocation(m_simpleProgram, "SourceColor");
    
    
    glEnableVertexAttribArray(positionSlot);
    glEnableVertexAttribArray(colorSlot);
    
    GLsizei stride = sizeof(vertex);
    const GLvoid* pCoords = &verties[0].Position[0];
    const GLvoid* pColors = &verties[0].Color[0];
    
    glVertexAttribPointer(positionSlot, 2, GL_FLOAT, GL_FALSE, stride, pCoords);
    glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, pColors);
    
    
    GLsizei vertexCount = sizeof(verties)/sizeof(vertex);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    
    glDisableVertexAttribArray(positionSlot);
    glDisableVertexAttribArray(colorSlot);

}

void RenderingEngine2::Initialize(int width, int height)
{
    glGenFramebuffers(1, &m_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_COLOR_ATTACHMENT0, 
                              GL_RENDERBUFFER, 
                              m_renderBuffer);
    
    glViewport(0, 0, width, height);
    
    m_simpleProgram = BuildProgram(SimpleVertexShader, SimpleFragmentShader);
    glUseProgram(m_simpleProgram);
    
    ApplyOrtho(2, 3);
    
    OnRotate(DeviceOrientationPortrait);
    m_currentAngle = m_desireAngle;
    
}
void RenderingEngine2::OnRotate(DeviceOrientation newOrientation)
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

float RenderingEngine2::RotationDirection() const
{
    float delta = m_desireAngle - m_currentAngle;
    
    if (0 == delta)
    {
        return 0;
    }
    
    bool counterClockWise = ((delta > 0 && delta <= 180) || (delta < -180));
    
    return counterClockWise? +1: -1;                            
    
}

void RenderingEngine2::UpdateAnimation(float timeStep)
{
    float direction = RotationDirection();
    if (0 == direction)
    {
        return;
    }
    
    float degrees = timeStep * 360 * RevolutionPerSecond;
    
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

GLuint RenderingEngine2::BuildShader(const char *source, GLenum shaderType) const
{
    GLuint shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &source, 0);
    glCompileShader(shaderHandle);
    
    
    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    
    if(GL_FALSE == compileSuccess)
    {
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        std::cout<<messages;
        exit(1);
    }
    
    return shaderHandle;
}


GLuint RenderingEngine2::BuildProgram(const char *vShader, const char *fShader) const
{
    GLuint vertexShader = BuildShader(vShader, GL_VERTEX_SHADER);
    GLuint fragementShader = BuildShader(fShader, GL_FRAGMENT_SHADER);
    
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragementShader);
    glLinkProgram(programHandle);
    
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    if(GL_FALSE == linkSuccess)
    {
        GLchar messages[256];
        glGetShaderInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        std::cout<<messages;
        exit(1);

    }
    
    return programHandle;
}

void RenderingEngine2::ApplyOrtho(float maxX, float maxY) const
{
    float a = 1.0f/maxX;
    float b = 1.0f/maxY;
    float ortho[16] = 
    {
        a,0,0,0,
        0,b,0,0,
        0,0,-1,0,
        0,0,0,1
    };
    
    GLint projectionUniform = glGetUniformLocation(m_simpleProgram, "Projection");
    glUniformMatrix4fv(projectionUniform, 1, 0, &ortho[0]);
}


void RenderingEngine2::ApplyRotation(float degrees) const
{
    float radians = degrees * 3.14159f / 180.0f;
    float s = std::sin(radians);
    float c = std::cos(radians);
    float zRotation[16] = 
    {
        c,s,0,0,
        -s,c,0,0,
        0,0,1,0,
        0,0,0,1
        
    };
    
    GLint modelViewUniform = glGetUniformLocation(m_simpleProgram, "Modelview");
    glUniformMatrix4fv(modelViewUniform, 1, 0, &zRotation[0]);
    
}












