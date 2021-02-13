#ifndef SHADER_INTEROP_RENDER_H
#define SHADER_INTEROP_RENDER_H

#include "shaderInterop.h"

struct FrameCB
{
    float2 gFrameScreenSize;
    float2 gFrameScreenSizeRCP;
};

struct CameraCB
{
    float4x4 gCameraVP;
    float4x4 gCameraView;
    float4x4 gCameraProj;
    float4x4 gCameraInvV;
    float4x4 gCameraInvP;
    float4x4 gCameraInvVP;

    float3   gCameraPos;
	float    gCameraPadding;

	float    gCameraNearZ;
	float    gCameraFarZ;
	float    gCameraInvNearZ;
	float    gCameraInvFarZ;
};

struct MaterialCB
{
    float4 mBaseColor;
};

#endif
