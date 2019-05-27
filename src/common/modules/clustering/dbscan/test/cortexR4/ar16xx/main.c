/*
 *   @file  main.c
 *
 *   @brief
 *      Unit Test code for the DBSCAN on R4
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
#include "modules/clustering/dbscan/api/RADARDEMO_clusteringDBscan.h"
#include <modules/utilities/radarOsal_malloc.h>

#define MAXNUMHEAPS (2)
#define L2HEAPSIZE (0x20000)
#define DDRSCRATCHSIZE (0x10000)
#define DDRHEAPSIZE (0x10000)
#define L2SCRATCHSIZE (0x2000)
#pragma DATA_SECTION(memHeapL2, ".L2heap")
uint8_t memHeapL2[L2HEAPSIZE];
#pragma DATA_SECTION(memHeapDDR, ".DDRheap")
uint8_t memHeapDDR[DDRHEAPSIZE];
#pragma DATA_SECTION(ddrScratchMem, ".ddrScratchSect")
uint8_t ddrScratchMem[DDRSCRATCHSIZE];
#pragma DATA_SECTION(l2ScratchMem, ".L2ScratchSect")
uint8_t l2ScratchMem[L2SCRATCHSIZE];

radarOsal_heapConfig heapconfig[MAXNUMHEAPS];

char *readArrayFromBinaryFile(FILE *fin, uint32_t *length);
typedef struct
{
    double startTime;
    uint64_t startCycles;
    uint64_t benchPoint[10];
    double endTime;
    uint64_t endCycles;
    int numPoints;
} benchMark;


typedef struct
{
    float x;
    float y;
    float speed;
} point2d;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t speed;
} point2dfxdp;


uint16_t testFrameSteps[20] = { 5,    10,    15,    20,    25,    30,    35,    40,    45,    50,    55,    60,    65,    70,    75,    80,    90,   100,   110,   120};
float clusterInfoErrorDetectionThreshold = 1;


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
    int32_t kk, i;
    FILE *fin, *fout;
    uint32_t fileLength, frameFileLength, pointFileLength;
    point2d *pointArray;
    point2dfxdp *pointArrayFxdp;
    uint16_t *indexM;
    uint16_t *clusterArray;
    uint16_t *frameArray;
	uint16_t *numCluster;
	int16_t    *clusterInfo;
	int16_t temp, diff;
	int16_t clusterID, ccID;
    RADARDEMO_clusteringDBscanReport *reportArray;
    RADARDEMO_clusteringDBscanConfig dbscanConfigParam;
    RADARDEMO_clusteringDBscanInput dbscanInputData;
    RADARDEMO_clusteringDBscanOutput dbscanOutputData;

	float *floatInputPoint, *floatSpeed;
	int16_t *int16InputPoint, *int16Speed;

    dbscanHandle handle;

    uint32_t totalNumPoints;
    uint32_t pointStart;
    uint32_t frameStart;
    uint32_t pointNum;

    char testSuccesful = 1;
    int32_t runNum;
    benchMark benchMark[20];
    int32_t       t1;
    int32_t         tsc_overhead;

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

    dbscanConfigParam.epsilon = 1.5f;
    dbscanConfigParam.weight = 1.5f;
    dbscanConfigParam.maxClusters = 100;
    dbscanConfigParam.minPointsInCluster = 4;
    dbscanConfigParam.maxPoints = 2048;
    dbscanConfigParam.inputPntPrecision = DBSCAN_INPUT_16BITFIXEDPT; //DBSCAN_INPUT_16BITFIXEDPT; //DBSCAN_INPUT_FLOATINGPT; //both point point and float point are matching, but the optimized code is no longer working.
    dbscanConfigParam.fixedPointScale = (1 << 8);

    if (dbscanConfigParam.inputPntPrecision == DBSCAN_INPUT_FLOATINGPT)
    {
        fin=fopen("..\\..\\points.bin","rb");
        if (!fin) {
            printf("Error: Unable to open file points.bin\n");
            exit(-1);
        }
        if((pointArray = (point2d *)readArrayFromBinaryFile(fin, &pointFileLength)) == NULL)
        {
            printf("Error: Unable to allocate memory\n");
            fclose(fin);
        }
        totalNumPoints = pointFileLength/sizeof(point2d);

		floatInputPoint	=	(float *) radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_LL2, 0, 2 * totalNumPoints * sizeof(float), 8);
		floatSpeed	=		(float *) radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_LL2, 0, totalNumPoints * sizeof(float), 8);
		for (i = 0; i < totalNumPoints; i++)
		{
			floatInputPoint[2 * i]		=	pointArray[i].x;
			floatInputPoint[2 * i + 1]	=	pointArray[i].y;
			floatSpeed[i]				=	pointArray[i].speed;
		}
    }
    else
    {

        fin=fopen("..\\..\\pointsFixed.bin","rb");
        if (!fin) {
            printf("Error: Unable to open file pointsFixed.bin\n");
            exit(-1);
        }
        if((pointArrayFxdp = (point2dfxdp *)readArrayFromBinaryFile(fin, &pointFileLength)) == NULL)
        {
            printf("Error: Unable to allocate memory\n");
            fclose(fin);
        }
        totalNumPoints	=	pointFileLength/sizeof(point2dfxdp);
		int16InputPoint	=	(int16_t *) radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_LL2, 0, 2 * totalNumPoints * sizeof(int16_t), 8);
		int16Speed		=	(int16_t *) radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_LL2, 0, totalNumPoints * sizeof(int16_t), 8);
		for (i = 0; i < totalNumPoints; i++)
		{
			int16InputPoint[2 * i]		=	pointArrayFxdp[i].x;
			int16InputPoint[2 * i + 1]	=	pointArrayFxdp[i].y;
			int16Speed[i]				=	pointArrayFxdp[i].speed;
		}
    }

    fin=fopen("..\\..\\frames.bin","rb");
    if (!fin) {
        printf("Unable to open file frames.bin!");
        exit(-1);
    }
    if((frameArray = (uint16_t *)readArrayFromBinaryFile(fin, &frameFileLength)) == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        fclose(fin);
    }
    fclose(fin);

    if(totalNumPoints != frameFileLength/sizeof(frameArray[0]))
    {
        printf("Error: Number of points differs from number of frame indices\n");
    }

    if (dbscanConfigParam.inputPntPrecision == DBSCAN_INPUT_FLOATINGPT)
    {
		fin=fopen("..\\..\\indexM.bin","rb");
    }
    else
    {
        fin=fopen("..\\..\\indexMFixed.bin","rb");
    }

    if (!fin) {
        printf("Unable to open file indexM.bin!");
        exit(-1);
    }
    if((indexM = (uint16_t *)readArrayFromBinaryFile(fin, &fileLength)) == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        fclose(fin);
    }
    if(totalNumPoints != fileLength/sizeof(indexM[0]))
    {
        printf("Error: Number of indices differs from number of points\n");
    }
    fclose(fin);


	fin=fopen("..\\..\\numCluster.bin","rb");
    if (!fin) {
        printf("Unable to open file numCluster.bin!");
        exit(-1);
    }
    if((numCluster = (uint16_t *)readArrayFromBinaryFile(fin, &frameFileLength)) == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        fclose(fin);
    }
    fclose(fin);
    
	fin=fopen("..\\..\\clusterInfo.bin","rb");
    if (!fin) {
        printf("Unable to open file clusterInfo.bin!");
        exit(-1);
    }
    if((clusterInfo = (int16_t *)readArrayFromBinaryFile(fin, &frameFileLength)) == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        fclose(fin);
    }
    fclose(fin);

    clusterArray = (uint16_t *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_LL2, 0, totalNumPoints*sizeof(uint16_t), 8);
    reportArray = (RADARDEMO_clusteringDBscanReport *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_LL2, 0, dbscanConfigParam.maxClusters*sizeof(RADARDEMO_clusteringDBscanReport), 8);
    handle = RADARDEMO_clusteringDBscanCreate(&dbscanConfigParam);
	dbscanInputData.inputPntPrecision = dbscanConfigParam.inputPntPrecision;

    // Send 100 frames per step
    frameStart = frameArray[0];
    pointStart = 0;
    pointNum = 0;
    runNum = 0;
    kk = 0;
	ccID = 0;

    while (pointNum < totalNumPoints)
    {
        if(frameArray[pointNum] >= frameStart+testFrameSteps[kk])
        {
            dbscanInputData.numPoints = pointNum-pointStart;
            printf("pointNum = %d, arraySize = %d\n", pointNum, dbscanInputData.numPoints);
            if (dbscanInputData.inputPntPrecision == DBSCAN_INPUT_FLOATINGPT)
			{
				dbscanInputData.pointArray.pointArrayFloat = (float *)&floatInputPoint[2 * pointStart];
				dbscanInputData.speed.speedFloat = (float *)&floatSpeed[pointStart];
			}
            else
			{
				dbscanInputData.pointArray.pointArrayInt16 = (int16_t *)&int16InputPoint[2 * pointStart];
				dbscanInputData.speed.speedInt16 = (int16_t *)&int16Speed[pointStart];
			}

            dbscanOutputData.IndexArray = &clusterArray[pointStart];
            dbscanOutputData.report = &reportArray[0];

            benchMark[runNum].startTime = Cycleprofiler_getTimeStamp();
            benchMark[runNum].startCycles = Cycleprofiler_getTimeStamp();

            RADARDEMO_clusteringDBscanRun(handle, &dbscanInputData, &dbscanOutputData);

            benchMark[runNum].endCycles = Cycleprofiler_getTimeStamp() - benchMark[runNum].startCycles - tsc_overhead;
            benchMark[runNum].endTime = Cycleprofiler_getTimeStamp() - benchMark[runNum].startTime;
            benchMark[runNum].numPoints = dbscanInputData.numPoints;

       	    if (dbscanOutputData.numCluster == numCluster[runNum])
			{
				for (clusterID = 0; clusterID < numCluster[runNum]; clusterID ++)
			    {
					temp = dbscanOutputData.report[clusterID].xCenter;
					diff = clusterInfo[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, xCenter got %d instead of %d\n", runNum, clusterID, temp, clusterInfo[ccID]);
					ccID ++;

					temp = dbscanOutputData.report[clusterID].yCenter;
					diff = clusterInfo[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, yCenter got %d instead of %d\n", runNum, clusterID, temp, clusterInfo[ccID]);
					ccID ++;

					temp = dbscanOutputData.report[clusterID].xSize;
					diff = clusterInfo[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, xSize got %d instead of %d\n", runNum, clusterID, temp, clusterInfo[ccID]);
					ccID ++;

					temp = dbscanOutputData.report[clusterID].ySize;
					diff = clusterInfo[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, xCenter got %d instead of %d\n", runNum, clusterID, temp, clusterInfo[ccID]);
					ccID ++;

					temp = dbscanOutputData.report[clusterID].maxVel;
					diff = clusterInfo[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, maxVel got %d instead of %d\n", runNum, clusterID, temp, clusterInfo[ccID]);
					ccID ++;

					temp = dbscanOutputData.report[clusterID].minVel;
					diff = clusterInfo[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, xCenter got %d instead of %d\n", runNum, clusterID, temp, clusterInfo[ccID]);
					ccID ++;
				}
			}
			else {
                printf("Error: runNum %d, number of cluster mismatch\n", runNum);

			}
            runNum++;
            kk++;
            frameStart = frameArray[pointNum];
            pointStart = pointNum;
        }
        pointNum++;
    }
    // Process the leftovers
    dbscanInputData.numPoints = pointNum-pointStart;
    if (dbscanInputData.inputPntPrecision == DBSCAN_INPUT_FLOATINGPT)
	{
		dbscanInputData.pointArray.pointArrayFloat = (float *)&floatInputPoint[2 * pointStart];
		dbscanInputData.speed.speedFloat = (float *)&floatSpeed[pointStart];
	}
    else
	{
		dbscanInputData.pointArray.pointArrayInt16 = (int16_t *)&int16InputPoint[2 * pointStart];
		dbscanInputData.speed.speedInt16 = (int16_t *)&int16Speed[pointStart];
	}
    dbscanOutputData.IndexArray = &clusterArray[pointStart];
    dbscanOutputData.report = &reportArray[0];

    benchMark[runNum].startTime = Cycleprofiler_getTimeStamp();
    benchMark[runNum].startCycles = Cycleprofiler_getTimeStamp();

    RADARDEMO_clusteringDBscanRun(handle, &dbscanInputData, &dbscanOutputData);

    benchMark[runNum].endCycles = Cycleprofiler_getTimeStamp() - benchMark[runNum].startCycles - tsc_overhead;
    benchMark[runNum].endTime = Cycleprofiler_getTimeStamp() - benchMark[runNum].startTime;
    benchMark[runNum].numPoints = dbscanInputData.numPoints;


    for(pointNum=0; pointNum<totalNumPoints; pointNum++) {
        if(clusterArray[pointNum] != indexM[pointNum]) {
            testSuccesful = 0;
            printf("Error: index %d, got %d instead of %d\n", pointNum, clusterArray[pointNum], indexM[pointNum]);
        }
    }
    if(testSuccesful == 1)
        printf("Test passed\n");

    printf("numPoints\t\tcycleUsed \n");
    for (pointNum = 0; pointNum <= runNum;  pointNum++)
    {
        printf("%d\t\t %llu\n", benchMark[pointNum].numPoints, benchMark[pointNum].endCycles);
    }

    // Save results
    fout=fopen("..\\indexC.bin","wb");
    fwrite(clusterArray, sizeof(uint16_t), totalNumPoints, fout);
    fclose(fout);


    RADARDEMO_clusteringDBscanDelete(handle);

	if(dbscanConfigParam.inputPntPrecision == DBSCAN_INPUT_FLOATINGPT)
	{
	    radarOsal_memFree(floatInputPoint, 2 * totalNumPoints*sizeof(float));
	    radarOsal_memFree(floatSpeed, totalNumPoints*sizeof(float));
	}
	else
	{
	    radarOsal_memFree(int16InputPoint, 2 * totalNumPoints*sizeof(int16_t));
	    radarOsal_memFree(int16Speed, totalNumPoints*sizeof(int16_t));
	}
    radarOsal_memFree(clusterArray, totalNumPoints*sizeof(uint16_t));
    if (dbscanConfigParam.inputPntPrecision == 0)
        radarOsal_memFree(pointArray, pointFileLength);
    else
        radarOsal_memFree(pointArrayFxdp, pointFileLength);
    radarOsal_memFree(frameArray, frameFileLength);

    radarOsal_memDeInit();

	exit(1);
}
char * readArrayFromBinaryFile(FILE *fin, uint32_t *length)
{
    unsigned long fileLen;
    char *buffer;

    //Get file length
    fseek(fin, 0, SEEK_END);
    fileLen=ftell(fin);
    fseek(fin, 0, SEEK_SET);

    //Allocate memory
    buffer = (char *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, fileLen+1, 1);
    if (!buffer)
    {
        fprintf(stderr, "Memory error!");
        *length = 0;
        return NULL;
    }

    //Read file contents of the binary file into the buffer
    fread(buffer, fileLen, 1, fin);
    *length  = fileLen;
    return buffer;
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

