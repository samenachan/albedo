cbuffer scene : register(b0){
    float4x4    ViewMatrix;
    float4x4    ProjectionMatrix;
    float4      ViewPosition;
    float4      LightPosition;
    float4      LightAmbient;
    float4x4    LightViewMatrix;
    float4x4    LightProjectionMatrix;
    float       LightIntensity;
    bool        IsEnabledShadowMapping;
    float       ShadowMappingBias;
};

cbuffer local : register(b1){
    float4x4    WorldMatrix;
    float       Specular;
};

Texture2D<float4> Texture0      : register(t0);
Texture2D<float4> ShadowMap     : register(t1);
TextureCube       CubeMap       : register(t2);
SamplerState      Sampler0      : register(s0);
SamplerState      Sampler1      : register(s1);

struct VS_INPUT{
    float3 Position : POSITION0;
    float4 Color    : COLOR0;
	float3 Normal   : NORMAL0;
    float2 UV       : TEXCOORD0;
};

struct PS_INPUT{
    float4 Position         : SV_POSITION;
    float4 Color            : COLOR0;
	float3 Normal	        : NORMAL0;
    float2 UV               : TEXCOORD0;
    float4 PositionWorld    : POSITION_W;
    float4 ShadowCoord	    : POSITION_SM;
    float3 Reflect          : COLOR1;
};

PS_INPUT VSMain(VS_INPUT input){
    PS_INPUT output;
    
    float4x4 wvp = mul(ProjectionMatrix, mul(ViewMatrix, WorldMatrix));
    float4 pos4 = float4(input.Position, 1.0);

	output.Position = mul(wvp, pos4);
    output.Color = input.Color;
	output.Normal = mul(WorldMatrix, input.Normal);
    output.UV = input.UV;
    output.PositionWorld = mul(WorldMatrix, pos4);

    float4 pos_shadow = float4(input.Position, 1.0);
    pos_shadow = mul(WorldMatrix, pos_shadow);
    pos_shadow = mul(LightViewMatrix, pos_shadow);
    pos_shadow = mul(LightProjectionMatrix, pos_shadow);

    output.ShadowCoord.x = (1.0 + pos_shadow.x) / 2.0f;
	output.ShadowCoord.y = (1.0 - pos_shadow.y) / 2.0f;
	output.ShadowCoord.z = pos_shadow.z;

    //float3 eye_dir = normalize(input.Position.xyz - ViewPosition.xyz);
    //output.Reflect = reflect(eye_dir, -input.Normal.xyz);
    output.Reflect = input.Position;

    return output;
}

float4 PSMain(PS_INPUT input) : SV_TARGET{

    float3 V = normalize(ViewPosition - input.PositionWorld.xyz);
    float3 L = normalize(LightPosition - input.PositionWorld.xyz);
    float3 H = normalize(V + L);
    
    float4 ambient = LightAmbient;
    float diffuse = clamp(dot(input.Normal, L), 0.0, 1.0);
    float specular = pow(clamp(dot(input.Normal, H), 0.0, 1.0), Specular);

    float4 surf_color = float4(diffuse, diffuse, diffuse, 1.0) + float4(specular, specular, specular, 1.0);
    float4 tex_color = Texture0.Sample(Sampler0, input.UV);
    surf_color *= LightIntensity;
    surf_color *= tex_color;
    surf_color += ambient;
    
    float shadow_map = ShadowMap.Sample(Sampler1, input.ShadowCoord.xy);
    float shadow_alpha = (shadow_map > input.ShadowCoord.z - ShadowMappingBias ) ? 1.0f : 0.5f;
    surf_color = IsEnabledShadowMapping ? surf_color * shadow_alpha : surf_color;

    //float4 cube_map_color = CubeMap.Sample(Sampler0, input.Reflect);
    //return cube_map_color;

    return surf_color;
}