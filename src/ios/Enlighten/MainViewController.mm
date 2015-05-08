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

#define PREAMBLE_FRAMES 1.0
#define DATA_FRAMES 1.5
#define DATA_BITS 16

#define BULB1
#define BULB2

using namespace cv;

@implementation MainViewController {
    NSDate *startTime;
    BOOL isCapturing;
    int frameSize;
    MBProgressHUD *hud;
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
    _titleLabel = [[UILabel alloc] initWithFrame:
                    CGRectMake(0.15 * width,
                               150,
                               0.7*width, 100)];
    _titleLabel.backgroundColor = [UIColor clearColor];
    _titleLabel.textColor = [UIColor blackColor];
    _titleLabel.text = @"Welcome to Enlighten\u2122";
    _titleLabel.textAlignment = NSTextAlignmentCenter;
    _titleLabel.numberOfLines = 0;
    _titleLabel.font = [UIFont fontWithName:@"Menlo-Bold" size:40];
    [self.view addSubview:_titleLabel];
    
    _instructionsLabel = [[UILabel alloc] initWithFrame:
                          CGRectMake(0.15 * width,
                                     height - 350,
                                     0.7*width, 300)];
    _instructionsLabel.backgroundColor = [UIColor clearColor];
    _instructionsLabel.textColor = [UIColor redColor];
    _instructionsLabel.text = @"Please point your camera at an Enlightened\u2122 work of art"
                                " and we'll take it from here!";
    _instructionsLabel.numberOfLines = 0;
    _instructionsLabel.textAlignment = NSTextAlignmentCenter;
    _instructionsLabel.font = [UIFont fontWithName:@"Menlo-Bold" size:24];
    [self.view addSubview:_instructionsLabel];
    
    
    [_captureSession startRunning];
    hud = [MBProgressHUD showHUDAddedTo:self.view animated:YES];
    hud.labelText = @"scanning..";

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
#ifdef BULB1
        freq.push_back(PREAMBLE1);
        freq.push_back(DATA1);
#endif
#ifdef BULB2
        freq.push_back(PREAMBLE2);
        freq.push_back(DATA2);
#endif
        Mat fftData = [DemodulationUtils getFFT:avg
                                      withFreq:freq
                                  andFrameSize:frameSize];
#ifdef BULB1
        Mat result1 = [DemodulationUtils getData:fftData.rowRange(0, 2)
                                           preRate:PREAMBLE_FRAMES
                                          dataRate:DATA_FRAMES
                                          dataBits:DATA_BITS];
        uint16_t demod1 = convertToInt(result1);
        BOOL error1 = demod1 == 0 || (0xff != (((demod1 >> 8) & 0xff) ^ (demod1 & 0xff)));
        demod1 &= 0xff;
#endif
#ifdef BULB2
        Mat result2 = [DemodulationUtils getData:fftData.rowRange(2, 4)
                                        preRate:PREAMBLE_FRAMES
                                       dataRate:DATA_FRAMES
                                       dataBits:DATA_BITS];
        uint16_t demod2 = convertToInt(result2);
        BOOL error2 = demod2 == 0 || (0xff != (((demod2 >> 8) & 0xff) ^ (demod2 & 0xff)));
        demod2 &= 0xff;
#endif
        
        dispatch_async(dispatch_get_main_queue(), ^{
            int finalResult = (!error1) ? demod1 : (!error2) ? demod2 : 0;
            [self processResult:finalResult];
        });
    });
    
}

- (void) processResult:(int)result {
    if (result != 0) {
        [_captureSession stopRunning];
        hud.labelText = @"loading..";
        ResultView *resultView = [[ResultView alloc] initWithFrame:self.view.frame
                                                            result:result
                                                       andDelegate:self];
        [self.view addSubview:resultView];
    }
}

- (void) resultViewDidClose:(UIView *)view {
    NSLog(@"closed");
    [_capturer reset];
    [_captureSession startRunning];
    hud = [MBProgressHUD showHUDAddedTo:self.view animated:YES];
    hud.labelText = @"scanning..";
}

uint16_t convertToInt(Mat& data) {
    uint16_t result = 0;
    for (int i = 0; i < data.cols; i++) {
        result |= (data.at<BOOL>(0,i) << i);
    }
    return result;
}


@end
