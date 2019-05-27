/**
 *   @file  testClusterTracker.c
 *
 *   @brief
 *      Unit Test bench for the cluster tracking module on DSP
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/ 
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#include "swpform.h"
#include <stdio.h>
#include <stdlib.h>
#include "modules/tracking/clusterTracker/api/RADARDEMO_clusterTracker.h"
//#include "modules/tracking/common/api/RADARDEMO_tracking_commonDef.h"

#include <modules/utilities/cycle_measure.h>
#include <modules/utilities/radarOsal_malloc.h>

#define MAXNUMHEAPS (3)
#define L2HEAPSIZE (0x30000)
#define L2SCRATCHSIZE (0x2000)
#define DDRSCRATCHSIZE (0x10000)
#define DDRHEAPSIZE (0x400000)
#define L1SCRATCHSIZE (0x8000)
#pragma DATA_SECTION(memHeapL2, ".L2heap")
uint8_t memHeapL2[L2HEAPSIZE];
#pragma DATA_SECTION(ddrScratchMem, ".ddrScratchSect")
uint8_t ddrScratchMem[DDRSCRATCHSIZE];
#pragma DATA_SECTION(ddrHeapMem, ".ddrHeap")
uint8_t ddrHeapMem[DDRHEAPSIZE];
#pragma DATA_SECTION(l2ScratchMem, ".L2ScratchSect")
uint8_t l2ScratchMem[L2SCRATCHSIZE];
#pragma DATA_SECTION(l1ScratchMem, ".L2ScratchSect")
uint8_t l1ScratchMem[L1SCRATCHSIZE];
radarOsal_heapConfig heapconfig[MAXNUMHEAPS];


int main()
{
    int32_t testIdx, totalError, runID;
    RADARDEMO_clusterTracker_config clusterTrackerConfigParam;
    RADARDEMO_tracker_output clusterTrackerOutput;
    RADARDEMO_clusterTracker_errorCode clusterTrackerErrorCode;
    RADARDEMO_tracker_input clusterTrackerInput;
    RADARDEMO_trackerInput_dataType *inputInfo;
	RADARDEMO_trackerOutput_dataType *outputInfo;
	uint16_t clusterSize;
	uint16_t trackerIdx, trackerID;

    float * tempInputPtr,  *outputPtr;
	float input[9];
	float output[7];
	float error, errorDetectTH;
    void * handle;
    FILE * f1in, * f2in, *fout;
    int32_t t1, t2;
    int32_t tsc_overhead;

	errorDetectTH = 1e-2;

    cache_setL1PSize(CACHE_L1_32KCACHE);
    cache_setL1DSize(CACHE_L1_32KCACHE);
    cache_setL2Size(CACHE_0KCACHE);
#ifndef _TMS320C6600 //C674x
    cache_setMar((unsigned int *)0xC0000000, 0x20000000, Cache_PC | Cache_PFX);
#else //C66x
//    Cache_setMar((unsigned int *)0x5B000000, 0x40000, Cache_PC | Cache_PFX);
    cache_setMar((unsigned int *)0x80000000, 0x20000000, Cache_PC | Cache_PFX);
#endif
    startClock();

    t1 = ranClock();
    t2 = ranClock();
    tsc_overhead = t2 - t1;

	memset(heapconfig, 0, sizeof(heapconfig));
	heapconfig[RADARMEMOSAL_HEAPTYPE_DDR_CACHED].heapType 	= 	RADARMEMOSAL_HEAPTYPE_DDR_CACHED;
	heapconfig[RADARMEMOSAL_HEAPTYPE_DDR_CACHED].heapAddr   = 	(int8_t *) &ddrHeapMem[0];
	heapconfig[RADARMEMOSAL_HEAPTYPE_DDR_CACHED].heapSize   = 	DDRHEAPSIZE;
	heapconfig[RADARMEMOSAL_HEAPTYPE_DDR_CACHED].scratchAddr= 	NULL; 	/* not DDR scratch for TM demo  */
	heapconfig[RADARMEMOSAL_HEAPTYPE_DDR_CACHED].scratchSize= 	0; 	/* not DDR scratch for TM demo  */

	heapconfig[RADARMEMOSAL_HEAPTYPE_LL2].heapType 			= 	RADARMEMOSAL_HEAPTYPE_LL2;
	heapconfig[RADARMEMOSAL_HEAPTYPE_LL2].heapAddr   		= 	(int8_t *) &memHeapL2[0];
	heapconfig[RADARMEMOSAL_HEAPTYPE_LL2].heapSize   		= 	L2HEAPSIZE;
	heapconfig[RADARMEMOSAL_HEAPTYPE_LL2].scratchAddr   	= 	(int8_t *)&l2ScratchMem[0];;
	heapconfig[RADARMEMOSAL_HEAPTYPE_LL2].scratchSize   	= 	L2SCRATCHSIZE;

	heapconfig[RADARMEMOSAL_HEAPTYPE_LL1].heapType 			= 	RADARMEMOSAL_HEAPTYPE_LL1;
	heapconfig[RADARMEMOSAL_HEAPTYPE_LL1].heapAddr   		= 	NULL;
	heapconfig[RADARMEMOSAL_HEAPTYPE_LL1].heapSize   		= 	0;
	heapconfig[RADARMEMOSAL_HEAPTYPE_LL1].scratchAddr   	= 	(int8_t *)&l1ScratchMem[0];;
	heapconfig[RADARMEMOSAL_HEAPTYPE_LL1].scratchSize   	= 	L1SCRATCHSIZE;
    if(radarOsal_memInit(&heapconfig[0], MAXNUMHEAPS) == RADARMEMOSAL_FAIL)
    {
        printf("Error: radarOsal_memInit fail\n");
        return(1);
    }

	clusterTrackerConfigParam.trackerAssociationThreshold = 1.0;
    clusterTrackerConfigParam.measurementNoiseVariance = 1.0;
    clusterTrackerConfigParam.trackerActiveThreshold = 2;
    clusterTrackerConfigParam.trackerForgetThreshold = 3;
	clusterTrackerConfigParam.timePerFrame = 0.08;
	clusterTrackerConfigParam.iirForgetFactor = 0.25;
	clusterTrackerConfigParam.fxInputScalar = 256;

	inputInfo = (RADARDEMO_trackerInput_dataType *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, RADARDEMO_CT_MAX_NUM_CLUSTER*sizeof(RADARDEMO_trackerInput_dataType), 1);
    outputInfo = (RADARDEMO_trackerOutput_dataType *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, RADARDEMO_CT_MAX_NUM_TRACKER*sizeof(RADARDEMO_trackerOutput_dataType), 1);
	clusterTrackerInput.inputInfo = inputInfo;
	clusterTrackerOutput.outputInfo = outputInfo;
	totalError = 0;

#ifdef _WIN32
    f1in=fopen("..\\testinput.bin","rb");
#else
    f1in=fopen("..\\..\\..\\testinput.bin","rb");
#endif
    if(f1in == NULL)
    {
        printf("Fail open file testinput.bin\n");
        exit(1);
    }

#ifdef _WIN32
    f2in=fopen("..\\testinputSummary.bin","rb");
#else
    f2in=fopen("..\\..\\..\\testinputSummary.bin","rb");
#endif
    if(f2in == NULL)
    {
        printf("Fail open file testinputSummary.bin\n");
        exit(1);
    }

#ifdef _WIN32
    fout=fopen("..\\testOutput.bin","rb");
#else
    fout=fopen("..\\..\\..\\testOutput.bin","rb");
#endif
    if(fout == NULL)
    {
        printf("Fail open file testOutput.bin\n");
        exit(1);
    }

    handle = (void *) RADARDEMO_clusterTracker_create(&clusterTrackerConfigParam, &clusterTrackerErrorCode);
    if (clusterTrackerErrorCode > RADARDEMO_CLUSTERTRACKER_NO_ERROR)
    {
        printf("Failed to create clusterTracker module! Exit the test now!\n");
        exit(1);
    }


	tempInputPtr    =   (float *) input;
	outputPtr       =   (float *) output;
	runID = 0;
	while (fread(&clusterSize, sizeof(uint16_t), 1, f2in) == 1)
	{
		clusterTrackerInput.totalNumInput = clusterSize;
		for (testIdx = 0; testIdx < clusterSize; testIdx ++)
		{
		    fread(tempInputPtr, sizeof(float), 9, f1in);
			clusterTrackerInput.inputInfo[testIdx].xCenter = (int16_t)(tempInputPtr[0]);
			clusterTrackerInput.inputInfo[testIdx].yCenter = (int16_t)(tempInputPtr[1]);
			clusterTrackerInput.inputInfo[testIdx].avgVel = (int16_t)(tempInputPtr[2]);
			clusterTrackerInput.inputInfo[testIdx].xSize = (int16_t)(tempInputPtr[3]);
			clusterTrackerInput.inputInfo[testIdx].ySize = (int16_t)(tempInputPtr[4]);
			clusterTrackerInput.inputInfo[testIdx].numPoints = (uint16_t)(tempInputPtr[5]);
			clusterTrackerInput.inputInfo[testIdx].centerRangeVar = tempInputPtr[6];
			clusterTrackerInput.inputInfo[testIdx].centerAngleVar = tempInputPtr[7];
			clusterTrackerInput.inputInfo[testIdx].centerDopplerVar = tempInputPtr[8];
		}
        t1 = ranClock();
        clusterTrackerErrorCode = RADARDEMO_clusterTracker_run(
                            handle,
                            &clusterTrackerInput,
							clusterTrackerConfigParam.timePerFrame,
                            &clusterTrackerOutput);


        t2 = ranClock() - t1 - tsc_overhead;

		//zg debug print out
        printf("runID = %d, number of cluster = %d, number of active tracker = %d, clock time = %d \n", runID, clusterSize,clusterTrackerOutput.totalNumOfOutput, t2);
		runID ++;

		if (clusterTrackerErrorCode > RADARDEMO_CLUSTERTRACKER_NO_ERROR)
        {
            printf("RADARDEMO_clusterTracker_run fatal error! Exit the test now!\n");
            exit(1);
        }

		/*check results */
		for (trackerIdx = 0; trackerIdx < clusterTrackerOutput.totalNumOfOutput; trackerIdx ++)
		{
			fread(outputPtr, sizeof(float), 7, fout);
			trackerID = clusterTrackerOutput.outputInfo[trackerIdx].trackerID; 
			if (clusterTrackerOutput.outputInfo[trackerIdx].trackerID != ((uint16_t)(outputPtr[0])-1))
			{
                printf("trackerID = %d is not matching with reference results of %d\n", trackerID, ((uint16_t)(outputPtr[0])-1));
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].S_hat[0] - outputPtr[1];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
				printf("tracker #%d, S_hat[0] = %4.5f is not matching with reference results of %4.5f\n", trackerID, clusterTrackerOutput.outputInfo[trackerIdx].S_hat[0], outputPtr[1]);
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].S_hat[1] - outputPtr[2];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("tracker #%d, S_hat[1] = %4.5f is not matching with reference results of %4.5f\n", trackerID, clusterTrackerOutput.outputInfo[trackerIdx].S_hat[1], outputPtr[2]);
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].S_hat[2] - outputPtr[3];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("tracker #%d, S_hat[2] = %4.5f is not matching with reference results of %4.5f\n", trackerID, clusterTrackerOutput.outputInfo[trackerIdx].S_hat[2], outputPtr[3]);
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].S_hat[3] - outputPtr[4];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("tracker #%d, S_hat[3] = %4.5f is not matching with reference results of %4.5f\n", trackerID, clusterTrackerOutput.outputInfo[trackerIdx].S_hat[3], outputPtr[4]);
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].xSize - outputPtr[5];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("tracker #%d, xSize = %3.5f is not matching with reference results of %3.5f\n", trackerID, clusterTrackerOutput.outputInfo[trackerIdx].xSize, outputPtr[5]);
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].ySize - outputPtr[6];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("tracker #%d, ySize = %3.5f is not matching with reference results of %3.5f\n", clusterTrackerOutput.outputInfo[trackerIdx].ySize, outputPtr[6]);
				totalError ++;
			}
		}
    }
    if (totalError)
        printf("Overall test failed\n");
    else
        printf("Overall test passed\n");
	fclose(f1in);
	fclose(f2in);
}


