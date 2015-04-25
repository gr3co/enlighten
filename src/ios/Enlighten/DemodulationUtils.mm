//
//  DemodulationUtils.mm
//  Enlighten
//
//  Created by Darrin Willis on 3/30/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import "DemodulationUtils.h"

// This should be the same size as a single image height
int Nfft = 1080;
// This is how far apart our samples are for each sample
int stepsPerFrame = 10;
int stepSize = Nfft / stepsPerFrame;
// This is how many frames could go by before we are sure to see the preamble
int preambleFrames = 30;

@implementation DemodulationUtils

+ (cv::Mat)getHann:(int)n
{
    cv::Mat window = cv::Mat();
    for (int i = 0; i < n; i++)
    {
        double multiplier = 0.5 * (1 - cos(2.0 * M_PI * i / (n - 1.0)));
        window.push_back(multiplier);
    }
    return window;
}

+ (cv::Mat)getFFT:(cv::Mat)imageRows withFreq:(cv::Mat)frequencies
{
    
    //std::cout << imageRows.t() << std::endl;
    
    //NSLog(@"The given image is of size %i by %i", imageRows.rows, imageRows.cols);
    
    // The number of frequencies we are scanning for
    int numFreqs = frequencies.rows;
    
    //int endVal = (numFrames - 1) * imageRows.rows;
    int numWindows = imageRows.rows / stepSize;
    cv::Mat computedFft = cv::Mat(numFreqs, numWindows, CV_64F);
    
    int h = 0;
    for (int i = 0; i < imageRows.rows - Nfft; i+= stepSize) {
        //First get a Mat representing this image
        //NSLog(@"FFTing on window from %i to %i", i, i+Nfft);
        cv::Mat thisImage = imageRows.rowRange(i, i + Nfft);
        
        int pixel_new = i % Nfft;
        int pixel_old = Nfft - pixel_new;
        cv::Mat hann = [DemodulationUtils getHann:pixel_old];
        hann.push_back([DemodulationUtils getHann:pixel_new]);
        
        if (h == 0 || h == 5) {
            //std::cout << hann.t() << std::endl;
        }
        
        
        //NSLog(@"Have hann filter of size %i by %i", hann.size().height, hann.size().width);
        //NSLog(@"Have matrix size %i by %i", thisImage.size().height, thisImage.size().width);
        
        thisImage = thisImage.mul(hann);
        
        // This is copied from OpenCV documentation on how to use DFT
        cv::Mat padded;                            //expand input image to optimal size
        int m = cv::getOptimalDFTSize( thisImage.rows );
        int n = cv::getOptimalDFTSize( thisImage.cols ); // on the border add zero values
        cv::copyMakeBorder(thisImage, padded, 0, m - thisImage.rows, 0, n - thisImage.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
        
        cv::Mat planes[] = {cv::Mat_<double>(padded), cv::Mat::zeros(padded.size(), CV_64F)};
        cv::Mat complexI;
        cv::merge(planes, 2, complexI);
        
        cv::dft(complexI, complexI);

        cv::split(complexI, planes);
        cv::magnitude(planes[0], planes[1], planes[0]);
        cv::Mat thisFft = planes[0];
        
        cv::normalize(thisFft, thisFft, 0, 255, cv::NORM_MINMAX);
        
        if (h == 155 || h == 255) {
            //std::cout << thisFft << std::endl;
        }
        
        //NSLog(@"Size of thisFFt = %i x %i", thisFft.rows, thisFft.cols);
        
        // Iterate through the target frequencies
        // The output has columns of frequencies, and rows over time
        for (int j = 0; j < numFreqs; j++) {
            double realFreq = frequencies.at<double>(j, 0);
            int transFreq = floor(realFreq / 60.0);
            cv::Mat releventFreqs = thisFft.rowRange(transFreq - 3, transFreq + 3);
            cv::Mat squared = releventFreqs.mul(releventFreqs);
            double val = sqrt(sum(squared)[0] / 6);
            computedFft.at<double>(j,h) = val;
        }
        h++;
    }
    //std::cout << frequencies.t() << std::endl;
    //std::cout << computedFft.rows << " " << computedFft.cols << std::endl;
    //std::cout << computedFft << std::endl;
    
    return computedFft;
}

//fftOverTime should be a 2xN matrix where the first row is the preamble
//fft over time and the second row is the data frequency fft over time
+ (cv::Mat) getData:(cv::Mat)fftOverTime
            preRate:(double)preRate
            dataRate:(double)dataRate
            dataBits:(double)dataBits
{
    //First we want to find a preamble peak, it should be the 
    cv::Mat preambleFft = fftOverTime.row(0);
    cv::Mat dataFft = fftOverTime.row(1);
    int numberSamples = dataFft.cols;
    //NSLog(@"Number of samples is %i", numberSamples);
    //NSLog(@"Preamble search length is %i", preambleFrames * stepsPerFrame + 1);
    cv::Mat preambleContainer =  preambleFft.colRange(0, preambleFrames * stepsPerFrame+1);

    cv::Point preamblePoint;
    cv::minMaxLoc(preambleContainer, NULL, NULL, NULL, &preamblePoint);

    int preambleIdx = preamblePoint.x;
    
    //std::cout << "preamble is located at " << preambleIdx << std::endl;

    int jumpPreamble = round((preRate + dataRate) / 2) * stepsPerFrame;
    int jumpData = round(dataRate * stepsPerFrame);

    //NSLog(@"JumpPreamble is %i, jumpData is %i", jumpPreamble, jumpData);
    
    int idxOn = preambleIdx + jumpPreamble;

    double offVal = dataFft.at<double>(0,preambleIdx);
    double onVal = dataFft.at<double>(0, idxOn);
    
    // This is the threshold to determine whether or not data is 0 or 1
    double threshold = (onVal + offVal) / 2;

    //NSLog(@"Threshold is %f", threshold);
    
    // We take the start of the transfer + sending the pilot on and the data
    int byteLength = jumpPreamble + dataBits * jumpData;

    // Demod data should be an array of single bytes which are 0 or 1
    cv::Mat demodData = cv::Mat(1, dataBits, CV_8S);

    if (preambleIdx + byteLength > numberSamples) {
        // this should handle the error gracefully, this should never happen
        // because if the preamble is detected in the back half of the image,
        // then it didn't actually find a viable preamble.
        return cv::Mat::ones(1, dataBits, CV_8U);
    } else {
        for (int i = 1; i <= dataBits; i++) {
            int bitIdx = preambleIdx + jumpPreamble + i * jumpData;
            double signalVal = dataFft.at<double>(0, bitIdx);
            BOOL demodVal = signalVal > threshold;
            //NSLog(@"Bit %i is detected at %i: %f (%i)", i, bitIdx, signalVal, demodVal);
            demodData.at<BOOL>(0,i-1) = demodVal;
        }
    }
    return demodData;
}

@end
