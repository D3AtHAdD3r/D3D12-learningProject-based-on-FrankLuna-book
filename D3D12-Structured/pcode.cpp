#include<d3d12.h>
#include<directxmath.h>
#include<wrl.h>
#include"d3dUtil.h"
#include"d3dx12.h"
#include"pcode.h"
#include <memory>
#include<d3dcompiler.h>
#include<fstream>
#include<sstream>
using namespace DirectX;

Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice = nullptr;
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList = nullptr;
//-----------Input layout-------------//
struct Vertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

D3D12_INPUT_ELEMENT_DESC desc1[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};

struct Vertex2
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 Tex0;
	DirectX::XMFLOAT2 Tex1;
};


D3D12_INPUT_ELEMENT_DESC desc2[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
};

//--------------Vertex Buffer-------------------//

//Utility function to create Default Buffer
Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer
)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

	// Create the actual default buffer resource.
	D3D12_HEAP_PROPERTIES heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
	ThrowIfFailed(device->CreateCommittedResource(
		&heap_properties,
		D3D12_HEAP_FLAG_NONE,
		&res_desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())
	));

	// In order to copy CPU memory data into our default buffer, we need
	// to create an intermediate upload heap.
	D3D12_HEAP_PROPERTIES heap_properties_2 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	ThrowIfFailed(device->CreateCommittedResource(
		&heap_properties_2,
		D3D12_HEAP_FLAG_NONE,
		&res_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())
	));

	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.
	// At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.
	// Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	D3D12_RESOURCE_BARRIER res_barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->ResourceBarrier(1, &res_barrier);

	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(),
		0, 0, 1, &subResourceData);

	D3D12_RESOURCE_BARRIER res_barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_GENERIC_READ);
	cmdList->ResourceBarrier(1, &res_barrier2);

	// Note: uploadBuffer has to be kept alive after the above function
	// calls because the command list has not been executed yet that
	// performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy
	// has been executed.
	return defaultBuffer;
}


void CreateVertBuffer()
{
	Vertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) }
	};

	const UINT64 vbByteSize = 8 * sizeof(Vertex);

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	VertexBufferGPU = CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices, vbByteSize, VertexBufferUploader);

	//create a vertex buffer view to the vertex buffer resource and bind to Pipeline's IA stage
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
	vbv.StrideInBytes = sizeof(Vertex);
	vbv.SizeInBytes = 8 * sizeof(Vertex);

	D3D12_VERTEX_BUFFER_VIEW vertexBuffers[1] = { vbv };
	mCommandList->IASetVertexBuffers(0, 1, vertexBuffers);
}



void CreateIndexBuffer() {
	std::uint16_t indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,
		// back face
		4, 6, 5,
		4, 7, 6,
		// left face
		4, 5, 1,
		4, 1, 0,
		// right face
		3, 2, 6,
		3, 6, 7,
		// top face
		1, 5, 6,
		1, 6, 2,
		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	const UINT ibByteSize = 36 * sizeof(std::uint16_t);

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;
	IndexBufferGPU = CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices,
		ibByteSize, IndexBufferUploader);

	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = DXGI_FORMAT_R16_UINT;
	ibv.SizeInBytes = ibByteSize;

	mCommandList->IASetIndexBuffer(&ibv);
}

//------------Constant Buffers---------------------//

struct ObjectConstants {
	DirectX::XMFLOAT4X4 WorldViewProj;
};


void CreateConstBuffer() {
	UINT NumElements = 1;
	UINT elementByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));
	Microsoft::WRL::ComPtr<ID3D12Resource> mUploadCBuffer;
	D3D12_HEAP_PROPERTIES heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC res_desc = CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * NumElements);
	md3dDevice->CreateCommittedResource(&heap_props,
		D3D12_HEAP_FLAG_NONE,
		&res_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mUploadCBuffer));
}


void updateConstBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> mUploadCBuffer, BYTE* data, UINT dataSizeInBytes) {
	BYTE* mMappedData = nullptr;
	mUploadCBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData));
	memcpy(mMappedData, data, dataSizeInBytes);
	mUploadCBuffer->Unmap(0, nullptr);
	mMappedData = nullptr;
}

void bindConstBuffer(){
	//descriptorheap to create view
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap;
	md3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&mCbvHeap));

	//create unique pointer to UploadBuffer
	std::unique_ptr<UploadBuffer<ObjectConstants>> mObjectCB = nullptr;
	int n = 1;
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), n, true);

	UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));

	// Address to start of the buffer (0th constant buffer).
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();

	// since constbuffer can be an array of cbuffers;
	// Offset to the ith object constant buffer in the buffer.
	int i = 1;
	int boxCBufIndex = i;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(ObjectConstants));

	md3dDevice->CreateConstantBufferView(
		&cbvDesc,
		mCbvHeap->GetCPUDescriptorHandleForHeapStart());

}

//-------------Root signature-------------//
//Signature of the resources that shader expects to be binded (constant buffers, textures....)

//The following code below creates a root signature that has one root parameter
//that is a descriptor table large enough to store one CBV(constant buffer view) :

void CreateRootSig() {
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
		1, // Number of descriptors in table
		0); //base shader register arguments are bound to for this root parameter

	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a
	// descriptor range consisting of a single constant buffer.

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

//The following code sets the root signature and CBV heap to the command list, and
//sets the descriptor table identifying the resource we want to bind to the pipeline :
void bindRootSig(
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mCbvHeap,
	Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature,
	const UINT& cbvIndex, const UINT& mCbvSrvUavDescriptorSize)
{
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	
	// Offset the CBV we want to use for this draw call.
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbv(mCbvHeap->GetGPUDescriptorHandleForHeapStart());
	cbv.Offset(cbvIndex, mCbvSrvUavDescriptorSize);
}

//---------compiling shader------------//

//runtime
Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target
)
{
	// Use debug flags in debug mode.
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr<ID3DBlob> byteCode = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errors;

	hr = D3DCompileFromFile(filename.c_str(), defines,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint.c_str(),
		target.c_str(), compileFlags, 0, &byteCode, &errors);

	// Output errors to debug window.
	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);
	return byteCode;
}



//load offline compiled shader
Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename)
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios::binary);
	std::ifstream::pos_type size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);

	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	ThrowIfFailed(D3DCreateBlob(size, blob.GetAddressOf()));

	fin.read((char*)blob->GetBufferPointer(), size);
	fin.close();

	return  blob;
}

