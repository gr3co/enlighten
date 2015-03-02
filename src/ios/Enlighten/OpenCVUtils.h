//
//  OpenCVUtils.h
//  Enlighten
//
//  Created by Stephen Greco on 3/2/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#ifdef __cplusplus
#import <opencv2/opencv.hpp>
#endif

@interface OpenCVUtils : NSObject

+ (cv::Mat)cvMatFromUIImage:(UIImage *)image;
+ (UIImage *)UIImageFromCvMat:(cv::Mat)cvMat;

@end
