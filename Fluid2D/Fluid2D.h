#pragma once

#include "AppBase.h"
#include "utility/Utility.h"
#include <DirectXMath.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define POISSON_LOOP 30

#include <d3d12.h>
#include <dxgi1_6.h>

#include "d3dx12.h"
#include <pix3.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace ComputeShaders {
    const LPCWSTR ClearFloat = L"clearFloat.cso";
    const LPCWSTR ClearFloat2 = L"clearFloat2.cso";
    const LPCWSTR ApplyExternalForce = L"applyExternalForce.cso";
    const LPCWSTR GenerateDivergence = L"generateDivergence.cso";
    const LPCWSTR SetTexture = L"setTexture.cso";
    const LPCWSTR UpdateAdvection = L"updateAdvection.cso";
    const LPCWSTR UpdatePressure = L"updatePressure.cso";
    const LPCWSTR UpdateResult = L"updateResult.cso";
    const LPCWSTR UpdateVelocity = L"updateVelocity.cso";
}

template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class Fluid2D : public AppBase {
public:
    Fluid2D(u32 width, u32 height);

    void Initialize() override;
    void Terminate() override;

    void Update() override;
    void Draw() override;

    void OnMouseDown(MouseButton button, s32 x, s32 y) override;
    void OnMouseUp(MouseButton button, s32 x, s32 y) override;
    void OnMouseMove(s32 dx, s32 dy) override;
    void OnKeyDown(UINT8 wparam) override;
    void OnMouseWheel(s32 rotate) override;

private:

    struct SceneParam
    {
        XMFLOAT2 mouseXY;
        XMFLOAT2 velocityXY;
        XMFLOAT2 bufferWH;
        f32 deltaTime;
    };

    enum FluidTerm
    {
        FluidTerm_Result,
        FluidTerm_Velocity,
        FluidTerm_Pressure,
        FluidTerm_Divergence
    };

    void FixedDispatch(const std::wstring cmdName);

    void CreateComputeRootSignatureAndPSO();
    void CreateComputeShaderStateObject(const LPCWSTR& compiledComputeShaderName, ComPtr<ID3D12PipelineState>& computePipelineState, ComPtr<ID3D12RootSignature> rootSig);
    void CreateResultBuffer();
    void CreateRegularBuffer();
    void CreateConstantBuffer();

    void UpdateSceneParams();
    void UpdateWindowText();

    void UpdateState(const D3D12_RESOURCE_STATES dst, const FluidTerm type, const u32 idx = 0);

    void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, u32 pathSize);
    std::wstring GetAssetFullPath(LPCWSTR assetName);
    f32 Clamp(f32 min, f32 max, f32 src);
    f32 getFrameRate();
    utility::TextureResource LoadTextureFromFile(const std::wstring& fileName);

    ComPtr<ID3D12GraphicsCommandList4> mCommandList;
    static const u32 BackBufferCount = dx12::RenderDeviceDX12::BackBufferCount;

    SceneParam mSceneParam;
    utility::TextureResource mBaseTex;

    //Buffers
    //float4
    ComPtr <ID3D12Resource> mResultBufferTbl[2];
    dx12::Descriptor mResultUAVTbl[2];
    dx12::Descriptor mResultSRVTbl[2];

    //float2
    ComPtr <ID3D12Resource> mVelocityBufferTbl[2];
    dx12::Descriptor mVelocityUAVTbl[2];
    dx12::Descriptor mVelocitySRVTbl[2];

    //float
    ComPtr <ID3D12Resource> mPressureBufferTbl[2];
    dx12::Descriptor mPressureUAVTbl[2];
    dx12::Descriptor mPressureSRVTbl[2];

    //float
    ComPtr <ID3D12Resource> mDivergenceBuffer;
    dx12::Descriptor mDivergenceUAV;
    dx12::Descriptor mDivergenceSRV;

    //State Checker 
    D3D12_RESOURCE_STATES mResultStateTbl[2];
    D3D12_RESOURCE_STATES mVelocityStateTbl[2];
    D3D12_RESOURCE_STATES mPressureStateTbl[2];
    D3D12_RESOURCE_STATES mDivergenceState;

    //ConstantBuffers
    ComPtr<ID3D12Resource> mSceneCB;

    //Pipeline State
    ComPtr<ID3D12RootSignature> mRsClearFloat;
    ComPtr<ID3D12PipelineState> mClearFloatPSO;

    ComPtr<ID3D12RootSignature> mRsClearFloat2;
    ComPtr<ID3D12PipelineState> mClearFloat2PSO;

    ComPtr<ID3D12RootSignature> mRsApplyExternalForce;
    ComPtr<ID3D12PipelineState> mApplyExternalForcePSO;

    ComPtr<ID3D12RootSignature> mRsGenerateDivergence;
    ComPtr<ID3D12PipelineState> mGenerateDivergencePSO;

    ComPtr<ID3D12RootSignature> mRsSetTexture;
    ComPtr<ID3D12PipelineState> mSetTexturePSO;

    ComPtr<ID3D12RootSignature> mRsUpdateAdvection;
    ComPtr<ID3D12PipelineState> mUpdateAdvectionPSO;

    ComPtr<ID3D12RootSignature> mRsUpdatePressure;
    ComPtr<ID3D12PipelineState> mUpdatePressurePSO;

    ComPtr<ID3D12RootSignature> mRsUpdateResult;
    ComPtr<ID3D12PipelineState> mUpdateResultPSO;

    ComPtr<ID3D12RootSignature> mRsUpdateVelocity;
    ComPtr<ID3D12PipelineState> mUpdateVelocityPSO;

    std::wstring mAssetPath;

    LARGE_INTEGER mCpuFreq;
    LARGE_INTEGER mStartTime;
    LARGE_INTEGER mEndTime;

    f32 mMousePosX = 300;
    f32 mMousePosY = 300;

    f32 mMoveVecX = 0;
    f32 mMoveVecY = 0;

    u32 mRenderFrame = 0;

    f32 mPower = 300;

    bool isTextureReset = false;
};
