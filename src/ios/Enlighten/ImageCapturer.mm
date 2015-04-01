//
//  ImageCapturer.m
//  Enlighten
//
//  Created by Stephen Greco on 3/5/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import "ImageCapturer.h"

// The number of frames we want to capture per session
#define FRAME_COUNT 60

using namespace cv;

@implementation ImageCapturer

- (id) initWithCaptureSession:(AVCaptureSession *)session {
    
    if (self = [super init]) {
        _output = [[AVCaptureVideoDataOutput alloc] init];
        
        dispatch_queue_t videoDataOutputQueue = dispatch_queue_create("VideoDataOutputQueue", DISPATCH_QUEUE_SERIAL);
        [_output setSampleBufferDelegate:self queue:videoDataOutputQueue];
        
        [session addOutput:_output];
        _session = session;
        
    }
    
    return self;
}


// This delegate function is called every time a frame is received from the Capture Session
- (void) captureOutput:(AVCaptureOutput *)captureOutput
 didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
        fromConnection:(AVCaptureConnection *)connection {
    
    // Below we're just generating an OpenCV Matrix from the raw
    // pixel data given to us by the sampleBuffer
    CVImageBufferRef imgBuf = CMSampleBufferGetImageBuffer(sampleBuffer);
    
    // lock the buffer
    CVPixelBufferLockBaseAddress(imgBuf, 0);
    
    // get the address to the image data
    void *imgBufAddr = CVPixelBufferGetBaseAddressOfPlane(imgBuf, 0);
    
    // get image properties
    int w = (int)CVPixelBufferGetWidth(imgBuf);
    int h = (int)CVPixelBufferGetHeight(imgBuf);
    
    // create the cv mat
    Mat image = Mat(h, w, CV_8UC1);
    memcpy(image.data, imgBufAddr, w * h);
    
    // unlock again
    CVPixelBufferUnlockBaseAddress(imgBuf, 0);
    
    if (_isRecording) {
        // Once we've collected 60 frames, stop recording and send the
        // frames to the delegate in order to process them
        if (_currentFrames.size() >= FRAME_COUNT) {
            NSLog(@"stopping capture");
            _isRecording = NO;
            [_delegate imageCapturerDidCaptureFrames:_currentFrames];
            return;
        }
        
        // Add the matrix to the current list of matrices
        _currentFrames.push_back(image);
        
        // logging is always fun
        if (_currentFrames.size() % 5 == 0) {
            NSLog(@"captured %d frames", (int)_currentFrames.size());
        }
    } else {
        [_delegate imageCapturerDidProcessPreviewFrame:image];
    }
    
}

// Start capturing frames then send to delegate
- (void) captureFrames {
    
    if (_isRecording) {
        @throw @"Session already running.";
    }
    
    NSLog(@"starting capture");
    _currentFrames = std::vector<Mat>();
    _isRecording = YES;
    
}

- (void)didReceiveMemoryWarning {
    // Reset current frames so maybe it'll get garbage collected
    _currentFrames.clear();
}

@end
