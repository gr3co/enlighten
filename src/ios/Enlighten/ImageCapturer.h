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
#endif

@interface ImageCapturer : AVCaptureStillImageOutput

- (id) initWithCaptureSession:(AVCaptureSession*) session;

- (void) captureOpenCvImageAsynchronouslyWithCompletion:(void (^)(cv::Mat &, NSError*)) block;

@end
