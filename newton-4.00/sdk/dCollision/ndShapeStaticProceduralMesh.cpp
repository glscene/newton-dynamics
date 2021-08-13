/* Copyright (c) <2003-2021> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "dCoreStdafx.h"
#include "ndCollisionStdafx.h"
#include "ndContact.h"
#include "ndBodyKinematic.h"
#include "ndShapeInstance.h"
#include "ndShapeStaticProceduralMesh.h"

template<class T>
class dTempArray : public dArray<T>
{
	public:
	dTempArray(dInt32 maxSize, T* const buffer) 
		:dArray<T>()
		,m_buffer(buffer)
	{
		m_array = buffer;
		m_capacity = maxSize;
	}

	~dTempArray()
	{
		m_array = nullptr;
	}

	T* m_buffer;
};

ndShapeStaticProceduralMesh::ndShapeStaticProceduralMesh(dFloat32 sizex, dFloat32 sizey, dFloat32 sizez)
	:ndShapeStaticMesh(m_staticProceduralMesh)
	,m_minBox(dVector::m_negOne * dVector::m_half * dVector(sizex, sizey, sizez, dFloat32(0.0f)))
	,m_maxBox(dVector::m_half * dVector(sizex, sizey, sizez, dFloat32(0.0f)))
	,m_localData()
	,m_maxVertexCount(256)
	,m_maxFaceCount(64)
{
	CalculateLocalObb();
}

ndShapeStaticProceduralMesh::ndShapeStaticProceduralMesh(const nd::TiXmlNode* const, const char* const)
	:ndShapeStaticMesh(m_staticProceduralMesh)
{
	dAssert(0);
}

ndShapeStaticProceduralMesh::~ndShapeStaticProceduralMesh(void)
{
}

void ndShapeStaticProceduralMesh::SetMaxVertexAndFaces(dInt32 maxVertex, dInt32 maxFaces)
{
	m_maxVertexCount = maxVertex;
	m_maxFaceCount = maxFaces;
}

//void ndShapeStaticProceduralMesh::Save(nd::TiXmlElement* const xmlNode, const char* const assetPath, dInt32 nodeid) const
void ndShapeStaticProceduralMesh::Save(nd::TiXmlElement* const, const char* const, dInt32) const
{
	dAssert(0);
}

ndShapeInfo ndShapeStaticProceduralMesh::GetShapeInfo() const
{
	ndShapeInfo info(ndShapeStaticMesh::GetShapeInfo());

	dAssert(0);
	//info.m_heightfield.m_width = m_width;
	//info.m_heightfield.m_height = m_height;
	//info.m_heightfield.m_gridsDiagonals = m_diagonalMode;
	//info.m_heightfield.m_verticalScale = m_verticalScale;
	//info.m_heightfield.m_horizonalScale_x = m_horizontalScale_x;
	//info.m_heightfield.m_horizonalScale_z = m_horizontalScale_z;
	//info.m_heightfield.m_elevation = (dInt16*)&m_elevationMap[0];
	//info.m_heightfield.m_atributes = (dInt8*)&m_atributeMap[0];

	return info;
}

void ndShapeStaticProceduralMesh::CalculateLocalObb()
{
	m_boxSize = (m_maxBox - m_minBox) * dVector::m_half;
	m_boxOrigin = (m_maxBox + m_minBox) * dVector::m_half;
}

//void ndShapeStaticProceduralMesh::UpdateElevationMapAabb()
//{
//	CalculateLocalObb();
//}
//
//const dInt32* ndShapeStaticProceduralMesh::GetIndexList() const
//{
//	return &m_cellIndices[(m_diagonalMode == m_normalDiagonals) ? 0 : 1][0];
//}

//void ndShapeStaticProceduralMesh::DebugShape(const dMatrix& matrix, ndShapeDebugCallback& debugCallback) const
void ndShapeStaticProceduralMesh::DebugShape(const dMatrix&, ndShapeDebugCallback&) const
{
	dAssert(0);
	//dVector points[4];
	//dVector triangle[3];
	//
	//ndShapeDebugCallback::ndEdgeType edgeType[4];
	//memset(edgeType, ndShapeDebugCallback::m_shared, sizeof(edgeType));
	//
	//const dInt32* const indirectIndex = GetIndexList();
	//const dInt32 i0 = indirectIndex[0];
	//const dInt32 i1 = indirectIndex[1];
	//const dInt32 i2 = indirectIndex[2];
	//const dInt32 i3 = indirectIndex[3];
	//
	//dInt32 base = 0;
	//for (dInt32 z = 0; z < m_height - 1; z++) 
	//{
	//	const dVector p0 ((0 + 0) * m_horizontalScale_x, m_verticalScale * dFloat32(m_elevationMap[base + 0]),               (z + 0) * m_horizontalScale_z, dFloat32(0.0f));
	//	const dVector p1 ((0 + 0) * m_horizontalScale_x, m_verticalScale * dFloat32(m_elevationMap[base + 0 + m_width + 0]), (z + 1) * m_horizontalScale_z, dFloat32(0.0f));
	//
	//	points[0 * 2 + 0] = matrix.TransformVector(p0);
	//	points[1 * 2 + 0] = matrix.TransformVector(p1);
	//
	//	for (dInt32 x = 0; x < m_width - 1; x++) 
	//	{
	//		const dVector p2 ((x + 1) * m_horizontalScale_x, m_verticalScale * dFloat32(m_elevationMap[base + x + 1]),			(z + 0) * m_horizontalScale_z, dFloat32(0.0f));
	//		const dVector p3 ((x + 1) * m_horizontalScale_x, m_verticalScale * dFloat32(m_elevationMap[base + x + m_width + 1]), (z + 1) * m_horizontalScale_z, dFloat32(0.0f));
	//
	//		points[0 * 2 + 1] = matrix.TransformVector(p2);
	//		points[1 * 2 + 1] = matrix.TransformVector(p3);
	//
	//		triangle[0] = points[i1];
	//		triangle[1] = points[i0];
	//		triangle[2] = points[i2];
	//		debugCallback.DrawPolygon(3, triangle, edgeType);
	//
	//		triangle[0] = points[i1];
	//		triangle[1] = points[i2];
	//		triangle[2] = points[i3];
	//		debugCallback.DrawPolygon(3, triangle, edgeType);
	//
	//		points[0 * 2 + 0] = points[0 * 2 + 1];
	//		points[1 * 2 + 0] = points[1 * 2 + 1];
	//	}
	//	base += m_width;
	//}
}

//void ndShapeStaticProceduralMesh::CalculateMinExtend2d(const dVector& p0, const dVector& p1, dVector& boxP0, dVector& boxP1) const
//{
//	const dVector scale(m_horizontalScale_x, dFloat32(0.0f), m_horizontalScale_z, dFloat32(0.0f));
//	const dVector q0(p0.GetMin(p1) - m_padding);
//	const dVector q1(p0.GetMax(p1) + scale + m_padding);
//
//	const dVector invScale(m_horizontalScaleInv_x, dFloat32(0.0f), m_horizontalScaleInv_z, dFloat32(0.0f));
//	boxP0 = (((q0 * invScale).Floor() * scale)         & m_yMask) - m_elevationPadding;
//	boxP1 = (((q1 * invScale).Floor() * scale + scale) & m_yMask) + m_elevationPadding;
//	dAssert(boxP0.m_w == dFloat32(0.0f));
//	dAssert(boxP1.m_w == dFloat32(0.0f));
//
//	const dVector minBox(boxP0.Select(m_minBox, m_yMask));
//	const dVector maxBox(boxP1.Select(m_maxBox, m_yMask));
//
//	boxP0 = boxP0.GetMax(minBox);
//	boxP1 = boxP1.GetMin(maxBox);
//}
//
//void ndShapeStaticProceduralMesh::CalculateMinExtend3d(const dVector& p0, const dVector& p1, dVector& boxP0, dVector& boxP1) const
//{
//	dAssert(p0.m_x <= p1.m_x);
//	dAssert(p0.m_y <= p1.m_y);
//	dAssert(p0.m_z <= p1.m_z);
//	dAssert(p0.m_w == dFloat32(0.0f));
//	dAssert(p1.m_w == dFloat32(0.0f));
//
//	const dVector scale(m_horizontalScale_x, dFloat32(0.0f), m_horizontalScale_z, dFloat32(0.0f));
//	const dVector q0(p0.GetMin(p1) - m_padding);
//	const dVector q1(p0.GetMax(p1) + scale + m_padding);
//	const dVector invScale(m_horizontalScaleInv_x, dFloat32(0.0f), m_horizontalScaleInv_z, dFloat32(0.0f));
//
//	boxP0 = q0.Select((q0 * invScale).Floor() * scale, m_yMask);
//	boxP1 = q1.Select((q1 * invScale).Floor() * scale + scale, m_yMask);
//	
//	boxP0 = boxP0.Select(boxP0.GetMax(m_minBox), m_yMask);
//	boxP1 = boxP1.Select(boxP1.GetMin(m_maxBox), m_yMask);
//}
//
//void ndShapeStaticProceduralMesh::GetLocalAabb(const dVector& q0, const dVector& q1, dVector& boxP0, dVector& boxP1) const
//{
//	// the user data is the pointer to the collision geometry
//	CalculateMinExtend3d(q0, q1, boxP0, boxP1);
//	
//	const dVector p0(boxP0.Scale(m_horizontalScaleInv_x).GetInt());
//	const dVector p1(boxP1.Scale(m_horizontalScaleInv_x).GetInt());
//	
//	dAssert(p0.m_ix == FastInt(boxP0.m_x * m_horizontalScaleInv_x));
//	dAssert(p0.m_iz == FastInt(boxP0.m_z * m_horizontalScaleInv_x));
//	dAssert(p1.m_ix == FastInt(boxP1.m_x * m_horizontalScaleInv_x));
//	dAssert(p1.m_iz == FastInt(boxP1.m_z * m_horizontalScaleInv_x));
//	
//	dInt32 x0 = dInt32(p0.m_ix);
//	dInt32 x1 = dInt32(p1.m_ix);
//	dInt32 z0 = dInt32(p0.m_iz);
//	dInt32 z1 = dInt32(p1.m_iz);
//	
//	dFloat32 minHeight = dFloat32(1.0e10f);
//	dFloat32 maxHeight = dFloat32(-1.0e10f);
//	CalculateMinAndMaxElevation(x0, x1, z0, z1, minHeight, maxHeight);
//	boxP0.m_y = minHeight;
//	boxP1.m_y = maxHeight;
//}
//
//dFloat32 ndShapeStaticProceduralMesh::RayCastCell(const dFastRay& ray, dInt32 xIndex0, dInt32 zIndex0, dVector& normalOut, dFloat32 maxT) const
//{
//	dVector points[4];
//	dInt32 triangle[3];
//
//	// get the 3d point at the corner of the cell
//	if ((xIndex0 < 0) || (zIndex0 < 0) || (xIndex0 >= (m_width - 1)) || (zIndex0 >= (m_height - 1))) 
//	{
//		return dFloat32(1.2f);
//	}
//
//	dAssert(maxT <= 1.0);
//
//	dInt32 base = zIndex0 * m_width + xIndex0;
//
//	//const dFloat32* const elevation = (dFloat32*)m_elevationMap;
//	points[0 * 2 + 0] = dVector((xIndex0 + 0) * m_horizontalScale_x, m_verticalScale * dFloat32 (m_elevationMap[base + 0]),			  (zIndex0 + 0) * m_horizontalScale_z, dFloat32(0.0f));
//	points[0 * 2 + 1] = dVector((xIndex0 + 1) * m_horizontalScale_x, m_verticalScale * dFloat32 (m_elevationMap[base + 1]),			  (zIndex0 + 0) * m_horizontalScale_z, dFloat32(0.0f));
//	points[1 * 2 + 1] = dVector((xIndex0 + 1) * m_horizontalScale_x, m_verticalScale * dFloat32 (m_elevationMap[base + m_width + 1]), (zIndex0 + 1) * m_horizontalScale_z, dFloat32(0.0f));
//	points[1 * 2 + 0] = dVector((xIndex0 + 0) * m_horizontalScale_x, m_verticalScale * dFloat32 (m_elevationMap[base + m_width + 0]), (zIndex0 + 1) * m_horizontalScale_z, dFloat32(0.0f));
//
//	dFloat32 t = dFloat32(1.2f);
//	//if (!m_diagonals[base]) 
//	if (m_diagonalMode == m_normalDiagonals)
//	{
//		triangle[0] = 1;
//		triangle[1] = 2;
//		triangle[2] = 3;
//
//		dVector e10(points[2] - points[1]);
//		dVector e20(points[3] - points[1]);
//		dVector normal(e10.CrossProduct(e20));
//		normal = normal.Normalize();
//		t = ray.PolygonIntersect(normal, maxT, &points[0].m_x, sizeof(dVector), triangle, 3);
//		if (t < maxT) 
//		{
//			normalOut = normal;
//			return t;
//		}
//
//		triangle[0] = 1;
//		triangle[1] = 0;
//		triangle[2] = 2;
//
//		dVector e30(points[0] - points[1]);
//		normal = e30.CrossProduct(e10);
//		normal = normal.Normalize();
//		t = ray.PolygonIntersect(normal, maxT, &points[0].m_x, sizeof(dVector), triangle, 3);
//		if (t < maxT) 
//		{
//			normalOut = normal;
//			return t;
//		}
//	}
//	else 
//	{
//		triangle[0] = 0;
//		triangle[1] = 2;
//		triangle[2] = 3;
//
//		dVector e10(points[2] - points[0]);
//		dVector e20(points[3] - points[0]);
//		dVector normal(e10.CrossProduct(e20));
//		normal = normal.Normalize();
//		t = ray.PolygonIntersect(normal, maxT, &points[0].m_x, sizeof(dVector), triangle, 3);
//		if (t < maxT) 
//		{
//			normalOut = normal;
//			return t;
//		}
//
//		triangle[0] = 0;
//		triangle[1] = 3;
//		triangle[2] = 1;
//
//		dVector e30(points[1] - points[0]);
//		normal = e20.CrossProduct(e30);
//		normal = normal.Normalize();
//		t = ray.PolygonIntersect(normal, maxT, &points[0].m_x, sizeof(dVector), triangle, 3);
//		if (t < maxT) 
//		{
//			normalOut = normal;
//			return t;
//		}
//	}
//	return t;
//}


//void ndShapeStaticProceduralMesh::CalculateMinAndMaxElevation(dInt32 x0, dInt32 x1, dInt32 z0, dInt32 z1, dFloat32& minHeight, dFloat32& maxHeight) const
//{
//	dInt16 minVal = 0x7fff;
//	dInt16 maxVal = -0x7fff;
//	dInt32 base = z0 * m_width;
//	for (dInt32 z = z0; z <= z1; z++) 
//	{
//		for (dInt32 x = x0; x <= x1; x++) 
//		{
//			dInt16 high = m_elevationMap[base + x];
//			minVal = dMin(high, minVal);
//			maxVal = dMax(high, maxVal);
//		}
//		base += m_width;
//	}
//
//	minHeight = minVal * m_verticalScale;
//	maxHeight = maxVal * m_verticalScale;
//}

void ndShapeStaticProceduralMesh::GetCollidingFaces(ndPolygonMeshDesc* const data) const
{
	//dgWorld* const world = data->m_objBody->GetWorld();
	// the user data is the pointer to the collision geometry
	//CalculateMinExtend3d(data->GetOrigin(), data->GetTarget(), boxP0, boxP1);
	//	const dVector q0(p0.GetMin(p1) - m_padding);
	//	const dVector q1(p0.GetMax(p1) + scale + m_padding);

	dVector* const vertexBuffer = dAlloca(dVector, m_maxVertexCount);
	dInt32* const faceBuffer = dAlloca(dInt32, m_maxFaceCount);
	dInt32* const materialBuffer = dAlloca(dInt32, m_maxFaceCount);
	dInt32* const indexBuffer = dAlloca(dInt32, m_maxFaceCount * 4);

	dTempArray<dVector> vertexList(m_maxVertexCount, vertexBuffer);
	dTempArray<dInt32> faceList(m_maxFaceCount, faceBuffer);
	dTempArray<dInt32> faceMaterialList(m_maxFaceCount, materialBuffer);
	dTempArray<dInt32> indexList(m_maxFaceCount * 4, indexBuffer);

	GetCollidingFaces(data->GetOrigin(), data->GetTarget(), vertexList, faceList, faceMaterialList, indexList);
	if (faceList.GetCount() == 0)
	{
		return;
	}

	std::thread::id threadId = std::this_thread::get_id();
	dList<ndLocalThreadData>::dNode* localDataNode = nullptr;
	for (dList<ndLocalThreadData>::dNode* node = m_localData.GetFirst(); node; node = node->GetNext())
	{
		if (node->GetInfo().m_threadId == threadId)
		{
			localDataNode = node;
			break;
		}
	}
	
	if (!localDataNode)
	{
		localDataNode = m_localData.Append();
		localDataNode->GetInfo().m_threadId = threadId;
	}
	
	//	// scan the vertices's intersected by the box extend
	//	dInt32 vertexCount = (z1 - z0 + 1) * (x1 - x0 + 1) + 2 * (z1 - z0) * (x1 - x0);
	dArray<dVector>& vertex = localDataNode->GetInfo().m_vertex;
	vertex.SetCount(vertexList.GetCount() + faceList.GetCount());
	memcpy(&vertex[0], &vertexList[0], vertexList.GetCount() * sizeof(dVector));
	
	ndEdgeMap edgeMap;
	dInt32 index = 0;
	dInt32 faceStart = 0;
	dInt32* const indices = data->m_globalFaceVertexIndex;
	dInt32* const faceIndexCount = data->m_meshData.m_globalFaceIndexCount;
	
	for (dInt32 i = 0; i < faceList.GetCount(); i++)
	{
		dInt32 i0 = indexList[faceStart + 0];
		dInt32 i1 = indexList[faceStart + 1];
		dVector area(dVector::m_zero);
		dVector edge0(vertex[i1] - vertex[i0]);

		for (dInt32 j = 2; j < faceList[i]; j++)
		{
			dInt32 i2 = indexList[faceStart + j];
			const dVector edge1(vertex[i2] - vertex[i0]);
			area += edge0.CrossProduct(edge1);
			edge0 = edge1;
		}

		dFloat32 faceSize = dSqrt(area.DotProduct(area).GetScalar());
		dInt32 normalIndex = vertexList.GetCount() + i;

		vertex[normalIndex] = area.Scale (dFloat32 (1.0f) / faceSize);

		indices[index + faceList[i] + 0] = faceMaterialList[i];
		indices[index + faceList[i] + 1] = normalIndex;
		indices[index + 2 * faceList[i] + 2] = dInt32(faceSize * dFloat32(0.5f));

		dInt32 j0 = faceList[i] - 1;
		faceIndexCount[i] = faceList[i];
		for (dInt32 j = 0; j < faceList[i]; j++) 
		{
			ndEdge edge;
			edge.m_i0 = indexList[faceStart + j0];
			edge.m_i1 = indexList[faceStart + j];
			dInt32 normalEntryIndex = index + j + faceList[i] + 2;
			edgeMap.Insert(normalEntryIndex, edge);

			indices[index + j] = indexList[faceStart + j];
			indices[normalEntryIndex] = normalIndex;

			j0 = j;
		}
		faceStart += faceList[i];
		index += faceList[i] * 2 + 3;
	}

	ndEdgeMap::Iterator iter(edgeMap);
	for (iter.Begin(); iter; iter++)
	{
		ndEdgeMap::dNode* const edgeNode = iter.GetNode();
		if (edgeNode->GetInfo() != -1)
		{
			ndEdge twin(iter.GetKey());
			dSwap(twin.m_i0, twin.m_i1);
			ndEdgeMap::dNode* const twinNode = edgeMap.Find(twin);
			if (twinNode)
			{
				dInt32 i0 = edgeNode->GetInfo();
				dInt32 i1 = twinNode->GetInfo();
				dSwap(indices[i0], indices[i1]);
				twinNode->GetInfo() = -1;
			}
		}
		edgeNode->GetInfo() = -1;
	}


	dInt32 faceCount0 = 0;
	dInt32 faceIndexCount0 = 0;
	dInt32 faceIndexCount1 = 0;
	dInt32 stride = sizeof(dVector) / sizeof(dFloat32);
	
	dInt32* const address = data->m_meshData.m_globalFaceIndexStart;
	dFloat32* const hitDistance = data->m_meshData.m_globalHitDistance;
	if (data->m_doContinuesCollisionTest) 
	{
		dAssert(0);
		//dFastRay ray(dVector::m_zero, data->m_boxDistanceTravelInMeshSpace);
		//for (dInt32 i = 0; i < faceCount; i++) 
		//{
		//	const dInt32* const indexArray = &indices[faceIndexCount1];
		//	const dVector& faceNormal = vertex[indexArray[4]];
		//	dFloat32 dist = data->PolygonBoxRayDistance(faceNormal, 3, indexArray, stride, &vertex[0].m_x, ray);
		//	if (dist < dFloat32(1.0f)) 
		//	{
		//		hitDistance[faceCount0] = dist;
		//		address[faceCount0] = faceIndexCount0;
		//		memcpy(&indices[faceIndexCount0], indexArray, 9 * sizeof(dInt32));
		//		faceCount0++;
		//		faceIndexCount0 += 9;
		//	}
		//	faceIndexCount1 += 9;
		//}
	}
	else 
	{
		for (dInt32 i = 0; i < faceList.GetCount(); i++)
		{
			const dInt32 vertexCount = faceIndexCount[i];
			const dInt32* const indexArray = &indices[faceIndexCount1];
			const dVector& faceNormal = vertex[indexArray[vertexCount + 1]];
			dFloat32 dist = data->PolygonBoxDistance(faceNormal, vertexCount, indexArray, stride, &vertex[0].m_x);
			if (dist > dFloat32(0.0f)) 
			{
				hitDistance[faceCount0] = dist;
				address[faceCount0] = faceIndexCount0;
				memcpy(&indices[faceIndexCount0], indexArray, (vertexCount * 2 + 3) * sizeof(dInt32));
				faceCount0++;
				faceIndexCount0 += vertexCount * 2 + 3;
			}
			faceIndexCount1 += vertexCount * 2 + 3;
		}
	}
	
	if (faceCount0) 
	{
		// initialize the callback data structure
		data->m_faceCount = faceCount0;
		data->m_vertex = &vertex[0].m_x;
		data->m_faceVertexIndex = indices;
		data->m_faceIndexStart = address;
		data->m_hitDistance = hitDistance;
		data->m_faceIndexCount = faceIndexCount;
		data->m_vertexStrideInBytes = sizeof(dVector);
	}
	
}

