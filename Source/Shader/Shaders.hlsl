cbuffer cbBuffer : register(b0){
    float4x4 WVP;
};

struct VS_INPUT{
    float3 Position : POSITION;
    float4 Color    : COLOR;
};

// (VS_OUTPUT)
struct PS_INPUT{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
};


PS_INPUT VSMain(VS_INPUT input){
    PS_INPUT output;

    //float4 pos4 = float4(input.Position, 1.0);
    //output.Position = mul(pos4, WVP);
	output.Position = float4(input.Position, 1.0);
    output.Color    = input.Color;

    return output;
}


float4 PSMain(PS_INPUT input) : SV_TARGET{
    return input.Color;
}