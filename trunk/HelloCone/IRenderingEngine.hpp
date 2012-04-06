//
//  IRenderingEngine.hpp
//  HelloOpengl
//
//  Created by was cienet on 05/04/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#define STRINGIFY(A)    #A


enum DeviceOrientation 
{
    DeviceOrientationUnknown,
    DeviceOrientationPortrait,
    DeviceOrientationPortraitUpsideDown,
    DeviceOrientationLandscapeLeft,
    DeviceOrientationLandscapeRight,
    DeviceOrientationFaceUp,
    DeviceOrientationfaceDown,
};

struct IRenderingEngine *CreateRender1();
struct IRenderingEngine *CreateRender2();

struct IRenderingEngine
{
    virtual void Initialize(int width, int height) = 0 ;
    
    virtual void Render() const = 0; 
    
    virtual void UpdateAnimation(float timeStep) = 0 ; 
    
    virtual void OnRotate(DeviceOrientation newOrientation) = 0 ; 
    
    virtual ~IRenderingEngine(){}
    
};