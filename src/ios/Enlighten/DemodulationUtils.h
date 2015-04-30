//
//  DemodulationUtils.h
//  Enlighten
//
//  Created by Darrin Willis on 3/30/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#ifdef __cplusplus
#import <opencv2/opencv.hpp>
#endif

@interface DemodulationUtils : NSObject

+ (cv::Mat)getFFT:(cv::Mat)imageRows
withFreq:(cv::Mat)frequencies
andFrameSize:(int)frameSize;

+ (cv::Mat) getData: (cv::Mat)fftOverTime
preRate: (double)preRate
dataRate: (double)dataRate
dataBits: (double)dataBits;

@end
