#pragma once

#include "core\scene\universe.h"
#include "core\engine.h"
#include "renderer\renderer.h"

#include "material.h"

namespace Cjing3D
{
	/// ////////////////////////////////////////////////////////////////////////////////
	/// Components
	struct CameraComponent
	{
		F32 mWidth = 0.0f;
		F32 mHeight = 0.0f;
		F32 mNear = 0.1f;
		F32 mFar = 800.0f;
		F32 mFov = XM_PI / 3.0f;
	};

	struct MaterialComponent
	{
		// res material
		MaterialRef mMaterial;

		// members
		BLENDMODE mBlendMode = BLENDMODE_OPAQUE;
		F32x4 mBaseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		bool mUseCustomShader = false;
		GPU::ResHandle mConstantBuffer;

		// methods
		BLENDMODE GetBlendMode()const { return mBlendMode; }

		GPU::ResHandle GetTexture(Material::TextureSlot slot)const {
			return mMaterial ? mMaterial->GetTexture(slot) : GPU::ResHandle::INVALID_HANDLE;
		}
	};

	struct MeshComponent
	{
		struct MeshSubset
		{
			ECS::Entity mMaterialID = ECS::INVALID_ENTITY;
			U32 mIndexOffset = 0;
			U32 mIndexCount = 0;
		};
		DynamicArray<MeshSubset> mSubsets;

		DynamicArray<F32x3> mVertexPositions;
		DynamicArray<F32x3> mVertexNormals;
		DynamicArray<F32x2> mVertexUVset;
		DynamicArray<U32> mVertexColors;
		DynamicArray<U32> mIndices;

		GPU::ResHandle mIndexBuffer;
		GPU::ResHandle mVertexBufferPos;
		GPU::ResHandle mVertexBufferTex;
		GPU::ResHandle mVertexBufferColor;

		GPU::IndexFormat GetIndexFormat()const { 
			return mVertexPositions.size() > 65536 ? 
				GPU::IndexFormat::INDEX_FORMAT_32BIT : GPU::IndexFormat::INDEX_FORMAT_16BIT;
		}

		struct VertexPos
		{
			F32x3 mPos = F32x3(0.0f, 0.0f, 0.0f);
			F32 mNormal = 0;
		};
		struct VertexTex
		{
			F32x2 mTex = F32x2(0.0f, 0.0f);
		};
		struct VertexColor
		{
			U32 mColor = 0;
		};
	};

	struct ObjectComponent
	{
		enum Flags
		{
			EMPTY = 0,
			RENDERABLE = 1,
			CAST_SHADOW = 1 << 1,
		};
		I32 mFalgs = (I32)RENDERABLE;
		ECS::Entity mMeshID = ECS::INVALID_ENTITY;
		I32   mRenderTypeMask = 0;
		F32x4 mColor = F32x4(1.0f, 1.0f, 1.0f, 1.0f);
		F32x3 mCenter = F32x3(0.0f, 0.0f, 0.0f); // assigned in system updating
		I32   mTransformIndex = -1;

		bool IsRenderable()const {
			return mFalgs & RENDERABLE;
		}
		bool IsCastShadow()const {
			return mFalgs & CAST_SHADOW;
		}
	};

	/// ////////////////////////////////////////////////////////////////////////////////
	/// RenderScene Systems
	DECLARE_SYSTEM(MaterialSystem);
	DECLARE_SYSTEM(MeshSystem);
	DECLARE_SYSTEM(ObjectSystem);

	/// ////////////////////////////////////////////////////////////////////////////////
	/// RenderScene
	class RenderScene : public ECS::IScene
	{
	public:
		RenderScene(Engine& engine, Universe& universe);
		virtual ~RenderScene();

		void Initialize()override;
		void Uninitialize()override;
		void Update(F32 dt)override;
		void LateUpdate(F32 dt)override;
		void Clear()override;
		Universe& GetUniverse()override;

		void GetCullingResult(Visibility& cullingResult, Frustum& frustum, I32 cullingFlag);

		static void RegisterReflect();

	public:
		ECS::ComponentManager<MaterialComponent>* mMaterials;
		ECS::ComponentManager<MeshComponent>* mMeshes;
		ECS::ComponentManager<ObjectComponent>* mObjects;
		ECS::ComponentManager<AABB>* mObjectAABBs;

	private:
		Engine& mEngine;
		Universe& mUniverse;
	};
}