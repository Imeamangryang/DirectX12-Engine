#include "FBXLoader.h"

FBXLoader::FBXLoader()
{

}

FBXLoader::~FBXLoader()
{
	if (_scene)
		_scene->Destroy();
	if (_manager)
		_manager->Destroy();
}

void FBXLoader::LoadFbx(const string& path)
{
	// 파일 데이터 로드
	Import(path);

	// Animation	
	LoadBones(_scene->GetRootNode());
	LoadAnimationInfo();

	// 로드된 데이터 파싱 (Mesh/Material/Skin)
	ParseNode(_scene->GetRootNode());

	// 우리 구조에 맞게 Texture / Material 생성
	//CreateTextures();
	//CreateMaterials();
}

void FBXLoader::Import(string path)
{
	// FBX SDK 관리자 객체 생성
	_manager = FbxManager::Create();

	// IOSettings 객체 생성 및 설정
	FbxIOSettings* settings = FbxIOSettings::Create(_manager, IOSROOT);
	_manager->SetIOSettings(settings);

	// FbxImporter 객체 생성
	_scene = FbxScene::Create(_manager, "");

	_importer = FbxImporter::Create(_manager, "");

	_importer->Initialize(path.c_str(), -1, _manager->GetIOSettings());

	_importer->Import(_scene);

	_scene->GetGlobalSettings().SetAxisSystem(FbxAxisSystem::DirectX);

	// 씬 내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
	FbxGeometryConverter geometryConverter(_manager);
	geometryConverter.Triangulate(_scene, true);

	_importer->Destroy();
}

void FBXLoader::ParseNode(FbxNode* node)
{
	FbxNodeAttribute* attribute = node->GetNodeAttribute();

	if (attribute)
	{
		switch (attribute->GetAttributeType())
		{
		case FbxNodeAttribute::eMesh:
			LoadMesh(node->GetMesh());
			break;
		}
	}

	//// Material 로드
	//const uUINT materialCount = node->GetMaterialCount();
	//for (uUINT i = 0; i < materialCount; ++i)
	//{
	//	FbxSurfaceMaterial* surfaceMaterial = node->GetMaterial(i);
	//	LoadMaterial(surfaceMaterial);
	//}

	// Tree 구조 재귀 호출
	const UINT childCount = node->GetChildCount();
	for (UINT i = 0; i < childCount; ++i)
		ParseNode(node->GetChild(i));
}

void FBXLoader::LoadMesh(FbxMesh* mesh)
{
	_meshes.push_back(FbxMeshInfo());
	FbxMeshInfo& meshInfo = _meshes.back();

	meshInfo.name = mesh->GetName();

	const UINT vertexCount = mesh->GetControlPointsCount();
	meshInfo.vertices.resize(vertexCount);
	meshInfo.boneWeights.resize(vertexCount);

	// Position
	FbxVector4* controlPoints = mesh->GetControlPoints();
	for (UINT i = 0; i < vertexCount; ++i)
	{
		meshInfo.vertices[i].Position.x = static_cast<float>(controlPoints[i].mData[0]);
		meshInfo.vertices[i].Position.y = static_cast<float>(controlPoints[i].mData[2]);
		meshInfo.vertices[i].Position.z = static_cast<float>(controlPoints[i].mData[1]);
	}

	const UINT materialCount = mesh->GetNode()->GetMaterialCount();
	meshInfo.indices.resize(materialCount);
	meshInfo.indices.clear();

	FbxGeometryElementMaterial* geometryElementMaterial = mesh->GetElementMaterial();

	std::uint32_t arrIdx[3];
	UINT vertexCounter = 0; // 정점의 개수

	const int triCount = mesh->GetPolygonCount(); // 메쉬의 삼각형 개수를 가져온다
	for (int i = 0; i < triCount; i++) // 삼각형의 개수
	{
		for (int j = 0; j < 3; j++) // 삼각형은 세 개의 정점으로 구성
		{
			std::uint32_t controlPointIndex = mesh->GetPolygonVertex(i, j); // 제어점의 인덱스 추출
			arrIdx[j] = controlPointIndex;

			GetNormal(mesh, &meshInfo, controlPointIndex, vertexCounter);
			GetTangent(mesh, &meshInfo, controlPointIndex, vertexCounter);
			GetUV(mesh, &meshInfo, controlPointIndex, mesh->GetTextureUVIndex(i, j));

			vertexCounter++;
		}

		const UINT subsetIdx = geometryElementMaterial->GetIndexArray().GetAt(i);
		meshInfo.indices.push_back(arrIdx[0]);
		meshInfo.indices.push_back(arrIdx[2]);
		meshInfo.indices.push_back(arrIdx[1]);
	}

	//// Animation
	//LoadAnimationData(mesh, &meshInfo);
}

void FBXLoader::GetNormal(FbxMesh* mesh, FbxMeshInfo* container, UINT idx, UINT vertexCounter)
{
	if (mesh->GetElementNormalCount() == 0)
		return;

	FbxGeometryElementNormal* normal = mesh->GetElementNormal();
	UINT normalIdx = 0;

	if (normal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
	{
		if (normal->GetReferenceMode() == FbxGeometryElement::eDirect)
			normalIdx = vertexCounter;
		else
			normalIdx = normal->GetIndexArray().GetAt(vertexCounter);
	}
	else if (normal->GetMappingMode() == FbxGeometryElement::eByControlPoint)
	{
		if (normal->GetReferenceMode() == FbxGeometryElement::eDirect)
			normalIdx = idx;
		else
			normalIdx = normal->GetIndexArray().GetAt(idx);
	}

	FbxVector4 vec = normal->GetDirectArray().GetAt(normalIdx);
	container->vertices[idx].Normal.x = static_cast<float>(vec.mData[0]);
	container->vertices[idx].Normal.y = static_cast<float>(vec.mData[2]);
	container->vertices[idx].Normal.z = static_cast<float>(vec.mData[1]);
}

void FBXLoader::GetTangent(FbxMesh* mesh, FbxMeshInfo* meshInfo, UINT idx, UINT vertexCounter)
{
	if (mesh->GetElementTangentCount() == 0)
	{
		// TEMP : 원래는 이런 저런 알고리즘으로 Tangent 만들어줘야 함
		meshInfo->vertices[idx].TangentU.x = 1.f;
		meshInfo->vertices[idx].TangentU.y = 0.f;
		meshInfo->vertices[idx].TangentU.z = 0.f;
		return;
	}

	FbxGeometryElementTangent* tangent = mesh->GetElementTangent();
	UINT tangentIdx = 0;

	if (tangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
	{
		if (tangent->GetReferenceMode() == FbxGeometryElement::eDirect)
			tangentIdx = vertexCounter;
		else
			tangentIdx = tangent->GetIndexArray().GetAt(vertexCounter);
	}
	else if (tangent->GetMappingMode() == FbxGeometryElement::eByControlPoint)
	{
		if (tangent->GetReferenceMode() == FbxGeometryElement::eDirect)
			tangentIdx = idx;
		else
			tangentIdx = tangent->GetIndexArray().GetAt(idx);
	}

	FbxVector4 vec = tangent->GetDirectArray().GetAt(tangentIdx);
	meshInfo->vertices[idx].TangentU.x = static_cast<float>(vec.mData[0]);
	meshInfo->vertices[idx].TangentU.y = static_cast<float>(vec.mData[2]);
	meshInfo->vertices[idx].TangentU.z = static_cast<float>(vec.mData[1]);
}

void FBXLoader::GetUV(FbxMesh* mesh, FbxMeshInfo* meshInfo, UINT idx, UINT uvIndex)
{
	FbxVector2 uv = mesh->GetElementUV()->GetDirectArray().GetAt(uvIndex);
	meshInfo->vertices[idx].TexC.x = static_cast<float>(uv.mData[0]);
	meshInfo->vertices[idx].TexC.y = 1.f - static_cast<float>(uv.mData[1]);
}

//void FBXLoader::LoadMaterial(FbxSurfaceMaterial* surfaceMaterial)
//{
//	FbxMaterialInfo material{};
//
//	material.name = surfaceMaterial->GetName();
//
//	material.diffuse = GetMaterialData(surfaceMaterial, FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor);
//	material.ambient = GetMaterialData(surfaceMaterial, FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor);
//	material.specular = GetMaterialData(surfaceMaterial, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor);
//
//	material.diffuseTexName = GetTextureRelativeName(surfaceMaterial, FbxSurfaceMaterial::sDiffuse);
//	material.normalTexName = GetTextureRelativeName(surfaceMaterial, FbxSurfaceMaterial::sNormalMap);
//	material.specularTexName = GetTextureRelativeName(surfaceMaterial, FbxSurfaceMaterial::sSpecular);
//
//	_meshes.back().materials.push_back(material);
//}
//
//XMFLOAT4 FBXLoader::GetMaterialData(FbxSurfaceMaterial* surface, const char* materialName, const char* factorName)
//{
//	FbxDouble3  material;
//	FbxDouble	factor = 0.f;
//
//	FbxProperty materialProperty = surface->FindProperty(materialName);
//	FbxProperty factorProperty = surface->FindProperty(factorName);
//
//	if (materialProperty.IsValid() && factorProperty.IsValid())
//	{
//		material = materialProperty.Get<FbxDouble3>();
//		factor = factorProperty.Get<FbxDouble>();
//	}
//
//	XMFLOAT4 ret = XMFLOAT4(
//		static_cast<float>(material.mData[0] * factor),
//		static_cast<float>(material.mData[1] * factor),
//		static_cast<float>(material.mData[2] * factor),
//		static_cast<float>(factor));
//
//	return ret;
//}
//
//string FBXLoader::GetTextureRelativeName(FbxSurfaceMaterial* surface, const char* materialProperty)
//{
//	string name;
//
//	FbxProperty textureProperty = surface->FindProperty(materialProperty);
//	if (textureProperty.IsValid())
//	{
//		UINT count = textureProperty.GetSrcObjectCount();
//
//		if (1 <= count)
//		{
//			FbxFileTexture* texture = textureProperty.GetSrcObject<FbxFileTexture>(0);
//			if (texture)
//				name = texture->GetRelativeFileName();
//		}
//	}
//
//	return name;
//}

void FBXLoader::LoadBones(FbxNode* node, UINT idx, UINT parentIdx)
{
	FbxNodeAttribute* attribute = node->GetNodeAttribute();

	if (attribute && attribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		shared_ptr<FbxBoneInfo> bone = make_shared<FbxBoneInfo>();
		bone->boneName = node->GetName();
		bone->parentIndex = parentIdx;
		_bones.push_back(bone);
	}

	const UINT childCount = node->GetChildCount();
	for (UINT i = 0; i < childCount; i++)
		LoadBones(node->GetChild(i), static_cast<UINT>(_bones.size()), idx);
}

void FBXLoader::LoadAnimationInfo()
{
	_scene->FillAnimStackNameArray(OUT _animNames);

	const UINT animCount = _animNames.GetCount();
	for (UINT i = 0; i < animCount; i++)
	{
		FbxAnimStack* animStack = _scene->FindMember<FbxAnimStack>(_animNames[i]->Buffer());
		if (animStack == nullptr)
			continue;

		shared_ptr<FbxAnimClipInfo> animClip = make_shared<FbxAnimClipInfo>();
		animClip->name = animStack->GetName();
		animClip->keyFrames.resize(_bones.size()); // 키프레임은 본의 개수만큼

		FbxTakeInfo* takeInfo = _scene->GetTakeInfo(animStack->GetName());
		animClip->startTime = takeInfo->mLocalTimeSpan.GetStart();
		animClip->endTime = takeInfo->mLocalTimeSpan.GetStop();
		animClip->mode = _scene->GetGlobalSettings().GetTimeMode();

		_animClips.push_back(animClip);
	}
}

void FBXLoader::LoadAnimationData(FbxMesh* mesh, FbxMeshInfo* meshInfo)
{
	const UINT skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
	if (skinCount <= 0 || _animClips.empty())
		return;

	meshInfo->hasAnimation = true;

	for (UINT i = 0; i < skinCount; i++)
	{
		FbxSkin* fbxSkin = static_cast<FbxSkin*>(mesh->GetDeformer(i, FbxDeformer::eSkin));

		if (fbxSkin)
		{
			FbxSkin::EType type = fbxSkin->GetSkinningType();
			if (FbxSkin::eRigid == type || FbxSkin::eLinear)
			{
				const UINT clusterCount = fbxSkin->GetClusterCount();
				for (UINT j = 0; j < clusterCount; j++)
				{
					FbxCluster* cluster = fbxSkin->GetCluster(j);
					if (cluster->GetLink() == nullptr)
						continue;

					UINT boneIdx = FindBoneIndex(cluster->GetLink()->GetName());
					assert(boneIdx >= 0);

					FbxAMatrix matNodeTransform = GetTransform(mesh->GetNode());
					LoadBoneWeight(cluster, boneIdx, meshInfo);
					LoadOffsetMatrix(cluster, matNodeTransform, boneIdx, meshInfo);

					const UINT animCount = _animNames.Size();
					for (UINT k = 0; k < animCount; k++)
						LoadKeyframe(k, mesh->GetNode(), cluster, matNodeTransform, boneIdx, meshInfo);
				}
			}
		}
	}

	FillBoneWeight(mesh, meshInfo);
}

void FBXLoader::FillBoneWeight(FbxMesh* mesh, FbxMeshInfo* meshInfo)
{
	const UINT size = static_cast<UINT>(meshInfo->boneWeights.size());
	for (UINT v = 0; v < size; v++)
	{
		BoneWeight& boneWeight = meshInfo->boneWeights[v];
		boneWeight.Normalize();

		float animBoneIndex[4] = {};
		float animBoneWeight[4] = {};

		const UINT weightCount = static_cast<UINT>(boneWeight.boneWeights.size());
		for (UINT w = 0; w < weightCount; w++)
		{
			animBoneIndex[w] = static_cast<float>(boneWeight.boneWeights[w].first);
			animBoneWeight[w] = static_cast<float>(boneWeight.boneWeights[w].second);
		}

		memcpy(&meshInfo->vertices[v].BoneIndices, animBoneIndex, sizeof(XMFLOAT4));
		memcpy(&meshInfo->vertices[v].BoneWeights, animBoneWeight, sizeof(XMFLOAT4));
	}
}

void FBXLoader::LoadBoneWeight(FbxCluster* cluster, UINT boneIdx, FbxMeshInfo* meshInfo)
{
	const UINT indicesCount = cluster->GetControlPointIndicesCount();
	for (UINT i = 0; i < indicesCount; i++)
	{
		double weight = cluster->GetControlPointWeights()[i];
		UINT vtxIdx = cluster->GetControlPointIndices()[i];
		meshInfo->boneWeights[vtxIdx].AddWeights(boneIdx, weight);
	}
}

void FBXLoader::LoadOffsetMatrix(FbxCluster* cluster, const FbxAMatrix& matNodeTransform, UINT boneIdx, FbxMeshInfo* meshInfo)
{
	FbxAMatrix matClusterTrans;
	FbxAMatrix matClusterLinkTrans;
	// The transformation of the mesh at binding time 
	cluster->GetTransformMatrix(matClusterTrans);
	// The transformation of the cluster(joint) at binding time from joint space to world space 
	cluster->GetTransformLinkMatrix(matClusterLinkTrans);

	FbxVector4 V0 = { 1, 0, 0, 0 };
	FbxVector4 V1 = { 0, 0, 1, 0 };
	FbxVector4 V2 = { 0, 1, 0, 0 };
	FbxVector4 V3 = { 0, 0, 0, 1 };

	FbxAMatrix matReflect;
	matReflect[0] = V0;
	matReflect[1] = V1;
	matReflect[2] = V2;
	matReflect[3] = V3;

	FbxAMatrix matOffset;
	matOffset = matClusterLinkTrans.Inverse() * matClusterTrans;
	matOffset = matReflect * matOffset * matReflect;

	_bones[boneIdx]->matOffset = matOffset.Transpose();
}

void FBXLoader::LoadKeyframe(UINT animIndex, FbxNode* node, FbxCluster* cluster, const FbxAMatrix& matNodeTransform, UINT boneIdx, FbxMeshInfo* meshInfo)
{
	if (_animClips.empty())
		return;

	FbxVector4	v1 = { 1, 0, 0, 0 };
	FbxVector4	v2 = { 0, 0, 1, 0 };
	FbxVector4	v3 = { 0, 1, 0, 0 };
	FbxVector4	v4 = { 0, 0, 0, 1 };
	FbxAMatrix	matReflect;
	matReflect.mData[0] = v1;
	matReflect.mData[1] = v2;
	matReflect.mData[2] = v3;
	matReflect.mData[3] = v4;

	FbxTime::EMode timeMode = _scene->GetGlobalSettings().GetTimeMode();

	// 애니메이션 골라줌
	FbxAnimStack* animStack = _scene->FindMember<FbxAnimStack>(_animNames[animIndex]->Buffer());
	_scene->SetCurrentAnimationStack(OUT animStack);

	FbxLongLong startFrame = _animClips[animIndex]->startTime.GetFrameCount(timeMode);
	FbxLongLong endFrame = _animClips[animIndex]->endTime.GetFrameCount(timeMode);

	for (FbxLongLong frame = startFrame; frame < endFrame; frame++)
	{
		FbxKeyFrameInfo keyFrameInfo = {};
		FbxTime fbxTime = 0;

		fbxTime.SetFrame(frame, timeMode);

		FbxAMatrix matFromNode = node->EvaluateGlobalTransform(fbxTime);
		FbxAMatrix matTransform = matFromNode.Inverse() * cluster->GetLink()->EvaluateGlobalTransform(fbxTime);
		matTransform = matReflect * matTransform * matReflect;

		keyFrameInfo.time = fbxTime.GetSecondDouble();
		keyFrameInfo.matTransform = matTransform;

		_animClips[animIndex]->keyFrames[boneIdx].push_back(keyFrameInfo);
	}
}

UINT FBXLoader::FindBoneIndex(string name)
{
	for (UINT i = 0; i < _bones.size(); ++i)
	{
		if (_bones[i]->boneName == name)
			return i;
	}

	return -1;
}

FbxAMatrix FBXLoader::GetTransform(FbxNode* node)
{
	const FbxVector4 translation = node->GetGeometricTranslation(FbxNode::eSourcePivot);
	const FbxVector4 rotation = node->GetGeometricRotation(FbxNode::eSourcePivot);
	const FbxVector4 scaling = node->GetGeometricScaling(FbxNode::eSourcePivot);
	return FbxAMatrix(translation, rotation, scaling);
}

