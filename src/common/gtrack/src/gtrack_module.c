/**
 *   @file  gtrack_module.c
 *
 *   @brief
 *      Implementation of the GTRACK Algorithm MODULE
 *
 *  \par
 *  NOTE:
 *      (C) Copyright 2017 Texas Instruments, Inc.
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

#include <math.h>
#include <float.h>
#include <ti/alg/gtrack/gtrack.h>
#include <ti/alg/gtrack/include/gtrack_int.h>

#define PI 3.14159265358979323846f

/**
*  @b Description
*  @n
*      This is a MODULE level predict function. The function is called by external step function to perform unit level kalman filter predictions
*
*  @param[in]  inst
*      Pointer to GTRACK module instance
*
*  \ingroup GTRACK_ALG_MODULE_FUNCTION
*
*  @retval
*      None
*/

void gtrack_modulePredict(GtrackModuleInstance *inst)
{
	GTrack_ListElem *tElem;
	uint16_t uid;

	tElem = gtrack_listGetFirst(&inst->activeList);
	while (tElem != 0)
	{
		uid = tElem->data;
		if (uid > inst->maxNumTracks)
		{
			/* This should never happen */
			gtrack_assert(0);
		}

		gtrack_unitPredict(inst->hTrack[uid]);

		tElem = gtrack_listGetNext(tElem);
	}
}

/**
*  @b Description
*  @n
*      This is a MODULE level associatiation function. The function is called by external step function to associate measurement points with known targets
*
*  @param[in]  inst
*      Pointer to GTRACK module instance
*  @param[in]  point
*      Pointer to an array of input measurments. Each measurement has range/angle/radial velocity information
*  @param[in]  num
*      Number of input measurements
*
*  \ingroup GTRACK_ALG_MODULE_FUNCTION
*
*  @retval
*      None
*/

void gtrack_moduleAssociate(GtrackModuleInstance *inst, GTRACK_measurementPoint *point, uint16_t num)
{
	GTrack_ListElem *tElem;
	uint16_t uid;

	tElem = gtrack_listGetFirst(&inst->activeList);
	while (tElem != 0)
	{
		uid = tElem->data;
		gtrack_unitScore(inst->hTrack[uid], point, inst->bestScore, inst->bestIndex, inst->isUniqueIndex, num);

		tElem = gtrack_listGetNext(tElem);
	}
}

/**
*  @b Description
*  @n
*      This is a MODULE level allocation function. The function is called by external step function to allocate new targets for the non-associated measurement points
*
*  @param[in]  inst
*      Pointer to GTRACK module instance
*  @param[in]  point
*      Pointer to an array of input measurments. Each measurement has range/angle/radial velocity information
*  @param[in]  num
*      Number of input measurements
*
*  \ingroup GTRACK_ALG_MODULE_FUNCTION
*
*  @retval
*      None
*/
// inst: 每一个聚类对象(内含多个跟踪点)
void gtrack_moduleAllocate(GtrackModuleInstance *inst, GTRACK_measurementPoint *point, uint16_t num)
{
	uint16_t n, k;

	//	float un[3], uk[3];
	//	float unSum[3];
	GTRACK_measurementUnion mCenter;
	GTRACK_measurementUnion mCurrent;
	GTRACK_measurementUnion mSum;

	// TODO: 统计每个聚类中大速度数量,和小速度数量,用来用来消除手臂摆动
	// uint16_t maxDopplerNum[];
	// uint16_t minDopplerNum[];

	uint16_t handCauseNum; //手臂抬起产生的点数

	GTRACK_measurement_vector hs;
	GtrackUnitInstance *uinst;
	uint16_t allocNum;
	float dist;
	float allocSNR;
	GTrack_ListElem *tElemFree;
	GTrack_ListElem *tElemActive;
	uint16_t uid;
	bool isBehind;
	bool isSnrThresholdPassed;
	bool isAdjacent;
	bool isHandCause;			   //判断是不是手臂摆动引起的聚类
	GTRACK_cartesian_position pos; //The structure defines a position in cartesian space : 笛卡尔空间坐标系的坐标值
	//num: Number of input measurements(输入的待测量点数目)
	for (n = 0; n < num; n++)
	{
		if (inst->bestIndex[n] == GTRACK_ID_POINT_NOT_ASSOCIATED)
		{

			tElemFree = gtrack_listGetFirst(&inst->freeList);
			if (tElemFree == 0)
			{

#ifdef GTRACK_LOG_ENABLED
				if (inst->verbose & VERBOSE_WARNING_INFO)
					gtrack_log(GTRACK_VERBOSE_WARNING, "Maximum number of tracks reached!");
#endif
				return;
			}

			inst->allocIndex[0] = n; //
			allocNum = 1;
			allocSNR = point[n].snr;

			mCenter.vector = point[n].vector;
			mSum.vector = point[n].vector;

			for (k = n + 1; k < num; k++)
			{
				if (inst->bestIndex[k] == GTRACK_ID_POINT_NOT_ASSOCIATED)
				{

					mCurrent.vector = point[k].vector;

					mCurrent.vector.doppler = gtrack_unrollRadialVelocity(inst->params.maxRadialVelocity, mCenter.vector.doppler, mCurrent.vector.doppler);

					if (fabsf(mCurrent.vector.doppler - mCenter.vector.doppler) < inst->params.allocationParams.maxVelThre)
					{
						dist = gtrack_calcDistance(&mCenter.vector, &mCurrent.vector);
						if (sqrtf(dist) < inst->params.allocationParams.maxDistanceThre)
						{
							//TODO:搜集每个聚类中大速度,小速度,用来消除手臂摆动
							inst->allocIndex[allocNum] = k; //该聚类中,每个点归属的,聚类索引id为k

							allocNum++; // 一个聚类中,点的个数
							allocSNR += point[k].snr;
							// Update the centroid
							gtrack_vectorAdd(GTRACK_MEASUREMENT_VECTOR_SIZE, mCurrent.array, mSum.array, mSum.array);
							gtrack_vectorScalarMul(GTRACK_MEASUREMENT_VECTOR_SIZE, mSum.array, 1.0f / (float)allocNum, mCenter.array);
						}
					}
				}
			}
			// Minimum number of points in a set (一个聚类要求的,最小点数)
			if ((allocNum > inst->params.allocationParams.pointsThre) &&
				(fabsf(mCenter.vector.doppler) > inst->params.allocationParams.velocityThre))
			{
				isBehind = false;
				tElemActive = gtrack_listGetFirst(&inst->activeList);
				while (tElemActive != 0)
				{
					uid = tElemActive->data;
					gtrack_unitGetH(inst->hTrack[uid], (float *)&hs);

					if (gtrack_isPointBehindTarget(&mCenter.vector, &hs))
					{
						isBehind = true;
						break;
					}
					tElemActive = gtrack_listGetNext(tElemActive);
				}

				if (isBehind)
					isSnrThresholdPassed = allocSNR > inst->params.allocationParams.snrThreObscured;
				else
					isSnrThresholdPassed = allocSNR > inst->params.allocationParams.snrThre;

				//check to ensure this new track is not too close to existing tracks
				//在这进行近邻聚类判别,
				gtrack_sph2cart(&mCenter.vector, &pos); // 这个函数用于将一个向量从sherical转换成笛卡尔坐标
				isAdjacent = false;
				tElemActive = gtrack_listGetFirst(&inst->activeList);
				while (tElemActive != 0)
				{
					uid = tElemActive->data;
					uinst = (GtrackUnitInstance *)inst->hTrack[uid];
					// TODO: 0.5f,通过修改0.5这个阈值,调节两个聚类间的距离,来达到近邻判断
					if (sqrtf(powf(uinst->S_hat[0] - pos.posX, 2.0f) + powf(uinst->S_hat[2] - pos.posZ, 2.0f)) < 0.2f)
					{
						if ((pos.posZ - uinst->S_hat[2] > 0.3f))
						{
							isAdjacent = true;
							break;
						}
					}

					tElemActive = gtrack_listGetNext(tElemActive);
				}
				// TODO: 将每个聚类,重新判断,如果该聚类上方突然出现(足以满足聚类点数的另一个聚类,通过判断,检测是不是手臂抬起,如果是手臂抬起,则忽略此聚类)
				gtrack_sph2cart(&mCenter.vector, &pos); // 这个函数用于将一个向量从sherical转换成笛卡尔坐标
				isHandCause = false;
				tElemActive = gtrack_listGetFirst(&inst->activeList);
				while (tElemActive != 0)
				{
					uid = tElemActive->data;
					uinst = (GtrackUnitInstance *)inst->hTrack[uid];
					if ((pos.posZ > 1.6f) && (uinst->S_hat[8] > 1.0f)) //uinst->S_hat[8]为accZ
					{
						handCauseNum++;
						if (handCauseNum > 10)
						{
							isHandCause = true;
							break;
						}
					}
					tElemActive = gtrack_listGetNext(tElemActive);
				}
				handCauseNum = 0;

				// 一个聚类中的点,分散太远,就要拆开为多个聚类
				if (isSnrThresholdPassed && !isAdjacent && !isHandCause)
				{

					/* Associate points with new uid  */
					for (k = 0; k < allocNum; k++)
						inst->bestIndex[inst->allocIndex[k]] = (uint8_t)tElemFree->data;

					/* Allocate new tracker */
					inst->targetNumTotal++;   //Total number of tracked targets: 跟踪目标总数
					inst->targetNumCurrent++; //Number of currently tracked Targets: 当前跟踪目标数量
					tElemFree = gtrack_listDequeue(&inst->freeList);

					// FIXME: 将聚类中心信噪比加入inst->hTrack[tElemFree->data]中，即相应的unit中
					GtrackUnitInstance *curUnitInst;
					curUnitInst = (GtrackUnitInstance *)inst->hTrack[tElemFree->data];
					curUnitInst->unitSNR = allocSNR;

					gtrack_unitStart(inst->hTrack[tElemFree->data], inst->heartBeat, inst->targetNumTotal, &mCenter.vector);
					gtrack_listEnqueue(&inst->activeList, tElemFree);
				}
			}
		}
	}
}

/**
*  @b Description
*  @n
*      This is a MODULE level update function. The function is called by external step function to perform unit level kalman filter updates
*
*  @param[in]  inst
*      Pointer to GTRACK module instance
*  @param[in]  point
*      Pointer to an array of input measurments. Each measurement has range/angle/radial velocity information
*  @param[in]  var
*      Pointer to an array of input measurment variances. Set to NULL if variances are unknown
*  @param[in]  num
*      Number of input measurements
*
*  \ingroup GTRACK_ALG_MODULE_FUNCTION
*
*  @retval
*      None
*/

void gtrack_moduleUpdate(GtrackModuleInstance *inst, GTRACK_measurementPoint *point, GTRACK_measurement_vector *var, uint16_t num)
{
	GTrack_ListElem *tElem;
	GTrack_ListElem *tElemToRemove;
	uint16_t uid;
	TrackState state;

	tElem = gtrack_listGetFirst(&inst->activeList);
	while (tElem != 0)
	{
		uid = tElem->data;
		state = gtrack_unitUpdate(inst->hTrack[uid], point, var, inst->bestIndex, inst->isUniqueIndex, num);
		if (state == TRACK_STATE_FREE)
		{
			tElemToRemove = tElem;
			tElem = gtrack_listGetNext(tElem);
			gtrack_listRemoveElement(&inst->activeList, tElemToRemove);

			gtrack_unitStop(inst->hTrack[tElemToRemove->data]);
			inst->targetNumCurrent--;
			gtrack_listEnqueue(&inst->freeList, tElemToRemove);
		}
		else
			tElem = gtrack_listGetNext(tElem);
	}
}
/**
*  @b Description
*  @n
*      This is a MODULE level report function. The function is called by external step function to obtain unit level data
*
*  @param[in]  inst
*      Pointer to GTRACK module instance
*  @param[out]  t
*      Pointer to an array of \ref GTRACK_targetDesc.
*      This function populates the descritions for each of the tracked target
*  @param[out]  tNum
*      Pointer to a uint16_t value.
*      Function returns a number of populated target descriptos 
*
*  \ingroup GTRACK_ALG_MODULE_FUNCTION
*
*  @retval
*      None
*/

void gtrack_moduleReport(GtrackModuleInstance *inst, GTRACK_targetDesc *t, uint16_t *tNum)
{
	GTrack_ListElem *tElem;
	uint16_t uid;
	uint16_t num = 0;

	tElem = gtrack_listGetFirst(&inst->activeList);
	while (tElem != 0)
	{
		uid = tElem->data;
		gtrack_unitReport(inst->hTrack[uid], &t[num++]);
		tElem = gtrack_listGetNext(tElem);
	}
	*tNum = num;
}
