// @
float4 main(float4 vOut : SV_Position) : SV_Target
{
   return float4(vOut.x / 1080.f, vOut.y / 720.f, 0.f, 1.f);
}