#include "render_struct.hlsli"

Texture2D gTex : register(t0);
SamplerState gSampler : register(s0);

#include "render_func.hlsli"

float4 main(VertexOut vOut) : SV_TARGET
{
   return gTex.Sample(gSampler, vOut.rTex);
}