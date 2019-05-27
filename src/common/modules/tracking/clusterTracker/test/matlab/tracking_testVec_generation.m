accNumFrames = 2;
testVecGenFlag = 1;
createMovieFlag = 0;

global pathPara;

%% get the input path and testList 
pro_path = getenv('MMWAVE_SIM_DEV');
%pathPara = strcat(pro_path,'\radarsim\main\radarCharacterization_AR12EA_Board\input\dynamic_dallas_param'); 
%pathPara = strcat(pro_path,'\radarsim\main\radarCharacterization_AR12EA_Board\input\adc_samples_tm70_param'); 
%pathPara = 'C:\Userdata\zigang\mmwave_sim_dev\data\march31\adc_samples_car_ds3b_48db_tm70_2x4_param';
pathPara = 'C:\Userdata\zigang\mmwave_sim_dev\data\cz_feb11\dynamic_scenes\adc_samples_lot_ds2_tm_2x4_f11_param';

%% load the detection and angle estimation output
%load scene5_detectOut.mat
%load movingScene2_detectOut.mat
%load newAlps_test1_angleEstOut.mat
%load adc_samples_car_ds3b_48db_tm70_2x4_detectOut.mat
%load test1_parkingLot_detectOut_k04p0.mat
load adc_samples_lot_ds2_tm_2x4_f11_detectOut.mat
%clusterObj = dbscan('epsilon', 1.5, 'weight', 1.25,'minPoints', accNumFrames+1);
clusterObj = dbscan('epsilon', 1.0, 'weight', 1.5,'minPoints', accNumFrames+1);
simTopObj = simTop('pfile',pathPara, 'name','simTop');     
numRxAnt = simTopObj.numRxAnt;  
if (simTopObj.MIMO_enable)
    DOAObj = DOA('method', 3, 'varTH', 5, 'numAntenna',numRxAnt*2,'antPos',-[0:numRxAnt*2-1]);    
else
    DOAObj = DOA('method', 3, 'varTH', 7.5); 
end
trackObj = clusterTracker(); %'trackerForgetThreshold', 5);
dt = simTopObj.framePeriodicity; % 20ms frame

i = 0; 

for frameID = accNumFrames:length(detection_results)
    angleEst{frameID} = datapath(DOAObj, detection_results{frameID});   
end

totalFrame = length(angleEst);
trackerSpeed = zeros(200, round(totalFrame/accNumFrames));
trackerDoppler = zeros(200, round(totalFrame/accNumFrames));
if testVecGenFlag
    fidInput = fopen('testinput.bin','w');
    fidFrameInfo = fopen('testinputSummary.bin','w');
    fidOutput = fopen('testOutput.bin','w');
end
for frameID = accNumFrames:accNumFrames:460 %totalFrame
    angleEst_acc = [];
    for ff = 0:accNumFrames - 1
       angleEst_acc = [angleEst_acc, angleEst{frameID-ff}];
    end
   
    pointArray = reFormatForCluster(DOAObj, angleEst_acc);        
    [clusterID, clusterInfo] = datapath(clusterObj, pointArray);
    [u, uvar] = generateInfoForClusterTracking(trackObj, clusterInfo);
    [track, numOfTracker]= datapath(trackObj, u, uvar, dt*accNumFrames);

    if (testVecGenFlag)   
        fwrite(fidFrameInfo, size(u, 2), 'uint16'); 
        for ind = 1:size(u, 2)
            fwrite(fidInput, round(clusterInfo(ind).xCenter*2^8), 'float');
            fwrite(fidInput, round(clusterInfo(ind).yCenter*2^8), 'float');
            fwrite(fidInput, round(clusterInfo(ind).avgVel*2^8), 'float');
            fwrite(fidInput, round(clusterInfo(ind).xSize*2^8), 'float');
            fwrite(fidInput, round(clusterInfo(ind).ySize*2^8), 'float');
            fwrite(fidInput, clusterInfo(ind).numPoints, 'float');
            fwrite(fidInput, clusterInfo(ind).centerRangeVar*2^16,'float');
            fwrite(fidInput, clusterInfo(ind).centerAngleVar,'float');
            fwrite(fidInput, clusterInfo(ind).centerDopplerVar*2^16,'float');
        end
        for ind = 1:numOfTracker
            fwrite(fidOutput, track(ind).trackerId, 'float');
            fwrite(fidOutput, track(ind).S(1:4), 'float');
            fwrite(fidOutput, track(ind).xsize, 'float');
            fwrite(fidOutput, track(ind).ysize, 'float');
        end
    end        
    i = i+1;
    display(max(clusterObj.vel));
    hfig1 = plotClusteringResultIn2D_dbScan(pointArray, clusterID, clusterInfo, 200);
    [hfig2, trackerSpeed(:,i), trackerDoppler(:,i)] = plotClusteringTrackingResultIn2D(frameID, pointArray, clusterID, clusterInfo, track, numOfTracker, 300);
    if (i == 59)
        %break;
    end
    if createMovieFlag
        F1(i) = getframe(hfig1);
        F2(i) = getframe(hfig2);
    end
end
if testVecGenFlag
    fclose(fidInput);
    fclose(fidFrameInfo);
    fclose(fidOutput);
end
figure(1); 
%subplot(1, 2, 1); 
plot((accNumFrames:accNumFrames:totalFrame)*dt, sqrt(trackerSpeed.'),'r'); hold on;
title('Tracking speed over time')
xlabel('Time (s)')
ylabel('Speed (m/s)')
%subplot(1, 2, 2); 
plot((accNumFrames:accNumFrames:totalFrame)*dt, abs(trackerDoppler.'),'b')
title('Doppler speed over time')
xlabel('Time (s)')
ylabel('Speed (m/s)')

% create a movie
if (createMovieFlag)
    vidObj = VideoWriter('clusteringOut_ds2_tm_2x4_f11_detectOut.mp4', 'MPEG-4');
    vidObj.FrameRate = 1/dt/accNumFrames; %50ms frames
    open(vidObj);
    writeVideo(vidObj,F1);
    close(vidObj); 
    
    vidObj = VideoWriter('trackingOut_ds2_tm_2x4_f11.mp4', 'MPEG-4');
    vidObj.FrameRate = 1/dt/accNumFrames; %50ms frames
    open(vidObj);
    writeVideo(vidObj,F2);
    close(vidObj); 
end