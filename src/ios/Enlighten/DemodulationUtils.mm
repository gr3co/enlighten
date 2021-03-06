//
//  DemodulationUtils.mm
//  Enlighten
//
//  Created by Darrin Willis on 3/30/15.
//  Copyright (c) 2015 18-549 Team10. All rights reserved.
//

#import "DemodulationUtils.h"

// This is how far apart our samples are for each sample
#define STEPS_PER_FRAME 10

// This is how many frames could go by before we are sure to see the preamble
#define PREAMBLE_FRAMES 30


//#define DEBUG1

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

+ (cv::Mat)getFFT:(cv::Mat)imageRows
withFreq:(cv::Mat)frequencies
andFrameSize:(int)frameSize
{
    
    // The number of frequencies we are scanning for
    int numFreqs = frequencies.rows;
    
    int stepSize = frameSize / STEPS_PER_FRAME;
    
    int numWindows = imageRows.rows / stepSize;
    cv::Mat computedFft = cv::Mat(numFreqs, numWindows, CV_64F);
    
    int h = 0;
    for (int i = 0; i < imageRows.rows - frameSize; i+= stepSize) {
        //First get a Mat representing this image
        cv::Mat thisImage = imageRows.rowRange(i, i + frameSize);
        
        int pixel_new = i % frameSize;
        int pixel_old = frameSize - pixel_new;
        cv::Mat hann = [DemodulationUtils getHann:pixel_old];
        hann.push_back([DemodulationUtils getHann:pixel_new]);
        
        thisImage = thisImage.mul(hann);
        
        // This is copied from OpenCV documentation on how to use DFT
        cv::Mat padded;                            //expand input image to optimal size
        int m = cv::getOptimalDFTSize( thisImage.rows );
        int n = cv::getOptimalDFTSize( thisImage.cols ); // on the border add zero values
        cv::copyMakeBorder(thisImage, padded, 0, m - thisImage.rows,
                           0, n - thisImage.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
        
        cv::Mat planes[] = {cv::Mat_<double>(padded), cv::Mat::zeros(padded.size(), CV_64F)};
        cv::Mat complexI;
        cv::merge(planes, 2, complexI);
        
        cv::dft(complexI, complexI);

        cv::split(complexI, planes);
        cv::magnitude(planes[0], planes[1], planes[0]);
        cv::Mat thisFft = planes[0];
        
        cv::normalize(thisFft, thisFft, 0, 255, cv::NORM_MINMAX);
        
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

    cv::Mat preambleContainer =  preambleFft.colRange(0, PREAMBLE_FRAMES * STEPS_PER_FRAME+1);

    cv::Point preamblePoint;
    cv::minMaxLoc(preambleContainer, NULL, NULL, NULL, &preamblePoint);

    int preambleIdx = preamblePoint.x;
    int jumpPreamble = round((preRate + dataRate) / 2 * STEPS_PER_FRAME);
    int jumpData = round(dataRate * STEPS_PER_FRAME);
    
    int idxOn = preambleIdx + jumpPreamble;
    
    double onVal;
    cv::minMaxLoc(dataFft.colRange(idxOn - 3, idxOn + 3), NULL, &onVal);
    
    // This is the threshold to determine whether or not data is 0 or 1
    double threshold = 0.9 * onVal; //(onVal + offVal) / 2;
    
#ifdef DEBUG1
    std::cout << preambleIdx << " " << threshold << std::endl;
#endif
    
    // We take the start of the transfer + sending the pilot on and the data
    int byteLength = jumpPreamble + dataBits * jumpData;

    // Demod data should be an array of single bytes which are 0 or 1
    cv::Mat demodData = cv::Mat(1, dataBits, CV_8S);

    if (preambleIdx + byteLength > numberSamples) {
        // this should handle the error gracefully, this should never happen
        // because if the preamble is detected in the back half of the image,
        // then it didn't actually find a viable preamble.
        return cv::Mat::zeros(1, dataBits, CV_8U);
    } else {
        for (int i = 1; i <= dataBits; i++) {
            int bitIdx = preambleIdx + jumpPreamble + i * jumpData;
            double signalVal;
            cv::minMaxLoc(dataFft.colRange(bitIdx - 5, bitIdx + 5), NULL, &signalVal);
#ifdef DEBUG1
            std::cout << bitIdx + 1 << " ";
#endif
            BOOL demodVal = signalVal > threshold;
            demodData.at<BOOL>(0,i-1) = demodVal;
        }
    }
#ifdef DEBUG1
    std::cout << std::endl;
#endif
    return demodData;
}

@end
