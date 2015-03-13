//
//  ImageCapturer.m
//  Enlighten
//
//  Created by Stephen Greco on 3/5/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import "ImageCapturer.h"
using namespace cv;

@implementation ImageCapturer

- (id) initWithCaptureSession:(AVCaptureSession *)session {
    if (self = [super init]) {
        [session addOutput:self];
    }
    return self;
}

- (void) captureOpenCvImageAsynchronouslyWithCompletion:(void (^)(Mat &, NSError*)) block {
    [super captureStillImageAsynchronouslyFromConnection:[self connectionWithMediaType:AVMediaTypeVideo]
                                       completionHandler:^(CMSampleBufferRef imageDataSampleBuffer, NSError *error) {
                                           
                                           // Set a timer for the data->Mat conversion
                                           NSDate *methodStart = [NSDate date];
                                           
                                           NSData *imageData = [AVCaptureStillImageOutput jpegStillImageNSDataRepresentation:imageDataSampleBuffer];
                                           Mat image = imdecode(Mat(1, (int)[imageData length], CV_8UC1, (void*)imageData.bytes), 0);
                                           
                                           NSDate *methodEnd = [NSDate date];
                                           NSTimeInterval executionTime = [methodEnd timeIntervalSinceDate:methodStart];
                                           NSLog(@"captureTime = %.1fms", executionTime * 1000.0);
                                           
                                           return block(image, error);
                                           
    }];
}

@end
