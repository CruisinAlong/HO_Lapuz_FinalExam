cbuffer CBPerObject : register(b0)
{
    row_major float4x4 world;
    row_major float4x4 view;
    row_major float4x4 projection;
    float time;
    float padding[3];
};

struct VS_INPUT
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

VS_OUTPUT vsmain(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 pos = float4(input.position, 1.0f);
    float4 worldPos = mul(pos, world);
    float4 viewPos = mul(worldPos, view);
    output.position = mul(viewPos, projection);
    output.texcoord = input.texcoord;
    return output;
}
