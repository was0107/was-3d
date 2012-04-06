//
//  GLView.h
//  HelloOpengl
//
//  Created by was cienet on 05/04/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import "IRenderingEngine.hpp"

@interface GLView : UIView {
    EAGLContext *m_context;
    IRenderingEngine *m_renderingEngine;
    float m_timestamp;
}
-(void) drawView:(CADisplayLink *)displayLink;
-(void) didRotate:(NSNotification *) notification;
@end
