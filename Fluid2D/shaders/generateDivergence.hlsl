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
RWTexture2D<float> divergence : register(u0);

[numthreads(8, 8, 1)]
void generateDivergence(uint2 dtid : SV_DispatchThreadID)
{
    const float2 uv = (dtid + 0.5.xx) / fluidConstantBuffer.bufferWH;

    float velocity_nx_x = sourceVelocity.SampleLevel(imageSampler, uv - float2(1 / fluidConstantBuffer.bufferWH.x, 0), 0).x;
    float velocity_px_x = sourceVelocity.SampleLevel(imageSampler, uv + float2(1 / fluidConstantBuffer.bufferWH.x, 0), 0).x;
    float velocity_ny_y = sourceVelocity.SampleLevel(imageSampler, uv - float2(0, 1 / fluidConstantBuffer.bufferWH.y), 0).y;
    float velocity_py_y = sourceVelocity.SampleLevel(imageSampler, uv + float2(0, 1 / fluidConstantBuffer.bufferWH.y), 0).y;

    divergence[dtid] = velocity_px_x - velocity_nx_x + velocity_py_y - velocity_ny_y;
}