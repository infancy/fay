float4 vs_main(float3 Position : POSITION) : SV_POSITION
{
	return float4(Position, 1.0);
}

float4 ps_main(float4 Position : SV_POSITION) : SV_TARGET
{
	return float4(1, 0, 0, 1);
}