clc; clear; close all;
% This code runs the demodulator for a single light.
% Currently this code runs on a test data set. 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% First, we configure some parameters. Most of these values are determined
% by the parameters programmed on the code running on the light.

%InputData : The data bit sequence ID of the light. Though this is
% an 8 bit sequence, the light only transmitted 6 bits
% OnFreqPt : This refers to the index of the particular frequency, when you
% take an FFT. In reality, each light has some frequency as its ON
% frequency. In this case I think it was 2.5k,3k,3.5k,4k. There is a
% linear relation between LED frequency and image frequency depending on
% the rolling shutter scan rate. Further, this number is also determined on
% how many point-FFT you take.

%InputData = 11;OnFreqPt = 79;   % Light 1
 InputData = 42;OnFreqPt = 91;   % Light 2
% InputData = 52;OnFreqPt = 104;  % Light 3
% InputData = 21;OnFreqPt = 113;  % Light 4
PreFreqPt = 63; % The point on the FFT that gives a peak for the frequency corresponding to the sync/preamble frequency
 
DispOn = 1; %Plot on or off
Dbits = 6; % Number of data bits. On the lights contoller side, we programmed the light to send only 6 bits.

% The symbol duration of data and the preamble was 50ms. We are interested
% in knowing this parameter in terms of a frame/image length. Both are
% equal to 50/33 ~ 1.5 in this case.

FrameTime = 50; % Symbol length in ms
DataRate = FrameTime/33;
PreRate = 1.5; % Length of preamble in terms of number of frames

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
VLCData = bitand(InputData,2^Dbits-1);%ANDing with Dbits number of ones

% Sample data path

%DataPath = 'DataSet/2014-03-13-22_27_49_659_pos09_sp04_L1_and_3_off';
DataPath = 'DataSet/2014-03-13-22_29_13_291_pos09_sp04';

[BerVal, storefft, on_val, off_val] = DemodulateDataOffline(DispOn,DataPath,InputData,PreRate,DataRate,Dbits,PreFreqPt,OnFreqPt);
disp(['Num of bit errors : ',num2str(BerVal),'/',num2str(Dbits)]);
