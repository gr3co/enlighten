//
//  MainViewController.mm
//  Enlighten
//
//  Created by Stephen Greco on 2/12/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import <MBProgressHUD.h>
#import "MainViewController.h"
#import "OpenCVUtils.h"
#import "ImageCapturer.h"
#import "DemodulationUtils.h"

using namespace cv;

@implementation MainViewController {
    NSDate *startTime;
    BOOL isCapturing;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // The same color as the launch image background
    self.view.backgroundColor = [UIColor colorWithRed:0xf7/255.0
                                                green:0xf4/255.0
                                                 blue:7/255.0
                                                alpha:1.0];
    
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
        
        // Reference: http://stackoverflow.com/questions/20330174/avcapture-capturing-and-getting-framebuffer-at-60-fps-in-ios-7
        for (AVCaptureDeviceFormat *format in [backCamera formats] ) {
            
            CMFormatDescriptionRef description = format.formatDescription;
            float maxrate = ((AVFrameRateRange*)[format.videoSupportedFrameRateRanges objectAtIndex:0]).maxFrameRate;
            
            if (maxrate == 60 &&
                CMFormatDescriptionGetMediaSubType(description) == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange){
                if ([backCamera lockForConfiguration:NULL]) {
                    backCamera.activeFormat = format;
                    
                    // Set the camera to always use 60fps if available
                    [backCamera setActiveVideoMinFrameDuration:CMTimeMake(1,60)];
                    [backCamera setActiveVideoMaxFrameDuration:CMTimeMake(1,60)];
                    
                    // Always turn off torch mode & flash mode
                    [backCamera setTorchMode:AVCaptureTorchModeOff];
                    [backCamera setFlashMode:AVCaptureFlashModeOff];
                    
                    // Adjust exposure to be as small as possible and ISO as big as possible
                    // Current format, iso 29-464 ; other format, iso 29-968
                    [backCamera setExposureModeCustomWithDuration:backCamera.activeFormat.minExposureDuration
                                                              ISO:backCamera.activeFormat.maxISO
                                                completionHandler:nil];
                    
                    [backCamera unlockForConfiguration];
                                        
                }
            }
        }
        
        
        if (error != nil) {
            NSLog(@"%@", [error localizedDescription]);
        }
        
        // Create an instance of ImageCapturer to, well, capture our images
        _capturer = [[ImageCapturer alloc] initWithCaptureSession:_captureSession];
        _capturer.delegate = self;
        
        [_captureSession startRunning];
        
    }
    
    
    float width = self.view.frame.size.width;
    float height = self.view.frame.size.height;
    _resultLabel = [[UILabel alloc] initWithFrame:
                    CGRectMake(0.15 * width,
                               0.5 * height - 50,
                               0.7*width, 100)];
    _resultLabel.backgroundColor = [UIColor clearColor];
    _resultLabel.textColor = [UIColor blueColor];
    _resultLabel.text = @"----";
    _resultLabel.textAlignment = NSTextAlignmentCenter;
    _resultLabel.font = [UIFont fontWithName:@"Menlo-Bold" size:64];
    [self.view addSubview:_resultLabel];
    
    [_captureSession startRunning];

}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    [_capturer didReceiveMemoryWarning];
}

- (void)imageCapturerDidCaptureFrames:(std::vector<Mat>*)frames {
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        // this can be changed to literally any iterable datatype
        Mat avg = Mat();
        
        // Take the average
        for (int i = 0; i < frames->size(); i++) {
            for (int j = 0; j < (*frames)[i].size().height; j++) {
                avg.push_back((double)mean((*frames)[i].row(j))[0]);
            }
        }
    
        // Fuck yeah memory management
        frames->clear();
        delete frames;
        
        // DO SOMETHING WITH THE AVERAGE VALUE ARRAY
        Mat freq = Mat();
        freq.push_back(3300.0); // PREAMBLE
        freq.push_back(1500.0); // DATA
        Mat result = [DemodulationUtils getFFT:avg withFreq:freq];
        Mat demodData = [DemodulationUtils getData:result
                                           preRate:1.5
                                          dataRate:1.5
                                          dataBits:16];
        
        uint32_t demod = convertToInt(demodData);
        
        dispatch_async(dispatch_get_main_queue(), ^{
            _resultLabel.textColor = (demod == 0)
                                ? [UIColor redColor]
                                : [UIColor blueColor];
            _resultLabel.text = (demod == 0)
                                ? @"ERROR"
                                : [NSString stringWithFormat:@"%04x", demod];
            std::cout << demodData << std::endl;
            std::cout << result << std::endl;
        });
    });
    
}

uint32_t convertToInt(Mat& data) {
    uint32_t result = 0;
    for (int i = 0; i < data.cols; i++) {
        result |= (data.at<BOOL>(0,i) << i);
    }
    return result;
}


@end
