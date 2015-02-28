% This is the function that runs the demodulator on an input frame sequence

function [ErrAll,storefft, on_val, off_val] = DemodulateData(DispOn,DataPath,InputData,PreRate,DataRate,Dbits,PreFreqPt,OnFreqPt)

PreambleOcc = 15; %This is the number of frames within which we would definitely see a preamble. 
%This is done since we do not know where in the sequence of images the
%preamble lies.

Nfft        = 720; % Same as frame length. 

fnames = dir([DataPath,'/*.TIFF']); % List the files
numfids = length(fnames);   % List of all the files. Name will vary from 0.TIFF to (N-1).TIFF

FreqPts = [PreFreqPt,OnFreqPt];
%Correspnding to [preamble, ON freq L1, ON freq L2, ON freq L3, ON freq L4]

storefft    = [];%zeros(260,2);
Match_f1    = [];%zeros(1,PreambleOcc);
BitInd      = [];%zeros(1,Dbits);
SignalVal   = [];%zeros(1,Dbits);
Thresh      = [];
Thresh_all = [];
SNR_all = [];
ErrAll = [];
FFTamp_all = [];
DataIn = [];DataOut=[];

trial_path = [DataPath,'/'];
HannWind = hann(Nfft); % A window for filtering 

for K = 1:numfids
    % Read the image
    RdImage = imread([trial_path,fnames(K).name]);
    BWImage = RdImage;
    
    % Concatenate the images
    if (K==1)
        Image = BWImage;
    else
        Image = [Image; BWImage];
    end
end
[Nrows,Ncols] = size(BWImage);

StepsPerFrame   = 10; %Number of pixels to jump while sliding the window. N=1 would be accurate but too long to run. 
% Higher N would run faster but less accurate

StepSize        = Nfft/StepsPerFrame;

h = 1;
% Find FFT across  frames
for i = 1:StepSize:(numfids-1)*Nrows-1 % Slide upto end of second last frame
    ImageTemp = Image(i:i+Nfft-1,:); %Slide the window and form the temporary image
    
    % Find the extent of pixels that belong to the old and the new frame
    pixel_new = mod(i,Nfft)-1;
    pixel_old = Nfft - pixel_new;
    wind = [hann(pixel_old) ; hann(pixel_new)];
    ImageWindowed = bsxfun(@times,im2double(ImageTemp),wind);
    % Window each of the two segments with a window and concatenate them
    % back together
    ImageTemp = ImageWindowed;
    
    colfft = fft(ImageTemp); %FFT of each column
    avgfft = abs(mean(colfft,2)); % We are interested only in the 2D-FFT along the vertical line - which is the mean of the column-wise FFT
    
    %Find root of power in the frequency band of interest by taking RMS value
    %We ned not store all FFT values. We only want to store the ones
    %corresponding to the frequencies of interest
    for p=1:length(FreqPts)
        storefft(h,p) = sqrt(sum([avgfft(FreqPts(p)-2:FreqPts(p)+2)].^2)/5);
    end
    h=h+1;
end

% Determine the point at which f1 is maximum - to find the bit boundaries -
% not using this in the actual demodulation
f1 = storefft(:,1);%figure;plot(storefft(:,1));
for q = 1:StepsPerFrame*PreambleOcc %15 frames - assuming preamble is present within first 20 frames
    Match_f1(q) = mean(f1(q:q+StepsPerFrame-1));
end
[v,ind] = max(Match_f1);

% Finding the preamble - Imax has the index of the preamble
[v,Ipre] = max(storefft(1:PreambleOcc*StepsPerFrame,1));

% We determine by how much to jump ahead for hitting the next data bit.
JumpPreamble = round(((PreRate+DataRate)/2)*StepsPerFrame);
JumpData     = round(DataRate*StepsPerFrame);

Ion = Ipre + JumpPreamble;% Pilot or ON-value is value @ (OFF-value + 1 frame)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Earlier I had a for loop here to demodulate each light. I removed that,
% but the same code can be used to demodualte all lights with little
% modification

DemodDataSet = zeros(1,Dbits);

    % OFF-value is the value of the frequency when the preamble is detected
    off_val = storefft(Ipre,2);
    on_val = storefft(Ion,2);
    
    Thresh_all = (on_val + off_val)/2;
    [FFTamp_all,dummy] = max(storefft(:,2)); %Find peak of FFT
    
    DemodData = zeros(1,Dbits);
    
    % Check if the data will fit in the frames or not
    if((Ipre + JumpPreamble + Dbits*JumpData)>length(storefft))
        DataComplete = 0;
        % In case the preamble occurs quite late, and we do not have
        % sufficient frames to accomodate all the data,
        % Do dummy demodulation - just to keep the plots, return values
        % consistent.
        for bits = 1:Dbits
            BitInd(bits) = Ipre + JumpPreamble; %Read all bits at the preamble
            SignalVal(bits) = storefft(BitInd(bits),2);
            DemodData(bits) = double(SignalVal(bits)> Thresh_all); %Determine if 1 or 0 by comparing with threshold
        end
        
    else % Normal good operation - we have sufficient frames to get all the data
        DataComplete = 1;
        for bits = 1:Dbits
            BitInd(bits) = Ipre + JumpPreamble + bits*JumpData;
            SignalVal(bits) = storefft(BitInd(bits),2);
            DemodData(bits) = double(SignalVal(bits)> Thresh_all);
        end
    end
    DemodDataSet(1:Dbits) = DemodData;

    DataIn = InputData;
    DataOut = bin2dec(num2str(DemodDataSet));
    ErrAll= biterr(DataIn,DataOut);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% The code below is for plotting the result
ErrAll;
Nbits = Dbits + 3; % This is used for determining how many bit boudaries to mark

if(DispOn == 1)
    
    TrueLight = find(ErrAll==min(ErrAll),1);
    T = TrueLight; %index of TrueLight in Storefft. Add 1 since preamble occupies position 1

    LnWd = 2;TitleSize = 14;

    % Plot the sliding window FFT
    %axes(handles.demodplot);
    figure;
    plot(storefft(:,1:2),'LineWidth',LnWd);hold on;


    % Mark the points used for detection
    GB = [0,0.3,0.3];GreenBlue = [0 0.8 0.8];
    scatter(BitInd,storefft(BitInd,T+1),100,GB,'MarkerFaceColor',GreenBlue,'LineWidth',2);

    % Mark the ON, OFF reference value points
    orange = [1 0.5 0.3];
    scatter([Ipre Ion],storefft([Ipre Ion],T+1),100,orange,'MarkerFaceColor','y','LineWidth',2);%'rhexagram'
    text(Ipre,storefft(Ipre,1),'\rightarrow Preamble','FontSize',10,'FontWeight','bold');
    text(Ipre,storefft(Ipre,T+1),'\leftarrow Pilot OFF value','FontSize',10,'FontWeight','bold');
    text(Ion,storefft(Ion,T+1),'\leftarrow Pilot ON value','FontSize',10,'FontWeight','bold');

    % Draw the threshold line
    x2 = 1:length(storefft);
    y2 = Thresh_all(T)*ones(1,length(storefft));
    plot(x2,y2,'c','LineWidth',LnWd);

    % Mark the bit boundaries
    t = ind:StepsPerFrame:ind+Nbits*StepsPerFrame;
    line_1_pos = Ipre-PreRate*StepsPerFrame/2;
    line_2_pos = Ipre-PreRate*StepsPerFrame/2+ PreRate*StepsPerFrame;
    t = [line_1_pos,line_2_pos:JumpData:line_2_pos+PreRate*StepsPerFrame+JumpData*(Dbits+1)];
    x1 = sort ([t t]);
    y1 = repmat([min(min(storefft)) max(max(storefft)) max(max(storefft)) min(min(storefft)) ],1,round(length(t)/2+1));
    y1 = y1(1:length(x1));
    plot(x1,y1,'m--');

    axis tight
    hold off;
    legend({'Preamble','Light On freq','Training','Decision','Detection samples'});
    %legend({'Preamble','Light 1','Light 2','Light 3','Light 4','Detection samples'});

    %disp(['Processed ',num2str(trial),'/',num2str(NtestPts)]);
    %disp('Frames Processed');
    %set(handles.EditText,'String','Frames Processed');

    title('Demodulation','FontSize',TitleSize);
end
