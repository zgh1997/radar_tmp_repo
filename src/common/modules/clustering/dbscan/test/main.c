/**
 *   @file  main.c
 *
 *   @brief
 *      Unit Test code for the DBSCAN on DSP
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
#include "RADARDEMO_clusteringDBscan.h"
#include "cycle_measure.h"
#include "radarOsal_malloc.h"

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

#define MAXNUMHEAPS (3)
#define L2HEAPSIZE (0x2000)
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
#pragma DATA_SECTION(l1ScratchMem, ".L1ScratchSect")
uint8_t l1ScratchMem[L1SCRATCHSIZE];
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

uint16_t testFrameSteps[20] = { 5,    10,    15,    20,    25,    30,    35,    40,    45,    50,    55,    60,    65,    70,    75,    80,    90,   100,   110,   120};
float clusterInfoErrorDetectionThreshold = 1;

int main()
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
	float    *clusterInfoRef;
	float temp, diff;
	int16_t clusterID, ccID;
    RADARDEMO_clusteringDBscanReport *reportArray;
    RADARDEMO_clusteringDBscanConfig dbscanConfigParam;
    RADARDEMO_clusteringDBscanInput dbscanInputData;
    RADARDEMO_clusteringDBscanOutput dbscanOutputData;

	float *floatInputPoint, *floatSpeed;
	int16_t *int16InputPoint, *int16Speed;
	float *inputSNRArray, *SNRArray;
	float *inputAoaVar, *aoaVar;

    dbscanHandle handle;

    uint32_t totalNumPoints;
    uint32_t pointStart;
    uint32_t frameStart;
    uint32_t pointNum;

    char testSuccesful = 1;
    int32_t runNum;
    benchMark benchMark[20];
    int32_t       t1, t2;
    int32_t         tsc_overhead;

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

    dbscanConfigParam.epsilon = 1.5f;
    dbscanConfigParam.weight = 1.5f;
	dbscanConfigParam.vFactor = 2.0f;
    dbscanConfigParam.maxClusters = 24;
    dbscanConfigParam.minPointsInCluster = 4;
    dbscanConfigParam.maxPoints = 2048;
    dbscanConfigParam.inputPntPrecision = DBSCAN_INPUT_16BITFIXEDPT; //DBSCAN_INPUT_16BITFIXEDPT; //DBSCAN_INPUT_FLOATINGPT; //both point point and float point are matching, but the optimized code is no longer working.
    dbscanConfigParam.fixedPointScale = (1 << 8);

    if (dbscanConfigParam.inputPntPrecision == DBSCAN_INPUT_FLOATINGPT)
    {
    #ifdef _WIN32
        fin=fopen("..\\points.bin","rb");
    #else
        fin=fopen("..\\..\\..\\points.bin","rb");
    #endif
        if((pointArray = (point2d *)readArrayFromBinaryFile(fin, &pointFileLength)) == NULL)
        {
            printf("Error: Unable to allocate memory\n");
            fclose(fin);
            return 1;
        }
        totalNumPoints = pointFileLength/sizeof(point2d);

		floatInputPoint	=	(float *) radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, 2 * totalNumPoints * sizeof(float), 8);
		floatSpeed	=		(float *) radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, totalNumPoints * sizeof(float), 8);
		for (i = 0; i < totalNumPoints; i++)
		{
			floatInputPoint[2 * i]		=	pointArray[i].x;
			floatInputPoint[2 * i + 1]	=	pointArray[i].y;
			floatSpeed[i]				=	pointArray[i].speed;
		}
    }
    else
    {

    #ifdef _WIN32
        fin=fopen("..\\pointsFixed.bin","rb");
    #else
        fin=fopen("..\\..\\..\\pointsFixed.bin","rb");
    #endif
        if((pointArrayFxdp = (point2dfxdp *)readArrayFromBinaryFile(fin, &pointFileLength)) == NULL)
        {
            printf("Error: Unable to allocate memory\n");
            fclose(fin);
            return 1;
        }
        totalNumPoints	=	pointFileLength/sizeof(point2dfxdp);
		int16InputPoint	=	(int16_t *) radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, 2 * totalNumPoints * sizeof(int16_t), 8);
		int16Speed		=	(int16_t *) radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, totalNumPoints * sizeof(int16_t), 8);
		for (i = 0; i < totalNumPoints; i++)
		{
			int16InputPoint[2 * i]		=	pointArrayFxdp[i].x;
			int16InputPoint[2 * i + 1]	=	pointArrayFxdp[i].y;
			int16Speed[i]				=	pointArrayFxdp[i].speed;
		}
    }
    if (!fin) {
        printf("Error: Unable to open file\n");
        return 1;
    }
    fclose(fin);

    // add the code to include SNR array and aoaVar
    #ifdef _WIN32
        fin=fopen("..\\SNRArray.bin","rb");
    #else
        fin=fopen("..\\..\\..\\SNRArray.bin","rb");
    #endif
        if((inputSNRArray = (float *)readArrayFromBinaryFile(fin, &pointFileLength)) == NULL)
        {
            printf("Error: Unable to allocate memory\n");
            fclose(fin);
            return 1;
        }
    totalNumPoints = pointFileLength/sizeof(float);
    SNRArray = (float *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, totalNumPoints*sizeof(float), 8);
    for (i = 0; i < totalNumPoints; i++)
		SNRArray[i] = inputSNRArray[i];	
	if (!fin) {
        printf("Error: Unable to open file\n");
        return 1;
    }
    fclose(fin);

    #ifdef _WIN32
        fin=fopen("..\\aoaVar.bin","rb");
    #else
        fin=fopen("..\\..\\..\\aoaVar.bin","rb");
    #endif
        if((inputAoaVar = (float *)readArrayFromBinaryFile(fin, &pointFileLength)) == NULL)
        {
            printf("Error: Unable to allocate memory\n");
            fclose(fin);
            return 1;
        }
    totalNumPoints = pointFileLength/sizeof(float);
	aoaVar = (float *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, totalNumPoints*sizeof(float), 8);
    for (i = 0; i < totalNumPoints; i++)
		aoaVar[i] = inputAoaVar[i];
    
	if (!fin) {
        printf("Error: Unable to open file\n");
        return 1;
    }
    fclose(fin);



#ifdef _WIN32
    fin=fopen("..\\frames.bin","rb");
#else
    fin=fopen("..\\..\\..\\frames.bin","rb");
#endif
    if (!fin) {
        printf("Unable to open file!");
        return -1;
    }
    if((frameArray = (uint16_t *)readArrayFromBinaryFile(fin, &frameFileLength)) == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        fclose(fin);
        return -1;
    }
    fclose(fin);

    if(totalNumPoints != frameFileLength/sizeof(frameArray[0]))
    {
        printf("Error: Number of points differs from number of frame indices\n");
        return -1;
    }

 	#ifdef _WIN32
		fin=fopen("..\\indexM.bin","rb");
	#else
		fin=fopen("..\\..\\..\\indexM.bin","rb");
	#endif

    if (!fin) {
        printf("Unable to open file!");
        return -1;
    }
    if((indexM = (uint16_t *)readArrayFromBinaryFile(fin, &fileLength)) == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        fclose(fin);
        return -1;
    }
    if(totalNumPoints != fileLength/sizeof(indexM[0]))
    {
        printf("Error: Number of indices differs from number of points\n");
        return -1;
    }
    fclose(fin);

 	#ifdef _WIN32
	    fin=fopen("..\\numCluster.bin","rb");
	#else
		fin=fopen("..\\..\\..\\numCluster.bin","rb");
	#endif

    if (!fin) {
        printf("Unable to open file!");
        return -1;
    }
    if((numCluster = (uint16_t *)readArrayFromBinaryFile(fin, &frameFileLength)) == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        fclose(fin);
        return -1;
    }
    fclose(fin);
    
 	#ifdef _WIN32
	    fin=fopen("..\\clusterInfo.bin","rb");
	#else
		fin=fopen("..\\..\\..\\clusterInfo.bin","rb");
	#endif
    if (!fin) {
        printf("Unable to open file!");
        return -1;
    }
    if((clusterInfoRef = (float *)readArrayFromBinaryFile(fin, &frameFileLength)) == NULL)
    {
        printf("Error: Unable to allocate memory\n");
        fclose(fin);
        return -1;
    }
    fclose(fin);

    clusterArray = (uint16_t *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, totalNumPoints*sizeof(uint16_t), 8);
    reportArray = (RADARDEMO_clusteringDBscanReport *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, dbscanConfigParam.maxClusters*sizeof(RADARDEMO_clusteringDBscanReport), 8);
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
			dbscanInputData.SNRArray = &SNRArray[pointStart];
			dbscanInputData.aoaVar = &aoaVar[pointStart];

            dbscanOutputData.IndexArray = &clusterArray[pointStart];
            dbscanOutputData.report = &reportArray[0];

            benchMark[runNum].startTime = getCPUTime();
            benchMark[runNum].startCycles = ranClock();

            RADARDEMO_clusteringDBscanRun(handle, &dbscanInputData, &dbscanOutputData);

            benchMark[runNum].endCycles = ranClock() - benchMark[runNum].startCycles - tsc_overhead;
            benchMark[runNum].endTime = getCPUTime() - benchMark[runNum].startTime;
            benchMark[runNum].numPoints = dbscanInputData.numPoints;

       	    if (dbscanOutputData.numCluster == numCluster[runNum])
			{
				for (clusterID = 0; clusterID < numCluster[runNum]; clusterID ++)
			    {
					temp = (float)(dbscanOutputData.report[clusterID].xCenter);
					diff = clusterInfoRef[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, xCenter got %d instead of %d\n", runNum, clusterID, temp, clusterInfoRef[ccID]);
					ccID ++;

					temp = (float)(dbscanOutputData.report[clusterID].yCenter);
					diff = clusterInfoRef[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, yCenter got %d instead of %d\n", runNum, clusterID, temp, clusterInfoRef[ccID]);
					ccID ++;

					temp = (float)(dbscanOutputData.report[clusterID].xSize);
					diff = clusterInfoRef[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, xSize got %d instead of %d\n", runNum, clusterID, temp, clusterInfoRef[ccID]);
					ccID ++;

					temp = (float)(dbscanOutputData.report[clusterID].ySize);
					diff = clusterInfoRef[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, xCenter got %d instead of %d\n", runNum, clusterID, temp, clusterInfoRef[ccID]);
					ccID ++;

					temp = (float)(dbscanOutputData.report[clusterID].avgVel);
					diff = clusterInfoRef[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, avgVel got %d instead of %d\n", runNum, clusterID, temp, clusterInfoRef[ccID]);
					ccID ++;

					temp = dbscanOutputData.report[clusterID].centerRangeVar;
					diff = clusterInfoRef[ccID] - temp;
					if (abs(diff) > (clusterInfoErrorDetectionThreshold*256))
                       printf("Error: runNum %d, clusterID %d, centerRangeVar got %f instead of %f\n", runNum, clusterID, temp, clusterInfoRef[ccID]);
					ccID ++;

					temp = dbscanOutputData.report[clusterID].centerAngleVar;
					diff = clusterInfoRef[ccID] - temp;
					if (abs(diff) > clusterInfoErrorDetectionThreshold)
                       printf("Error: runNum %d, clusterID %d, centerAngleVar got %f instead of %f\n", runNum, clusterID, temp, clusterInfoRef[ccID]);
					ccID ++;

					temp = dbscanOutputData.report[clusterID].centerDopplerVar;
					diff = clusterInfoRef[ccID] - temp;
					if (abs(diff) > (clusterInfoErrorDetectionThreshold * 256))
                       printf("Error: runNum %d, clusterID %d, centerDopplerVar got %f instead of %f\n", runNum, clusterID, temp, clusterInfoRef[ccID]);
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

    benchMark[runNum].startTime = getCPUTime();
    benchMark[runNum].startCycles = ranClock();

    RADARDEMO_clusteringDBscanRun(handle, &dbscanInputData, &dbscanOutputData);

    benchMark[runNum].endCycles = ranClock() - benchMark[runNum].startCycles - tsc_overhead;
    benchMark[runNum].endTime = getCPUTime() - benchMark[runNum].startTime;
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
    radarOsal_memFree(SNRArray, totalNumPoints*sizeof(float));
    radarOsal_memFree(aoaVar, totalNumPoints*sizeof(float));
    radarOsal_memFree(clusterArray, totalNumPoints*sizeof(uint16_t));
    if (dbscanConfigParam.inputPntPrecision == 0)
        radarOsal_memFree(pointArray, pointFileLength);
    else
        radarOsal_memFree(pointArrayFxdp, pointFileLength);
    radarOsal_memFree(frameArray, frameFileLength);

    radarOsal_memDeInit();

    return 0;
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
    buffer = (char *)radarOsal_memAlloc(RADARMEMOSAL_HEAPTYPE_DDR_CACHED, 0, fileLen+1, 8);
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


