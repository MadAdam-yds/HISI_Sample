
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
/*****************************************************************************
* function : set vpss chn.
*****************************************************************************/
HI_S32 SAMPLE_COMM_VPSS_SetExtChn(VPSS_GRP VpssGrp,VPSS_CHN VpssExtChn,VPSS_CHN_ATTR_S* pastVpssChnAttr ){
    VPSS_EXT_CHN_ATTR_S extChnAttr = {0};
    extChnAttr.s32BindChn = 1;//bind phy chn 1
    extChnAttr.enCompressMode = pastVpssChnAttr->enCompressMode;
    extChnAttr.enPixelFormat = pastVpssChnAttr->enPixelFormat;
    extChnAttr.enVideoFormat = pastVpssChnAttr->enVideoFormat;
    extChnAttr.u32Depth = pastVpssChnAttr->u32Depth;
    extChnAttr.stFrameRate.s32SrcFrameRate = pastVpssChnAttr->stFrameRate.s32SrcFrameRate;
    extChnAttr.stFrameRate.s32DstFrameRate = pastVpssChnAttr->stFrameRate.s32DstFrameRate;
    extChnAttr.u32Height = pastVpssChnAttr->u32Height;
    extChnAttr.u32Width = pastVpssChnAttr->u32Width;
    extChnAttr.enDynamicRange = pastVpssChnAttr->enDynamicRange;
    HI_S32 s32Ret = HI_MPI_VPSS_SetExtChnAttr(VpssGrp,VpssExtChn,&extChnAttr);

    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_SetExtChnAttr(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssExtChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}
/*****************************************************************************
* function : start vpss grp.
*****************************************************************************/
HI_S32 SAMPLE_COMM_VPSS_Start(VPSS_GRP VpssGrp, HI_BOOL* pabChnEnable, VPSS_GRP_ATTR_S* pstVpssGrpAttr, VPSS_CHN_ATTR_S* pastVpssChnAttr)
{
    VPSS_CHN VpssChn;
    HI_S32 s32Ret;
    HI_S32 j;

    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);

    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
        return HI_FAILURE;
    }

    for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++)
    {
        if(HI_TRUE == pabChnEnable[j])
        {
            VpssChn = j;
            s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &pastVpssChnAttr[VpssChn]);

            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);

            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
                return HI_FAILURE;
            }
        }
    }

    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);

    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 SAMPLE_COMM_VPSS_ExtChn_Stop(VPSS_GRP VpssGrp,VPSS_CHN VpssExtChn){
    HI_S32 s32Ret = HI_SUCCESS;
    s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssExtChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x! VpssExtChn %u\n", s32Ret,VpssExtChn);
        return HI_FAILURE;
    }

    return HI_TRUE;
}
/*****************************************************************************
* function : stop vpss grp
*****************************************************************************/
HI_S32 SAMPLE_COMM_VPSS_Stop(VPSS_GRP VpssGrp, HI_BOOL* pabChnEnable)
{
    HI_S32 j;
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_CHN VpssChn;

    for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++)
    {
        if(HI_TRUE == pabChnEnable[j])
        {
            VpssChn = j;
            s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);

            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("failed with %#x! VpssChn %u\n", s32Ret,VpssChn);
                return HI_FAILURE;
            }
        }
    }

    s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);

    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);

    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
