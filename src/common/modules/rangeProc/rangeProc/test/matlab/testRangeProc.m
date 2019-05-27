% Test vector generation for range processing module test
clear
x = 1000;
rand('state',x);
randn('state',x);

saveTestVec     =  0;

rangeFFTOrder   =   6:12; % range FFT size from 64 to 4096
nTests          =   length(rangeFFTOrder);
nChirpsPerFrame =   32;
ADCbitRange     =   [12 14];
largestFFTSize  =   2^(rangeFFTOrder(end));
xin             =   round((rand(largestFFTSize, 1) + 1j*rand(largestFFTSize,1))*(2^14 - 1));
win1D           =   round(rand(largestFFTSize/2, 1) * (2^15));
win2D           =   round(rand(nChirpsPerFrame/2, 1) * (2^15));

if saveTestVec == 1
	fid1 = fopen('rangeProctestvectors.bin', 'wb');
    fwrite(fid1, nTests, 'int32');
    fwrite(fid1, rangeFFTOrder, 'int32');
    fwrite(fid1, nChirpsPerFrame, 'int32');
    xintemp(1:2:2*largestFFTSize) = imag(xin);
    xintemp(2:2:2*largestFFTSize) = real(xin);
    fwrite(fid1, xintemp, 'int16');
    clear xintemp;
    fwrite(fid1, win1D, 'int16');
    fwrite(fid1, win2D, 'int16');
end


for ii = 1:nTests
    FFT1Dsize   =   2^(rangeFFTOrder(ii));
    itemp       =   max(round((rand(1) * FFT1Dsize)/2), 1);
    nValidSamplesPerChirp = FFT1Dsize - itemp;
    ADCNOB      =   ADCbitRange(mod(ii, length(ADCbitRange)) + 1);
    inputTemp   =   floor(xin(1:nValidSamplesPerChirp)/(2^(14 - ADCNOB)));
    
    DCadjustFlag = mod(ii, 1);
    
    if DCadjustFlag == 1
        %testing DC adjust for ADC data
        inputTemp     = inputTemp - mean(inputTemp);
        %testing DC adjust for ADC data
    elseif DCadjustFlag == 2
        inputReal = real(inputTemp);
        indx1 = find(inputReal >= 2^(ADCNOB -1));
        inputReal(indx1) = inputReal(indx1) - 2^ADCNOB;
        
        inputImag = imag(inputTemp);
        indx1 = find(inputImag >= 2^(ADCNOB -1));
        inputImag(indx1) = inputImag(indx1) - 2^ADCNOB;
        
        inputTemp =  inputReal + 1j*inputImag;
        inputTemp     = inputTemp - mean(inputTemp);
    else
        inputTemp   =   inputTemp - (2^(ADCNOB - 1)) - 1j*(2^(ADCNOB - 1)); % this is not testing module functionality!!! Rather it adjusts the range of input in test bench to make input more realistic.
    end
    
    if mod(nValidSamplesPerChirp, 4) == 0
        win1DTemp   =   [win1D(1:nValidSamplesPerChirp/2); win1D(nValidSamplesPerChirp/2:-1:1)];
    else
        win1DTemp   =   [win1D(1:2*floor(nValidSamplesPerChirp/4)); 32767*ones(mod(nValidSamplesPerChirp, 4), 1); win1D(2*floor(nValidSamplesPerChirp/4):-1:1)];
    end
    win2DTemp   =   [win2D; win2D(end:-1:1)];
    
    win1Doutput =   round(win1DTemp.* inputTemp/(2^15));
    FFT1Doutput =   fft(win1Doutput, FFT1Dsize)/sqrt(FFT1Dsize);
    win2Doutput =   win2DTemp(ii) * FFT1Doutput/(2^15);


    if saveTestVec == 1
        fwrite(fid1, nValidSamplesPerChirp, 'int32');
        fwrite(fid1, ADCNOB, 'int32');
        fwrite(fid1, DCadjustFlag, 'int32');
        xintemp(1:2:2*nValidSamplesPerChirp) = imag(win1Doutput);
        xintemp(2:2:2*nValidSamplesPerChirp) = real(win1Doutput);
        fwrite(fid1, xintemp, 'float');
        clear xintemp;
        xintemp(1:2:2*FFT1Dsize) = imag(FFT1Doutput);
        xintemp(2:2:2*FFT1Dsize) = real(FFT1Doutput);
        fwrite(fid1, xintemp, 'float');
        clear xintemp;
        xintemp(1:2:2*FFT1Dsize) = imag(win2Doutput);
        xintemp(2:2:2*FFT1Dsize) = real(win2Doutput);
        fwrite(fid1, xintemp, 'float');
        clear xintemp;
    end
end

if saveTestVec == 1
    fclose(fid1);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%