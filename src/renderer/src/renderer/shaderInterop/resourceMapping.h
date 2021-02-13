#ifndef SHADER_RESOURCE_MAPPING_H
#define SHADER_RESOURCE_MAPPING_H

////////////////////////////////////////////////////////////
// TEXTURE
////////////////////////////////////
// SYSTEM TEXTURES

////////////////////////////////////
// ONDEMEND TEXTURES
#define SLOT_TEX_ONDEMEND_1			0
#define SLOT_TEX_ONDEMEND_2			1
#define SLOT_TEX_ONDEMEND_3			2

////////////////////////////////////////////////////////////
// RENDERER
#define SLOT_TEX_RENDERER_BASECOLORMAP SLOT_TEX_ONDEMEND_1
#define SLOT_TEX_RENDERER_NORMALMAP    SLOT_TEX_ONDEMEND_2
#define SLOT_TEX_RENDERER_SURFACEMAP   SLOT_TEX_ONDEMEND_3

////////////////////////////////////////////////////////////
// IMAGE
#define SLOT_TEX_IMAGE_BASE SLOT_TEX_ONDEMEND_1

#endif