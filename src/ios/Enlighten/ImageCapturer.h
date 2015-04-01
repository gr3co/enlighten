//
//  ImageCapturer.h
//  Enlighten
//
//  Created by Stephen Greco on 3/5/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>

#ifdef __cplusplus
#import <opencv2/opencv.hpp>
#include <vector>
#endif

@protocol ImageCapturerDelegate <NSObject>

@required
- (void) imageCapturerDidProcessPreviewFrame:(cv::Mat&) frame;
- (void) imageCapturerDidCaptureFrames:(std::vector<cv::Mat>&) frames;

@end


@interface ImageCapturer : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

// We could set up the session here, but it felt easier
// just to pass it along in case we want to do something
// with it in the other classes
- (id) initWithCaptureSession:(AVCaptureSession*) session;

// Self-explanatory
- (void) captureFrames;

// NSObjects don't have this function by default so we have
// to manage our memory ourselves
- (void)didReceiveMemoryWarning;

// The capture session that was set up by the viewController
@property AVCaptureSession *session;

// The output is what sends us the pixel data
@property AVCaptureVideoDataOutput *output;

// This is just a list of the frames that we're currently capturing
@property std::vector<cv::Mat> currentFrames;

// Self-explanatory
@property id<ImageCapturerDelegate> delegate;

@property BOOL isRecording;

@end
