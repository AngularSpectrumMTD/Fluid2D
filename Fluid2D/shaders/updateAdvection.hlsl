struct fluidCB
{
    float2 mouseXY;
    float2 velocityXY;
    float2 bufferWH;
    float deltaTime;
};

SamplerState imageSampler : register(s0);
ConstantBuffer<fluidCB> fluidConstantBuffer : register(b0);
Texture2D<float2> updateVelocity : register(t0);
RWTexture2D<float2> destVelocity : register(u0);

[numthreads(8, 8, 1)]
void updateAdvection(uint2 dtid : SV_DispatchThreadID)
{  
    const float2 uv = (dtid + 0.5.xx) / fluidConstantBuffer.bufferWH;
    
    float2 updVelocity = updateVelocity.SampleLevel(imageSampler, uv, 0);
    float2 resultVelocity = updateVelocity.SampleLevel(imageSampler, uv - updVelocity * fluidConstantBuffer.deltaTime, 0);

    destVelocity[dtid] = resultVelocity;
}