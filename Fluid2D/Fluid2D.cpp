#include "Fluid2D.h"

#include "AppInvoker.h"

#include <fstream>
#include <DirectXTex.h>

#include <wincodec.h>
#include "utility/Utility.h"

#include <iostream>
#include <string>
#include <sstream>

using namespace DirectX;

//This Program is only support TRIANGULAR POLYGON
//If u see beautiful caustics, polygon normal must be smooth!!!
Fluid2D::Fluid2D(u32 width, u32 height) : AppBase(width, height, L"Fluid2D")
{
   WCHAR assetsPath[512];
   GetAssetsPath(assetsPath, _countof(assetsPath));
   mAssetPath = assetsPath;
}

void Fluid2D::FixedDispatch()
{
    mCommandList->Dispatch(GetWidth() / 8, GetHeight() / 8, 1);
}

void valueSwap(u32& a, u32& b)
{
    u32 tmp = a;
    a = b;
    b = tmp;
}

void Fluid2D::UpdateState(const D3D12_RESOURCE_STATES dst, const FluidTerm type, const u32 idx)
{
    switch (type)
    {
        case FluidTerm_Result : 
        {
            if (dst != mResultStateTbl[idx])
            {
                auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mResultBufferTbl[idx].Get(), mResultStateTbl[idx], dst);
                mCommandList->ResourceBarrier(1, &barrier);
                mResultStateTbl[idx] = dst;
            }
        }
        break;
        case FluidTerm_Velocity:
        {
            if (dst != mVelocityStateTbl[idx])
            {
                auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mVelocityBufferTbl[idx].Get(), mVelocityStateTbl[idx], dst);
                mCommandList->ResourceBarrier(1, &barrier);
                mVelocityStateTbl[idx] = dst;
            }
        }
        break;
        case FluidTerm_Pressure:
        {
            if (dst != mPressureStateTbl[idx])
            {
                auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mPressureBufferTbl[idx].Get(), mPressureStateTbl[idx], dst);
                mCommandList->ResourceBarrier(1, &barrier);
                mPressureStateTbl[idx] = dst;
            }
        }
        break;
        case FluidTerm_Divergence:
        {
            if (dst != mDivergenceState)
            {
                auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(mDivergenceBuffer.Get(), mDivergenceState, dst);
                mCommandList->ResourceBarrier(1, &barrier);
                mDivergenceState = dst;
            }
        }
        break;
    }
}

void Fluid2D::Initialize()
{
    if (!InitializeRenderDevice(AppInvoker::GetHWND()))
    {
        throw std::runtime_error("Failed Initialize RenderDevice.");
    }
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if ((hr == S_OK) || (hr == S_FALSE))
    {
        CreateRegularBuffer();
        CreateConstantBuffer();
        CreateComputeRootSignatureAndPSO();

        mCommandList = mDevice->CreateCommandList();
        mCommandList->Close();

        mBaseTex = LoadTextureFromFile(L"ParisEquirec.png");
    }
    else
    {
        throw std::runtime_error("Failed CoInitializeEx.");
    }
}

void Fluid2D::Terminate()
{
    TerminateRenderDevice();
}

void Fluid2D::UpdateWindowText()
{
    std::wstringstream windowText;
    windowText.str(L"");
    windowText << L" x : " << mMousePosX << L" y : " << mMousePosY << L"button R : Reset Image";

    std::wstring finalWindowText = std::wstring(GetTitle()) + windowText.str().c_str();
    SetWindowText(AppInvoker::GetHWND(), finalWindowText.c_str());
}

void Fluid2D::Update()
{
    mSceneParam.mouseXY.x = mMousePosX;
    mSceneParam.mouseXY.y = mMousePosY;
    mSceneParam.bufferWH.x = GetWidth();
    mSceneParam.bufferWH.y = GetHeight();
    mSceneParam.deltaTime += 1 / 6000.f;
    mSceneParam.velocityXY.x = mPower * mMoveVecX;
    mSceneParam.velocityXY.y = mPower * mMoveVecY;

    UpdateWindowText();
}

void Fluid2D::OnKeyDown(UINT8 wparam)
{
    switch (wparam)
    {
    case 'R':
        isTextureReset = true;
        break;
    case 'P':
        mPower = Clamp(100, 1000, mPower + 100);
        break;
    case 'N':
        mPower = Clamp(100, 1000, mPower - 100);
        break;
    }
}

void Fluid2D::OnMouseDown(MouseButton button, s32 x, s32 y)
{
    mMousePosX = x;
    mMousePosY = y;
}

void Fluid2D::OnMouseUp(MouseButton button, s32 x, s32 y)
{
}

void Fluid2D::OnMouseMove(s32 dx, s32 dy)
{
    f32 fdx = f32(dx) / GetWidth();
    f32 fdy = f32(dy) / GetHeight();
    mMoveVecX = fdx;
    mMoveVecY = fdy;
}

void Fluid2D::OnMouseWheel(s32 rotate)
{
}

f32 Fluid2D::Clamp(f32 min, f32 max, f32 src)
{
    return std::fmax(min, std::fmin(src, max));
}

void Fluid2D::UpdateSceneParams()
{
    auto sceneConstantBuffer = mSceneCB.Get();
    mDevice->ImmediateBufferUpdateHostVisible(sceneConstantBuffer, &mSceneParam, sizeof(mSceneParam));
}

void Fluid2D::Draw()
{
    QueryPerformanceFrequency(&mCpuFreq);

    auto renderTarget = mDevice->GetRenderTarget();
    auto allocator = mDevice->GetCurrentCommandAllocator();
    allocator->Reset();
    mCommandList->Reset(allocator.Get(), nullptr);
    auto frameIndex = mDevice->GetCurrentFrameIndex();

    UpdateSceneParams();

    ID3D12DescriptorHeap* descriptorHeaps[] = {
        mDevice->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).Get(),
    };
    mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    u32 ping = (mRenderFrame + 1) % 2;
    u32 pong = (mRenderFrame + 0) % 2;

    if (mRenderFrame == 0 || isTextureReset)
    {
        UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Result, 0);
        UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Result, 1);
        UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Velocity, 0);
        UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Velocity, 1);
        UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Pressure, 0);
        UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Pressure, 1);

        mCommandList->SetComputeRootSignature(mRsSetTexture.Get());
        mCommandList->SetComputeRootConstantBufferView(0, mSceneCB.Get()->GetGPUVirtualAddress());
        mCommandList->SetComputeRootDescriptorTable(1, mBaseTex.srv.hGpu);
        mCommandList->SetComputeRootDescriptorTable(2, mResultUAVTbl[0].hGpu);
        mCommandList->SetPipelineState(mSetTexturePSO.Get());
        FixedDispatch();

        mCommandList->SetComputeRootSignature(mRsSetTexture.Get());
        mCommandList->SetComputeRootConstantBufferView(0, mSceneCB.Get()->GetGPUVirtualAddress());
        mCommandList->SetComputeRootDescriptorTable(1, mBaseTex.srv.hGpu);
        mCommandList->SetComputeRootDescriptorTable(2, mResultUAVTbl[1].hGpu);
        mCommandList->SetPipelineState(mSetTexturePSO.Get());
        FixedDispatch();

        mCommandList->SetComputeRootSignature(mRsClearFloat2.Get());
        mCommandList->SetComputeRootDescriptorTable(0, mVelocityUAVTbl[0].hGpu);
        mCommandList->SetPipelineState(mClearFloat2PSO.Get());
        FixedDispatch();

        mCommandList->SetComputeRootSignature(mRsClearFloat2.Get());
        mCommandList->SetComputeRootDescriptorTable(0, mVelocityUAVTbl[1].hGpu);
        mCommandList->SetPipelineState(mClearFloat2PSO.Get());
        FixedDispatch();

        mCommandList->SetComputeRootSignature(mRsClearFloat.Get());
        mCommandList->SetComputeRootDescriptorTable(0, mPressureUAVTbl[0].hGpu);
        mCommandList->SetPipelineState(mClearFloatPSO.Get());
        FixedDispatch();

        mCommandList->SetComputeRootSignature(mRsClearFloat.Get());
        mCommandList->SetComputeRootDescriptorTable(0, mPressureUAVTbl[1].hGpu);
        mCommandList->SetPipelineState(mClearFloatPSO.Get());
        FixedDispatch();

        isTextureReset = false;
        mSceneParam.deltaTime = 0;
    }
    else
    {
          UpdateState(D3D12_RESOURCE_STATE_COPY_SOURCE, FluidTerm_Velocity, ping);
          UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Velocity, pong);

          mCommandList->SetComputeRootSignature(mRsUpdateAdvection.Get());
          mCommandList->SetComputeRootConstantBufferView(0, mSceneCB.Get()->GetGPUVirtualAddress());
          mCommandList->SetComputeRootDescriptorTable(1, mVelocitySRVTbl[ping].hGpu);
          mCommandList->SetComputeRootDescriptorTable(2, mVelocityUAVTbl[pong].hGpu);
          mCommandList->SetPipelineState(mUpdateAdvectionPSO.Get());
          FixedDispatch();

          UpdateState(D3D12_RESOURCE_STATE_COPY_SOURCE, FluidTerm_Velocity, pong);
          UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Velocity, ping);

          mCommandList->SetComputeRootSignature(mRsApplyExternalForce.Get());
          mCommandList->SetComputeRootConstantBufferView(0, mSceneCB.Get()->GetGPUVirtualAddress());
          mCommandList->SetComputeRootDescriptorTable(1, mVelocitySRVTbl[pong].hGpu);
          mCommandList->SetComputeRootDescriptorTable(2, mVelocityUAVTbl[ping].hGpu);
          mCommandList->SetPipelineState(mApplyExternalForcePSO.Get());
          FixedDispatch();

          UpdateState(D3D12_RESOURCE_STATE_COPY_SOURCE, FluidTerm_Velocity, ping);
          UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Divergence);

          mCommandList->SetComputeRootSignature(mRsGenerateDivergence.Get());
          mCommandList->SetComputeRootConstantBufferView(0, mSceneCB.Get()->GetGPUVirtualAddress());
          mCommandList->SetComputeRootDescriptorTable(1, mVelocitySRVTbl[ping].hGpu);
          mCommandList->SetComputeRootDescriptorTable(2, mDivergenceUAV.hGpu);
          mCommandList->SetPipelineState(mGenerateDivergencePSO.Get());
          FixedDispatch();

          //poisson
          for (u32 i = 0; i < POISSON_LOOP; i++)
          {
              UpdateState(D3D12_RESOURCE_STATE_COPY_SOURCE, FluidTerm_Pressure, ping);
              UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Pressure, pong);
              UpdateState(D3D12_RESOURCE_STATE_COPY_SOURCE, FluidTerm_Divergence);

              mCommandList->SetComputeRootSignature(mRsUpdatePressure.Get());
              mCommandList->SetComputeRootConstantBufferView(0, mSceneCB.Get()->GetGPUVirtualAddress());
              mCommandList->SetComputeRootDescriptorTable(1, mPressureSRVTbl[ping].hGpu);
              mCommandList->SetComputeRootDescriptorTable(2, mDivergenceSRV.hGpu);
              mCommandList->SetComputeRootDescriptorTable(3, mPressureUAVTbl[pong].hGpu);
              mCommandList->SetPipelineState(mUpdatePressurePSO.Get());
              FixedDispatch();

              valueSwap(ping, pong);
          }

          UpdateState(D3D12_RESOURCE_STATE_COPY_SOURCE, FluidTerm_Pressure, ping);
          UpdateState(D3D12_RESOURCE_STATE_COPY_SOURCE, FluidTerm_Velocity, ping);
          UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Velocity, pong);

          mCommandList->SetComputeRootSignature(mRsUpdateVelocity.Get());
          mCommandList->SetComputeRootConstantBufferView(0, mSceneCB.Get()->GetGPUVirtualAddress());
          mCommandList->SetComputeRootDescriptorTable(1, mPressureSRVTbl[ping].hGpu);
          mCommandList->SetComputeRootDescriptorTable(2, mVelocitySRVTbl[ping].hGpu);
          mCommandList->SetComputeRootDescriptorTable(3, mVelocityUAVTbl[pong].hGpu);
          mCommandList->SetPipelineState(mUpdateVelocityPSO.Get());
          FixedDispatch();

          UpdateState(D3D12_RESOURCE_STATE_COPY_SOURCE, FluidTerm_Result, ping);
          UpdateState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS, FluidTerm_Result, pong);
          UpdateState(D3D12_RESOURCE_STATE_COPY_SOURCE, FluidTerm_Velocity, pong);

          mCommandList->SetComputeRootSignature(mRsUpdateResult.Get());
          mCommandList->SetComputeRootConstantBufferView(0, mSceneCB.Get()->GetGPUVirtualAddress());
          mCommandList->SetComputeRootDescriptorTable(1, mResultSRVTbl[ping].hGpu);
          mCommandList->SetComputeRootDescriptorTable(2, mVelocitySRVTbl[pong].hGpu);
          mCommandList->SetComputeRootDescriptorTable(3, mResultUAVTbl[pong].hGpu);
          mCommandList->SetPipelineState(mUpdateResultPSO.Get());
          FixedDispatch();
    }
    
    UpdateState(D3D12_RESOURCE_STATE_COPY_SOURCE, FluidTerm_Result, pong);
    mCommandList->CopyResource(renderTarget.Get(), mResultBufferTbl[pong].Get());

    auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
    mCommandList->ResourceBarrier(1, &barrierToRT);

    const auto& rtv = mDevice->GetRenderTargetView();
    const auto& viewport = mDevice->GetDefaultViewport();
    mCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    mCommandList->RSSetViewports(1, &viewport);

    auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(renderTarget.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    mCommandList->ResourceBarrier(1, &barrierToPresent);

    mCommandList->Close();

    QueryPerformanceCounter(&mStartTime);
    mDevice->ExecuteCommandList(mCommandList);
    mDevice->Present(1);
    QueryPerformanceCounter(&mEndTime);

    mRenderFrame++;
}

f32 Fluid2D::getFrameRate()
{
    return 1000.0f * (mEndTime.QuadPart - mStartTime.QuadPart) / mCpuFreq.QuadPart;
}