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
#include "Quaternion.hpp"
#include "IRenderingEngine.hpp"
#include <vector>

using namespace std;

#include "Simple.frag"
#include "Simple.vert"

static const float RevolutionPerSecond = 1; 
static const float AnimationDuration = 0.25f;

struct vertex
{
    vec3 Position;
    vec4 Color;
};

struct Animation
{
    Quaternion Start;
    Quaternion End;
    Quaternion Current;
    float Elapsed;
    float Duration;
};

class RenderingEngine2: public IRenderingEngine
{
    
public:
    
    RenderingEngine2();
    
    void Initialize(int width, int height);
    
    void Render() const; 
    
    void UpdateAnimation(float timeStep);
    
    void OnRotate(DeviceOrientation newOrientation);

private:
    
    GLuint BuildShader(const char* source, GLenum shaderType) const;
    
    GLuint BuildProgram(const char* vShader, const char* fShader) const;
    
private:
    
    vector<vertex> m_cone;
    vector<vertex> m_disk;
    
    Animation m_animation;
    
    GLuint m_simpleProgram;
    GLuint m_frameBuffer;
    GLuint m_colorRenderBuffer;
    GLuint m_depthRenderBuffer;

};


struct IRenderingEngine *CreateRender2()
{
    return new RenderingEngine2();
}

RenderingEngine2::RenderingEngine2()
{
    glGenRenderbuffers(1, &m_colorRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderBuffer);
}

void RenderingEngine2::Render() const
{
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
    GLuint positionSlot = glGetAttribLocation(m_simpleProgram, "Position");
    GLuint colorSlot    = glGetAttribLocation(m_simpleProgram, "SourceColor");
    
    
    glEnableVertexAttribArray(positionSlot);
    glEnableVertexAttribArray(colorSlot);
    
    mat4 rotation(m_animation.Current.ToMatrix());
    mat4 translation  = mat4::Translate(0, 0, -7);
    
    GLuint modelViewUniform = glGetUniformLocation(m_simpleProgram, "Modelview");
    mat4   modelviewMatrix  = translation * rotation;
    glUniformMatrix4fv(modelViewUniform, 1, 0, modelviewMatrix.Pointer());
    
    {
        GLsizei stride = sizeof(vertex);
        const GLvoid* pCoords = &m_cone[0].Position.x;
        const GLvoid* pColors = &m_cone[0].Color.x;
        
        glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE,stride,pCoords );
        glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE,stride,pColors );
        glDrawArrays(GL_TRIANGLE_STRIP, 0, m_cone.size());
    }
    {
        GLsizei stride = sizeof(vertex);
        const GLvoid* pCoords = &m_disk[0].Position.x;
        const GLvoid* pColors = &m_disk[0].Color.x;
        
        glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE,stride,pCoords );
        glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE,stride,pColors );
        glDrawArrays(GL_TRIANGLE_FAN, 0, m_disk.size());
    }
    
    glDisableVertexAttribArray(positionSlot);
    glDisableVertexAttribArray(colorSlot);

}

void RenderingEngine2::Initialize(int width, int height)
{
    const float coneRadious = 0.5f;
    const float coneHeight  = 1.866f;
    const int   coneSlices  = 40;
    
    m_cone.resize((coneSlices + 1) * 2 );
    vector<vertex>::iterator verIt = m_cone.begin();
    const float dtheta = TwoPi / coneSlices;
    
    for(float theta = 0 ; verIt != m_cone.end(); theta += dtheta )
    {
        float brightness = abs(sin(theta));
        vec4 color(brightness, brightness, brightness,1);
        verIt->Position =  vec3(0,1,0);
        verIt->Color    = color;
        verIt++;
        
        verIt->Position.x = coneRadious * cos(theta);
        verIt->Position.y = 1 - coneHeight;
        verIt->Position.z = coneRadious * sin(theta);
        verIt->Color = color;
        
        verIt++;
        
    }
    {
        m_disk.resize(coneSlices + 2);
        vector<vertex>::iterator verIt = m_disk.begin();
        verIt->Color = vec4(0.75, 0.75, 0.75,1);
        verIt->Position.x = 0;
        verIt->Position.y = 1 - coneHeight;
        verIt->Position.z = 0;
        verIt++;
        
        const float dtheta = TwoPi/ coneSlices;
        for (float theTa = 0; verIt != m_disk.end(); theTa += dtheta)
        {
            verIt->Color = vec4(0.75, 0.75, 0.75,1);
            verIt->Position.x = coneRadious * cos(theTa);
            verIt->Position.y = 1 - coneHeight;
            verIt->Position.z = coneRadious * sin(theTa);
            verIt++;
            
        }
    }
    
    // Create the depth buffer 
    glGenRenderbuffers(1, &m_depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER,
                             GL_DEPTH_COMPONENT16,
                             width, 
                             height);
    
    
    glGenFramebuffers(1, &m_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, 
                                 GL_COLOR_ATTACHMENT0,
                                 GL_RENDERBUFFER,
                                 m_colorRenderBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                 GL_DEPTH_ATTACHMENT, 
                                 GL_RENDERBUFFER,
                                 m_depthRenderBuffer);
    
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderBuffer);
    
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    
    
    m_simpleProgram = BuildProgram(SimpleVertexShader, SimpleFragmentShader);
    glUseProgram(m_simpleProgram);
    
    GLint projectionUniform = glGetUniformLocation(m_simpleProgram, "Projection");
    mat4 projectionMatrix = mat4::Frustum(-1.6f, 1.6, -2.4, 2.4, 5, 10);
    glUniformMatrix4fv(projectionUniform, 1, 0, projectionMatrix.Pointer());
    
}
void RenderingEngine2::OnRotate(DeviceOrientation newOrientation)
{
    vec3 direction;
    
    switch (newOrientation) 
    {
        case DeviceOrientationPortrait:
        case DeviceOrientationUnknown:
            direction = vec3(0,1,0);
            break;  
        case DeviceOrientationFaceUp:  
            direction = vec3(0,0,1);
            break;
        case DeviceOrientationfaceDown:  
            direction = vec3(0,0,-1);
            break;
        case DeviceOrientationLandscapeLeft:
            direction = vec3(1,0,0);
            break;
        case DeviceOrientationPortraitUpsideDown:
            direction = vec3(0,-1,0);
            break;
        case DeviceOrientationLandscapeRight:
            direction = vec3(-1,0,0);
            break;
        default:
            break;
    }
    m_animation.Elapsed = 0;
    m_animation.Start   = m_animation.Current = m_animation.End;
    m_animation.End     = Quaternion::CreateFromVectors(vec3(0,1,0),
                                                        direction);
}


void RenderingEngine2::UpdateAnimation(float timeStep)
{
    if (m_animation.Current == m_animation.End)
    {
        return;
    }
    
    m_animation.Elapsed += timeStep;
    if (m_animation.Elapsed >= AnimationDuration)
    {
        m_animation.Current = m_animation.End;
    }
    else
    {
        float mu = m_animation.Elapsed / AnimationDuration;
        m_animation.Current = m_animation.Start.Slerp(mu, m_animation.End);
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











