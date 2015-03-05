//
//  ImageCapturer.m
//  Enlighten
//
//  Created by Stephen Greco on 3/5/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import "ImageCapturer.h"

@implementation ImageCapturer

- (id) initWithCaptureSession:(AVCaptureSession *)session {
    if (self = [super init]) {
        [session addOutput:self];
    }
    return self;
}

- (void) captureOpenCvImageAsynchronouslyWithCompletion:(void (^)(cv::Mat &, NSError*)) block {
    [super captureStillImageAsynchronouslyFromConnection:[self connectionWithMediaType:AVMediaTypeVideo]
                                       completionHandler:^(CMSampleBufferRef imageDataSampleBuffer, NSError *error) {
                                           
                                           
                                           // FYI, some of this code came directly from
                                           // http://stackoverflow.com/questions/12355257/open-cv-ios-video-processing
                                           
                                           CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(imageDataSampleBuffer);
                                           
                                           CVPixelBufferLockBaseAddress( pixelBuffer, 0 );
                                           
                                           int bufferWidth = (int)CVPixelBufferGetWidth(pixelBuffer);
                                           int bufferHeight = (int)CVPixelBufferGetHeight(pixelBuffer);
                                           
                                           unsigned char *pixel = (unsigned char *)CVPixelBufferGetBaseAddress(pixelBuffer);
                                           
                                           // Just point the original buffer to the OpenCV Mat
                                           cv::Mat temp = cv::Mat(bufferHeight, bufferWidth, CV_8UC4, pixel);
                                           // But we don't own that, so we should make our own copy of the Mat
                                           cv::Mat image = temp.clone();
                                           CVPixelBufferUnlockBaseAddress( pixelBuffer, 0 );
                                           
                                           return block(image, error);
                                           
    }];
}

@end
