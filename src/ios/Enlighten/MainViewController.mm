//
//  MainViewController.mm
//  Enlighten
//
//  Created by Stephen Greco on 2/12/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import "MainViewController.h"
#import "OpenCVUtils.h"
#import "ImageCapturer.h"

using namespace cv;

@implementation MainViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // The same color as the launch image background
    self.view.backgroundColor = [UIColor colorWithRed:0xf7/255.0
                                                green:0xf4/255.0
                                                 blue:7/255.0
                                                alpha:1.0];
    
    _button = [[UIButton alloc] initWithFrame:CGRectMake(10, 50, 300, 50)];
    [_button setTitle:@"Capture Image" forState:UIControlStateNormal];
    [_button setTitle: @"Capture Unavailable" forState:UIControlStateDisabled];
    
    // Whenever the button is pressed, we want to call the captureImage method defined below
    [_button addTarget:self action:@selector(captureImage) forControlEvents:UIControlEventTouchUpInside];
    [_button setTitleColor: [UIColor blackColor] forState:UIControlStateNormal];
    [self.view addSubview:_button];
    
    
    // Set up the capture session to the default settings for 720p
    _captureSession = [[AVCaptureSession alloc] init];
    _captureSession.sessionPreset = AVCaptureSessionPreset1280x720;
    
    // Find the back camera (there should be an easier way to do this...)
    NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    AVCaptureDevice *backCamera = nil;
    for (AVCaptureDevice *device in devices) {
        if(device.position == AVCaptureDevicePositionBack) {
            backCamera = device;
        }
    }
    
    // If we can't find a back camera, then there isn't a camera.
    if (backCamera == nil) {
        [[[UIAlertView alloc] initWithTitle:@"No camera"
                                    message:@"This device does not have a back camera."
                                   delegate:nil
                          cancelButtonTitle:@"OK"
                          otherButtonTitles:nil]show];
        [_button setEnabled: NO];
    } else {
        
        // Tell the capture session that we want to use the back camera
        NSError *error;
        AVCaptureDeviceInput *backCameraInput = [[AVCaptureDeviceInput alloc] initWithDevice:backCamera error:&error];
        [_captureSession addInput:backCameraInput];
        
        if (error != nil) {
            NSLog(@"%@", [error localizedDescription]);
        }
        
        // Start running the capture session (this means that it is ready to capture images/video)
        [_captureSession startRunning];
        
        // Create an instance of ImageCapturer to, well, capture our images
        _capturer = [[ImageCapturer alloc] initWithCaptureSession:_captureSession];
        
    }
    
    _imageView = [[UIImageView alloc] initWithFrame:CGRectMake(10, 100, 300, 300)];
    [self.view addSubview:_imageView];
    

}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)captureImage {
    [_capturer captureOpenCvImageAsynchronouslyWithCompletion:^(cv::Mat& cvImage, NSError *error) {
        if (error != nil) {
            NSLog(@"%@", [error localizedDescription]);
        } else {
            // Note- images are rotated 90deg when converted directly from a Mat to a UIImage
            [_imageView setImage:[OpenCVUtils UIImageFromCvMat:cvImage]];
        }
    }];
}

@end
