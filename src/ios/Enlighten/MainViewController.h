//
//  MainViewController.h
//  Enlighten
//
//  Created by Stephen Greco on 2/12/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import "ImageCapturer.h"

@interface MainViewController : UIViewController <ImageCapturerDelegate, UIScrollViewDelegate>

@property (nonatomic, retain) AVCaptureSession *captureSession;

@property UIButton *button;
@property ImageCapturer *capturer;
@property UIImageView *imageView;
@property UILabel *resultLabel1;
@property UILabel *resultLabel2;

@end

