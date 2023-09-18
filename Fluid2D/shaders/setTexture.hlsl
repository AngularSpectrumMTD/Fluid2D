struct fluidCB
{
    float2 mouseXY;
    float2 velocityXY;
    float2 bufferWH;
    float deltaTime;
};

SamplerState imageSampler : register(s0);
ConstantBuffer<fluidCB> fluidConstantBuffer : register(b0);
Texture2D<float4> sourceTexture : register(t0);
RWTexture2D<float4> destTexture : register(u0);

[numthreads(8, 8, 1)]
void setTexture(uint2 dtid : SV_DispatchThreadID)
{
    const float2 uv = (dtid + 0.5.xx) / fluidConstantBuffer.bufferWH;
    
    destTexture[dtid] = sourceTexture.SampleLevel(imageSampler, uv, 0);
}