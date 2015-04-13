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
    
    CGSize frameSize = self.view.frame.size;
    
    _button = [[UIButton alloc] initWithFrame:CGRectMake(frameSize.width / 2 - 100,
                                                         frameSize.height - 100, 200, 50)];
    [_button setBackgroundColor:[UIColor whiteColor]];
    [_button setTitle:@"Capture Frames" forState:UIControlStateNormal];
    [_button setTitle: @"Capture Unavailable" forState:UIControlStateDisabled];
    
    // Whenever the button is pressed, we want to call the captureImage method defined below
    [_button addTarget:self action:@selector(captureImages) forControlEvents:UIControlEventTouchUpInside];
    [_button setTitleColor: [UIColor blackColor] forState:UIControlStateNormal];
    
    
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
                    [backCamera setExposureModeCustomWithDuration:CMTimeMake(1,64800)
                                                              ISO:backCamera.activeFormat.maxISO
                                                completionHandler:nil];
                    
                    [backCamera unlockForConfiguration];
                    
                    NSLog(@"min %f max %f", backCamera.activeFormat.minISO, backCamera.activeFormat.maxISO);
                    
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
        
    _imageView = [[UIImageView alloc] initWithFrame:self.view.frame];
    _imageView.contentMode = UIViewContentModeScaleAspectFit;
    _imageView.userInteractionEnabled = YES;
    [_imageView addSubview:_button];
    [self.view addSubview:_imageView];

}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void)captureImages {
    
    if (isCapturing) {
        return;
    }
    
    if (![_captureSession isRunning]) {
        [_captureSession startRunning];
        return;
    }
    
    isCapturing = YES;
    startTime = [NSDate date];
    [_capturer captureFrames];
    // This is just that loading icon, so we have some sort of visual feedback
    [MBProgressHUD showHUDAddedTo:self.view animated:YES];
}

- (void)imageCapturerDidCaptureFrames:(std::vector<Mat>&)frames {
    
    [_captureSession stopRunning];
    
    
    /* 
     * The lines below this simply append the matrices from the vector
     * together so that we can run some sort of signal processing on them.
     *
     * The reason I'm doing combining the frames here and not as we capture
     * them is that the frames come in asynchronously and we want to make
     * sure they're in the correct order as much as we can.
     *
     * I'm not sure if the amount of time it takes to append the matrices
     * together is longer than the processing time of a single frame, so we
     * should wait until we have all of them in what we assume is the correct
     * order before we combine them in order to run signal processing.
     */
    Mat total = Mat();
    Mat first = frames[0];
    NSLog(@"One frame size = %i x %i", first.rows, first.cols);
    for (int i = 0; i < frames.size(); i++) {
        total.push_back(frames[i]);
    }
    
    // this can be changed to literally any iterable datatype
    Mat avg = Mat();
    
    // Take the average
    for (int i = 0; i < total.size().height; i++) {
        avg.push_back((double)mean(total.row(i))[0]);
    }
    
    NSLog(@"Total size is %i x %i", total.rows, total.cols);
    NSLog(@"Average vector length: %i", avg.rows);
    
    // Fuck yeah memory management
    frames.clear();
    ~total;
    
    // The time we are displaying below is from when we press the button
    // to when the final matrix is created and ready for processing.
    if (startTime != nil) {
        NSDate *endTime = [NSDate date];
        NSTimeInterval elapsed = [endTime timeIntervalSinceDate:startTime];
        NSLog(@"capture took %.2fms", elapsed * 1000.0);
        startTime = nil;
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        // Get rid of the loading icon when the image is displayed
        [MBProgressHUD hideHUDForView:self.view animated:YES];
        isCapturing = NO;
        
        // DO SOMETHING WITH THE AVERAGE VALUE ARRAY
        Mat freq = Mat();
        freq.push_back((double)150.0);
        Mat result = [DemodulationUtils getFFT:avg withFreq:freq];
        _fft = [[NSMutableArray alloc] init];
        for (int i = 0; i < result.rows; i++) {
            [_fft addObject:@((double)result.at<double>(i, 0))];
        }
        
        GKLineGraph *graphView = [[GKLineGraph alloc] initWithFrame:
                                  CGRectMake(0, 200, self.view.frame.size.width, 200)];
        
        graphView.dataSource = self;
        graphView.lineWidth = 3.0;
        
        [graphView draw];
        
        [_imageView addSubview:graphView];
    });
    
}

- (void) imageCapturerDidProcessPreviewFrame:(Mat &)frame {
    transpose(frame, frame);
    flip(frame, frame, 1);
    UIImage *image = [OpenCVUtils UIImageFromCvMat:frame];
    ~frame;
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.imageView setImage:image];
    });
}

- (NSInteger)numberOfLines {
    return 1;
}

- (UIColor *)colorForLineAtIndex:(NSInteger)index {
    return [UIColor blueColor];
}

- (NSArray *)valuesForLineAtIndex:(NSInteger)index {
    return _fft;
}

- (CFTimeInterval)animationDurationForLineAtIndex:(NSInteger)index {
    return 0;
}

- (NSString *)titleForLineAtIndex:(NSInteger)index {
    return @"FFT";
}

@end
