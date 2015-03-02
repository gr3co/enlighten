//
//  MainViewController.h
//  Enlighten
//
//  Created by Stephen Greco on 2/12/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <opencv2/highgui/cap_ios.h>

@interface MainViewController : UIViewController <CvVideoCameraDelegate>

@property (nonatomic, retain) CvVideoCamera *videoCamera;

@end

