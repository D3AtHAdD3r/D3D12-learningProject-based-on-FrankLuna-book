#include"D3DApp.h"
#include"MathHelper.h"
#include"UploadBuffer.h"
#include <memory>

struct Vertex1
{
	DirectX::XMFLOAT3 Pos;
	
};
struct Vertex2
{
	DirectX::XMFLOAT4 Color;
};


struct ObjectConstants
{
	DirectX::XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};


class BoxApp : public D3DApp
{
public:
	BoxApp(HINSTANCE hInstance);
	BoxApp(const BoxApp& rhs) = delete;
	BoxApp& operator=(const BoxApp& rhs) = delete;
	~BoxApp();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

private:
	void BuildDescriptorHeaps(); //Creates descriptor heap for constant-buffer-view
	void BuildConstantBuffers(); //creates Constan-Buffer-View
	void BuildRootSignature();
	void BuildShaderAndInputLayout();
	void BuildboxGeometry();
	void BuildPSO();

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;
	std::unique_ptr<MeshGeometry> mBoxGeo1 = nullptr;
	std::unique_ptr<MeshGeometry> mBoxGeo2 = nullptr;

	Microsoft::WRL::ComPtr<ID3D10Blob> mvsByteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3D10Blob> mpsByteCode = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;

	DirectX::XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.5f * DirectX::XM_PI;
	float mPhi = DirectX::XM_PIDIV4;
	float mRadius = 5.0f;

	POINT mLastMousePos;
};