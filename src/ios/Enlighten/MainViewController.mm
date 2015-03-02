//
//  MainViewController.mm
//  Enlighten
//
//  Created by Stephen Greco on 2/12/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import "MainViewController.h"

using namespace cv;

@implementation MainViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.view.backgroundColor = [UIColor whiteColor];
    
    // Set up the capture session to the default settings for 1080p/60fps
    _videoCamera = [[CvVideoCamera alloc] initWithParentView:self.view];
    _videoCamera.defaultAVCaptureDevicePosition = AVCaptureDevicePositionBack;
    _videoCamera.defaultAVCaptureSessionPreset = AVCaptureSessionPreset1920x1080;
    _videoCamera.defaultAVCaptureVideoOrientation = AVCaptureVideoOrientationPortrait;
    _videoCamera.useAVCaptureVideoPreviewLayer = YES;
    _videoCamera.defaultFPS = 60;
    _videoCamera.grayscaleMode = NO;
    _videoCamera.delegate = self;
    
    [_videoCamera start];
    
    if (![_videoCamera captureSessionLoaded]) {
        [[[UIAlertView alloc] initWithTitle:@"No camera"
                                    message:@"This device does not have a back camera."
                                   delegate:nil
                          cancelButtonTitle:@"OK"
                          otherButtonTitles:nil]show];
    }
    
}

#ifdef __cplusplus
- (void)processImage: (Mat&)image {
    // Do some OpenCV stuff with the image
}
#endif

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
