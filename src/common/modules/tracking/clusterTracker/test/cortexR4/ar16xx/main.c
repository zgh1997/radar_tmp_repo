/**
 *   @file  main.c
 *
 *   @brief
 *      Unit Test bench for the cluster tracking module on R4F
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
 

/**************************************************************************
 *************************** Include Files ********************************
 **************************************************************************/

/* Standard Include Files. */
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* BIOS/XDC Include Files. */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/IHeap.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/heaps/HeapBuf.h>
#include <ti/sysbios/heaps/HeapMem.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/utils/cycleprofiler/cycle_profiler.h>

/* mmWave SK Include Files: */
#include <ti/common/sys_common.h>
#include <ti/drivers/osal/SemaphoreP.h>
#include <ti/drivers/esm/esm.h>
#include <ti/drivers/soc/soc.h>

/**************************************************************************
 *************************** Local Definitions ****************************
 **************************************************************************/

/**************************************************************************
 *************************** Global Definitions ****************************
 **************************************************************************/


SOC_Handle gSOCHandle;


#include "swpform.h"
#include "modules/tracking/clusterTracker/api/RADARDEMO_clusterTracker.h"
#include <modules/utilities/radarOsal_malloc.h>

#define MAXNUMHEAPS (2)
#define L2HEAPSIZE (0x20000)
#define DDRSCRATCHSIZE (0x10000)
#define DDRHEAPSIZE (0x40000)
#define L2SCRATCHSIZE (0x3000)
#pragma DATA_SECTION(memHeapL2, ".L2heap")
uint8_t memHeapL2[L2HEAPSIZE];
#pragma DATA_SECTION(memHeapDDR, ".DDRheap")
uint8_t memHeapDDR[DDRHEAPSIZE];
#pragma DATA_SECTION(ddrScratchMem, ".ddrScratchSect")
uint8_t ddrScratchMem[DDRSCRATCHSIZE];
#pragma DATA_SECTION(l2ScratchMem, ".L2ScratchSect")
uint8_t l2ScratchMem[L2SCRATCHSIZE];

radarOsal_heapConfig heapconfig[MAXNUMHEAPS];


/**
 *  @b Description
 *  @n
 *      System Initialization Task which initializes the various
 *      components in the system.
 *
 *  @retval
 *      Not Applicable.
 */
static void Test_initTask(UArg arg0, UArg arg1)
{
    int32_t testIdx, totalError;
    RADARDEMO_clusterTracker_config clusterTrackerConfigParam;
    RADARDEMO_tracker_output clusterTrackerOutput;
    RADARDEMO_clusterTracker_errorCode clusterTrackerErrorCode;
    RADARDEMO_tracker_input clusterTrackerInput;
    RADARDEMO_trackerInput_dataType *inputInfo;
	RADARDEMO_trackerOutput_dataType *outputInfo;
	uint16_t clusterSize;
	uint16_t trackerIdx;

    float * tempInputPtr,  *outputPtr;
	float input[9];
	float output[7];
	float error, errorDetectTH; 
    void * handle;
    FILE * f1in, * f2in, *fout;
    int32_t t1, t2;
    int32_t tsc_overhead;

	errorDetectTH = 1e-2;

    /* Configure banchmark counter */
    Pmu_configureCounter(0, 0x11, FALSE);
    Pmu_startCounter(0);

    t1 = Cycleprofiler_getTimeStamp();
	tsc_overhead	=	Cycleprofiler_getTimeStamp() - t1;
	
    memset(heapconfig, 0, sizeof(heapconfig));
    memcpy(heapconfig[0].heapName, "RADARMEMOSAL_HEAPTYPE_DDR_CACHED", sizeof("RADARMEMOSAL_HEAPTYPE_DDR_CACHED"));
    heapconfig[0].heapAddr   = (int8_t *)memHeapDDR;    /* not used for default DDR heap  */
    heapconfig[0].heapSize   = DDRHEAPSIZE;   /* not used for default DDR heap  */
    heapconfig[0].scratchAddr   = (int8_t *)ddrScratchMem;  /* not used for default DDR heap  */
    heapconfig[0].scratchSize   = DDRSCRATCHSIZE;   /* not used for default DDR heap  */

    memcpy(heapconfig[1].heapName, "RADARMEMOSAL_HEAP_LL2", sizeof("RADARMEMOSAL_HEAP_LL2"));
    heapconfig[1].heapAddr   = (int8_t *)memHeapL2;
    heapconfig[1].heapSize   = L2HEAPSIZE;
    heapconfig[1].scratchAddr   = (int8_t *)l2ScratchMem;   /* not used for default DDR heap  */
    heapconfig[1].scratchSize   = L2SCRATCHSIZE;    /* not used for default DDR heap  */

    if(radarOsal_memInit(&heapconfig[0], MAXNUMHEAPS) == RADARMEMOSAL_FAIL)
    {
        printf("Error: radarOsal_memInit fail\n");
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

    f1in=fopen("..\\..\\testinput.bin","rb");
    if(f1in == NULL)
    {
        printf("Fail open file testinput.bin\n");
        exit(1);
    }

    f2in=fopen("..\\..\\testinputSummary.bin","rb");
    if(f2in == NULL)
    {
        printf("Fail open file testinputSummary.bin\n");
        exit(1);
    }

    fout=fopen("..\\..\\testOutput.bin","rb");
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
        t1 = Cycleprofiler_getTimeStamp();
        clusterTrackerErrorCode = RADARDEMO_clusterTracker_run(
                            handle,
                            &clusterTrackerInput,
							clusterTrackerConfigParam.timePerFrame,
                            &clusterTrackerOutput);

        t2 = Cycleprofiler_getTimeStamp() - t1 - tsc_overhead;

        printf("number of cluster = %d, number of active tracker = %d, clock time = %d \n", clusterSize,clusterTrackerOutput.totalNumOfOutput, t2);

		if (clusterTrackerErrorCode > RADARDEMO_CLUSTERTRACKER_NO_ERROR)
        {
            printf("RADARDEMO_clusterTracker_run fatal error! Exit the test now!\n");
            exit(1);
        }

		/*check results */
		for (trackerIdx = 0; trackerIdx < clusterTrackerOutput.totalNumOfOutput; trackerIdx ++)
		{
			fread(outputPtr, sizeof(float), 7, fout);
			if (clusterTrackerOutput.outputInfo[trackerIdx].trackerID != ((uint16_t)(outputPtr[0])-1))
			{
                printf("trackerID = %d is not matching with reference results of %d\n", clusterTrackerOutput.outputInfo[trackerIdx].trackerID, ((uint16_t)(outputPtr[0])-1));
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].S_hat[0] - outputPtr[1];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("S_hat[0] = %4.5f is not matching with reference results of %4.5f\n", clusterTrackerOutput.outputInfo[trackerIdx].S_hat[0], outputPtr[1]);
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].S_hat[1] - outputPtr[2];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("S_hat[1] = %4.5f is not matching with reference results of %4.5f\n", clusterTrackerOutput.outputInfo[trackerIdx].S_hat[1], outputPtr[2]);
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].S_hat[2] - outputPtr[3];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("S_hat[2] = %4.5f is not matching with reference results of %4.5f\n", clusterTrackerOutput.outputInfo[trackerIdx].S_hat[2], outputPtr[3]);
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].S_hat[3] - outputPtr[4];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("S_hat[3] = %4.5f is not matching with reference results of %4.5f\n", clusterTrackerOutput.outputInfo[trackerIdx].S_hat[3], outputPtr[4]);
				totalError ++;
			}
			
			error = clusterTrackerOutput.outputInfo[trackerIdx].xSize - outputPtr[5];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("xSize = %3.5f is not matching with reference results of %3.5f\n", clusterTrackerOutput.outputInfo[trackerIdx].xSize, outputPtr[5]);
				totalError ++;
			}

			error = clusterTrackerOutput.outputInfo[trackerIdx].ySize - outputPtr[6];
			if ((error > errorDetectTH) || (error < -errorDetectTH))
			{
                printf("ySize = %3.5f is not matching with reference results of %3.5f\n", clusterTrackerOutput.outputInfo[trackerIdx].ySize, outputPtr[6]);
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
	exit(1);
}


/**
 *  @b Description
 *  @n
 *      Entry point into the test code.
 *
 *  @retval
 *      Not Applicable.
 */
int main (void)
{
	Task_Params     taskParams;
    int32_t         errCode;
    SOC_Cfg         socCfg;

    /* Initialize the ESM: Dont clear errors as TI RTOS does it */
    ESM_init(0U);

    /* Initialize the SOC confiugration: */
    memset ((void *)&socCfg, 0, sizeof(SOC_Cfg));

    /* Populate the SOC configuration: */
    socCfg.clockCfg = SOC_SysClock_INIT;

    /* Initialize the SOC Module: This is done as soon as the application is started
     * to ensure that the MPU is correctly configured. */
    gSOCHandle = SOC_init (&socCfg, &errCode);
    if (gSOCHandle == NULL)
    {
        System_printf ("Error: SOC Module Initialization failed [Error code %d]\n", errCode);
        return -1;
    }

    /* Initialize the Task Parameters. */
    Task_Params_init(&taskParams);
    taskParams.stackSize = 4*1024;
    Task_create(Test_initTask, &taskParams, NULL);

    /* Debug Message: */
    System_printf ("Debug: Launching BIOS \n");

    /* Start BIOS */
	BIOS_start();
    return 0;
}

