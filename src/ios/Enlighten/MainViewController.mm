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


#define DATA1 3400.0
#define DATA2 4200.0

#define PREAMBLE1 1800.0
#define PREAMBLE2 2600.0

#define TWOVALUES

using namespace cv;

@implementation MainViewController {
    NSDate *startTime;
    BOOL isCapturing;
    int frameSize;
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
            
            if (maxrate == 60
                && CMFormatDescriptionGetMediaSubType(description) == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange
                && format.highResolutionStillImageDimensions.height >= 480){
                if ([backCamera lockForConfiguration:NULL]) {
                    
                    backCamera.activeFormat = format;
                    
                    // Either going to be 480 or 1080, depending on my phone or Darrin's phone
                    frameSize = min(format.highResolutionStillImageDimensions.height, 1080);
                    
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
    _resultLabel1 = [[UILabel alloc] initWithFrame:
                    CGRectMake(0.15 * width,
                               0.5 * height - 100,
                               0.7*width, 100)];
    _resultLabel1.backgroundColor = [UIColor clearColor];
    _resultLabel1.textColor = [UIColor redColor];
    _resultLabel1.text = @"--";
    _resultLabel1.textAlignment = NSTextAlignmentCenter;
    _resultLabel1.font = [UIFont fontWithName:@"Menlo-Bold" size:64];
    [self.view addSubview:_resultLabel1];
#ifdef TWOVALUES
    _resultLabel2 = [[UILabel alloc] initWithFrame:
                     CGRectMake(0.15 * width,
                                0.5 * height,
                                0.7*width, 100)];
    _resultLabel2.backgroundColor = [UIColor clearColor];
    _resultLabel2.textColor = [UIColor redColor];
    _resultLabel2.text = @"--";
    _resultLabel2.textAlignment = NSTextAlignmentCenter;
    _resultLabel2.font = [UIFont fontWithName:@"Menlo-Bold" size:64];
    [self.view addSubview:_resultLabel2];
#endif
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
        
        Mat freq = Mat();
        freq.push_back(PREAMBLE1);
        freq.push_back(DATA1);
#ifdef TWOVALUES
        freq.push_back(PREAMBLE2);
        freq.push_back(DATA2);
#endif
        Mat fftData = [DemodulationUtils getFFT:avg
                                      withFreq:freq
                                  andFrameSize:frameSize];
        
        Mat result1 = [DemodulationUtils getData:fftData.rowRange(0, 2)
                                           preRate:1.5
                                          dataRate:1.5
                                          dataBits:16];
        uint16_t demod1 = convertToInt(result1);
        BOOL error1 = demod1 == 0 || (0xff != (((demod1 >> 8) & 0xff) ^ (demod1 & 0xff)));
        
#ifdef TWOVALUES
        Mat result2 = [DemodulationUtils getData:fftData.rowRange(2, 4)
                                        preRate:1.5
                                       dataRate:1.5
                                       dataBits:16];
        uint16_t demod2 = convertToInt(result2);
        BOOL error2 = demod2 == 0 || (0xff != (((demod2 >> 8) & 0xff) ^ (demod2 & 0xff)));
#endif
        
        dispatch_async(dispatch_get_main_queue(), ^{
            _resultLabel1.textColor = error1
                                ? [UIColor redColor]
                                : [UIColor blueColor];
            _resultLabel1.text = error1
                                ? @"--"
                                : [NSString stringWithFormat:@"%02x", demod1 & 0xff];
#ifdef TWOVALUES
            _resultLabel2.textColor = error2
                                ? [UIColor redColor]
                                : [UIColor greenColor];
            _resultLabel2.text = error2
                                ? @"--"
                                : [NSString stringWithFormat:@"%02x", demod2 & 0xff];
#endif
            std::cout << result1 << std::endl;
            std::cout << result2 << std::endl;
            //std::cout << fftData << std::endl;
        });
    });
    
}

uint16_t convertToInt(Mat& data) {
    uint16_t result = 0;
    for (int i = 0; i < data.cols; i++) {
        result |= (data.at<BOOL>(0,i) << i);
    }
    return result;
}


@end
