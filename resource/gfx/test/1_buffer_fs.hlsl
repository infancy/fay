#include "test_render_struct.hlsli"

// @
float4 main(VertexOut vOut) : SV_TARGET
{
   return float4(vOut.rPos.x / 1080, vOut.rPos.y / 720, 0.f, 1.f);
}