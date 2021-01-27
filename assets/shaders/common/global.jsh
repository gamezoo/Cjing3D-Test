#ifndef GLOBAL_JSH
#define GLOBAL_JSH

// Creates a full screen triangle from 3 vertices
void FullScreenTriangle(in uint vertexID, out float4 pos)
{
	pos.x = (float)(vertexID / 2) * 4.0 - 1.0;
	pos.y = (float)(vertexID % 2) * 4.0 - 1.0;
	pos.z = 0;
	pos.w = 1;
}

void FullScreenTriangle(in uint vertexID, out float4 pos, out float2 uv)
{
	FullScreenTriangle(vertexID, pos);

	uv.x = (float)(vertexID / 2) * 2;
	uv.y = 1 - (float)(vertexID % 2) * 2;
}

#endif
