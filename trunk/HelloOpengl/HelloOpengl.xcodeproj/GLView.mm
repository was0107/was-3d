//
//  GLView.m
//  HelloOpengl
//
//  Created by was cienet on 05/04/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import "GLView.h"

#define GL_RENDERBUFFER 0x8d41
const bool ForceES1 = false;


@implementation GLView


- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
        CAEAGLLayer *layer = (CAEAGLLayer *)super.layer;
        layer.opaque = YES;
        
        EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
        m_context = [[EAGLContext alloc] initWithAPI:api];
        
        if (!m_context || ForceES1) 
        {
            api = kEAGLRenderingAPIOpenGLES1;
            m_context = [[EAGLContext alloc] initWithAPI:api];
        }        
        if (!m_context || ![EAGLContext setCurrentContext:m_context]) 
        {
            [self release];
            return  nil;
        }
        if (kEAGLRenderingAPIOpenGLES1 == api)
        {
            m_renderingEngine = CreateRender1();
        }
        else 
        {
            m_renderingEngine = CreateRender2();
        }
        
        [m_context renderbufferStorage:GL_RENDERBUFFER 
                          fromDrawable:layer];
        
        m_renderingEngine->Initialize(CGRectGetWidth(frame), CGRectGetHeight(frame));
        
        [self drawView:nil];
        
        m_timestamp = CACurrentMediaTime();
        
        CADisplayLink *displayLink = [CADisplayLink displayLinkWithTarget:self
                                                                 selector:@selector(drawView:)];
        
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop]
                          forMode:NSDefaultRunLoopMode];
        
        
        [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
        
        [[NSNotificationCenter defaultCenter] addObserver:self 
                                                 selector:@selector(didRotate:) 
                                                     name:UIDeviceOrientationDidChangeNotification
                                                   object:nil];
        
        
    }
    return self;
}

+(Class) layerClass
{
    return [CAEAGLLayer class];
}

- (void)dealloc
{
    if ([EAGLContext currentContext] == m_context) {
        [EAGLContext setCurrentContext:nil];
    }
    [m_context release];
    [super dealloc];
}

- (void) drawView:(CADisplayLink *)displayLink
{
    if (displayLink!= nil)
    {
        float elapseSeconds = displayLink.timestamp - m_timestamp;
        m_timestamp = displayLink.timestamp;
        m_renderingEngine->UpdateAnimation(elapseSeconds);
    }
    
    m_renderingEngine->Render();
    [m_context presentRenderbuffer:GL_RENDERBUFFER];
    
}

- (void) didRotate:(NSNotification *)notification
{
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    m_renderingEngine->OnRotate((DeviceOrientation)orientation);
    [self drawView:nil];
}

@end
