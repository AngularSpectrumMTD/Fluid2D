struct fluidCB
{
    float2 mouseXY;
    float2 velocityXY;
    float2 bufferWH;
    float deltaTime;
};

SamplerState imageSampler : register(s0);
ConstantBuffer<fluidCB> fluidConstantBuffer : register(b0);
Texture2D<float2> sourceVelocity : register(t0);
RWTexture2D<float2> destVelocity : register(u0);

[numthreads(8, 8, 1)]
void applyExternalForce(uint2 dtid : SV_DispatchThreadID)
{
    const float2 uv = (dtid + 0.5.xx) / fluidConstantBuffer.bufferWH;
    
    float2 updVelocity = sourceVelocity.SampleLevel(imageSampler, uv, 0);
    float power = length(fluidConstantBuffer.mouseXY / fluidConstantBuffer.bufferWH - uv);

    if (power < 0.01)
    {
        updVelocity += fluidConstantBuffer.velocityXY / fluidConstantBuffer.bufferWH;
    }

    destVelocity[dtid] = updVelocity;
}