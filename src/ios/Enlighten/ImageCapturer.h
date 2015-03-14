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
- (void) imageCapturerDidCaptureFrames:(std::vector<cv::Mat>&) frames;

@end


@interface ImageCapturer : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

- (id) initWithCaptureSession:(AVCaptureSession*) session;

- (void) captureFrames;

@property AVCaptureSession *session;
@property AVCaptureVideoDataOutput *output;
@property std::vector<cv::Mat> currentFrames;

@property id<ImageCapturerDelegate> delegate;

@end
