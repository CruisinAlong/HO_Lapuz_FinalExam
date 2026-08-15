struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

Texture2D texDiffuse : register(t0);
SamplerState sampLinear : register(s0);

float4 psmain(PS_INPUT input) : SV_Target
{
    return texDiffuse.Sample(sampLinear, input.texcoord*0.5);
}