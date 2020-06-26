static float2 positions[6] =
{
	float2(0.0, 0.0),
	float2(0.5, 1.0),
	float2(1.0, 0.5),
	
	float2( 0.0,  0.0),
	float2(-1.0, -0.5),
	float2(-0.5, -1.0),
};

float4 vs_main(uint vid : SV_VertexID) : SV_Position
{
	return float4(positions[vid], 0.0, 1.0);
}

float4 ps_main(float4 vOut : SV_Position) : SV_Target
{
   return float4(vOut.x / 1080.f, vOut.y / 720.f, 0.f, 1.f);
}