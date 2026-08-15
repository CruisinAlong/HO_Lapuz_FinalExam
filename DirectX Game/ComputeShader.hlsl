cbuffer CBPerObject : register(b0)
{
    row_major float4x4 view;
    row_major float4x4 projection;
    float time;
    float padding[3];
};

// Per-vertex input
struct VS_INPUT
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
    // Per-instance world matrix (4 x float4), supplied from input slot 1
    float4 instanceRow0 : INSTANCE0;
    float4 instanceRow1 : INSTANCE1;
    float4 instanceRow2 : INSTANCE2;
    float4 instanceRow3 : INSTANCE3;
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

    // Reconstruct world matrix from instance rows (row-major arrangement)
    float4x4 instanceWorld = float4x4(
        input.instanceRow0,
        input.instanceRow1,
        input.instanceRow2,
        input.instanceRow3
    );

    float4 worldPos = mul(pos, instanceWorld); // pos * world (row-major)
    float4 viewPos = mul(worldPos, view);
    output.position = mul(viewPos, projection);
    output.texcoord = input.texcoord;
    return output;
}