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
#import "ResultView.h"

@interface MainViewController : UIViewController <ImageCapturerDelegate,
UIScrollViewDelegate, ResultViewDelegate>

@property (nonatomic, retain) AVCaptureSession *captureSession;

@property ImageCapturer *capturer;
@property UILabel *titleLabel;
@property UILabel *instructionsLabel;

@end

