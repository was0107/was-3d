//
//  RenderingEngine1.cpp
//  HelloOpengl
//
//  Created by was cienet on 05/04/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include "Quaternion.hpp"
#include "IRenderingEngine.hpp"
#include <vector>

using namespace std;

static const float RevolutioonsPerSecond = 1;
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


class RenderingEngine1:public IRenderingEngine
{
public:
    
    RenderingEngine1();
    
    void Initialize(int width, int height);
    
    void Render() const; 
    
    void UpdateAnimation(float timeStep);
    
    void OnRotate(DeviceOrientation newOrientation);
    
private:
    
    vector<vertex> m_cone;
    vector<vertex> m_disk;
    
    Animation m_animation;
    
    GLuint m_frameBuffer;
    GLuint m_colorRenderBuffer;
    GLuint m_depthRenderBuffer;
    
};

IRenderingEngine * CreateRender1()
{
    return new RenderingEngine1();
}


RenderingEngine1::RenderingEngine1()
{
    glGenRenderbuffersOES(1, &m_colorRenderBuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_colorRenderBuffer);
}

void RenderingEngine1::Initialize(int width, int height)
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
    glGenRenderbuffersOES(1, &m_depthRenderBuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_depthRenderBuffer);
    glRenderbufferStorageOES(GL_RENDERBUFFER_OES,
                             GL_DEPTH_COMPONENT16_OES,
                             width, 
                             height);
    
    
    glGenFramebuffersOES(1, &m_frameBuffer);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_frameBuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, 
                                 GL_COLOR_ATTACHMENT0_OES,
                                 GL_RENDERBUFFER_OES,
                                 m_colorRenderBuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES,
                                 GL_DEPTH_ATTACHMENT_OES, 
                                 GL_RENDERBUFFER_OES,
                                 m_depthRenderBuffer);
    
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_colorRenderBuffer);
    
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    
    const float maxX = 1.6f; 
    const float maxY = 2.4;
    
    glOrthof(-maxX, +maxX, -maxY, +maxY, 5, 10);
    
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(0, 0, -7);
}

void RenderingEngine1::Render() const
{
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    mat4 rotation(m_animation.Current.ToMatrix());
    glMultMatrixf(rotation.Pointer());
    
    glVertexPointer(3, GL_FLOAT, sizeof(vertex), &m_cone[0].Position.x);
    glColorPointer(4, GL_FLOAT, sizeof(vertex), &m_cone[0].Color.x);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_cone.size());
    
    glVertexPointer(3, GL_FLOAT, sizeof(vertex), &m_disk[0].Position.x);
    glColorPointer(4, GL_FLOAT, sizeof(vertex), &m_disk[0].Color.x);
    glDrawArrays(GL_TRIANGLE_FAN, 0, m_disk.size());
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glPopMatrix();

}

void RenderingEngine1::OnRotate(DeviceOrientation newOrientation)
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


void RenderingEngine1::UpdateAnimation(float timeStep)
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














