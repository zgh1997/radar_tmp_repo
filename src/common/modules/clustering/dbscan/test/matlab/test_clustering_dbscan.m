% testbench for DBSCAN clustering
close all

SAVETESTVEC = 1;
dt = 20e-3; % 20ms frame
maxPoints = 1000;

% Create DBSCAN clastering object
global pathPara;

%% get the input path and testList 
pro_path = getenv('MMWAVE_SIM_DEV');
pathPara = strcat(pro_path,'\radarsim\main\autoRadar\input\Alek_adjustedHighRes_param');  
clusterObj = dbscan('epsilon', 1.5, 'weight', 1.5, 'minPoints', 4);
DOAObj = DOA('method', 3); 

load('angleEstResult_forClustering.mat');
numFrames = length(angleEst);
frameSteps = [5:5:80 90 100 110 120];
ind = [];
startFrame = 1;
outArrayAcc = [];
SNRArrayAcc = [];
aoaVarAcc = [];
clusterIndexAll = [];
numClusterAll = [];
clusterInfoAll = [];
for i = 1:5
    angleEst_acc = [];
    endFrame = startFrame + frameSteps(i) - 1;
    if endFrame > numFrames
        display('number of frames exceed');
        display(i)
        break;
    end
    for ff = startFrame:endFrame
        angleEst_acc = [angleEst_acc, angleEst{ff}];
        numPoint = length(angleEst{ff});
        ind = [ind; ff * ones(numPoint, 1)];
    end

    pointArray = reFormatForCluster(DOAObj, angleEst_acc); 
    [outArray, SNRArray, aoaVar] = reFormatForClusterPrint(DOAObj, angleEst_acc); 
    outArrayAcc = [outArrayAcc; outArray];
    SNRArrayAcc = [SNRArrayAcc; SNRArray];
    aoaVarAcc = [aoaVarAcc; aoaVar];
    if (length(outArray) > maxPoints)
        display('number of maxPoints exceed');
        display(i);
        break;
    end
    [clusterIndex, clusterInfo] = datapath(clusterObj, pointArray);
    clusterIndexAll = [clusterIndexAll;clusterIndex];
    numClusterAll = [numClusterAll; max(clusterIndex)];
    if i == 1
        clusterInfoAll = clusterInfo.';
    else
        clusterInfoAll = [clusterInfoAll; clusterInfo.'];        
    end
    
    startFrame = endFrame + 1;
end
pos = outArrayAcc;


if SAVETESTVEC == 1
    fid = fopen('points.bin', 'wb');
    ptemp = pos.';
    cnt = fwrite(fid, ptemp, 'float');
    fclose(fid);
    
    fid = fopen('SNRArray.bin', 'wb');
    ptemp = SNRArrayAcc.';
    cnt = fwrite(fid, ptemp, 'float');
    fclose(fid);

    fid = fopen('aoaVar.bin', 'wb');
    ptemp = aoaVarAcc.';
    cnt = fwrite(fid, ptemp, 'float');
    fclose(fid);        
    
    fid = fopen('pointsFixed.bin', 'wb');
    ptemp = round(pos.'*2^8);
    cnt = fwrite(fid, ptemp, 'int16');
    fclose(fid);

    fid = fopen('frames.bin', 'wb');
    cnt = fwrite(fid, ind, 'uint16');
    fclose(fid);

    fid = fopen('indexM.bin', 'wb');
    cnt = fwrite(fid, clusterIndexAll, 'uint16');
    fclose(fid);
    
    %fid = fopen('indexMFixed.bin', 'wb');
    %cnt = fwrite(fid, clusterIndexAll, 'uint16');
    %fclose(fid);

    fid = fopen('numCluster.bin','wb');
    cnt = fwrite(fid, numClusterAll, 'uint16');
    fclose(fid);

    fid = fopen('clusterInfo.bin','wb');
    for ind = 1:length(clusterInfoAll)
        cnt = fwrite(fid, round(clusterInfoAll(ind).xCenter*2^8), 'float');
        cnt = fwrite(fid, round(clusterInfoAll(ind).yCenter*2^8), 'float');
        cnt = fwrite(fid, round(clusterInfoAll(ind).xSize*2^8), 'float');
        cnt = fwrite(fid, round(clusterInfoAll(ind).ySize*2^8), 'float');
        cnt = fwrite(fid, round(clusterInfoAll(ind).avgVel*2^8), 'float');
        cnt = fwrite(fid, clusterInfoAll(ind).centerRangeVar*2^16, 'float');
        cnt = fwrite(fid, clusterInfoAll(ind).centerAngleVar, 'float');
        cnt = fwrite(fid, clusterInfoAll(ind).centerDopplerVar*2^16, 'float');
    end
    fclose(fid);
    
    
    
end