#ifndef SAMPLERS_JSH
#define SAMPLERS_JSH

#include "shaderInterop/samplerMapping.h"

[static]
[register(SLOT_SS_LINEAR_CLAMP)]
SamplerState SS_LINEAR_CLAMP = 
{
    .mFilter = FILTER_MIN_MAG_MIP_LINEAR,
	.mAddressU = TEXTURE_ADDRESS_CLAMP,
	.mAddressV = TEXTURE_ADDRESS_CLAMP,
	.mAddressW = TEXTURE_ADDRESS_CLAMP,
};

[static]
[register(SLOT_SS_LINEAR_WARP)]
SamplerState SS_LINEAR_WARP = 
{
    .mFilter = FILTER_MIN_MAG_MIP_LINEAR,
	.mAddressU = TEXTURE_ADDRESS_WRAP,
	.mAddressV = TEXTURE_ADDRESS_WRAP,
	.mAddressW = TEXTURE_ADDRESS_WRAP,
};


#endif