/* Copyright (c) <2003-2019> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "ndSandboxStdafx.h"
#include "ndTargaToOpenGl.h"
#include "ndDemoMesh.h"
#include "ndDemoEntityManager.h"
#include "ndDemoCamera.h"
#include "ndPhysicsUtils.h"
#include "ndDebugDisplay.h"
#include "ndHeightFieldPrimitive.h"

#define GAUSSIAN_BELL 2
//#define GAUSSIAN_BELL	1
#define TILE_SIZE	128

// uncomment this to remove the nasty crease generated by mid point displacement
#define RE_SAMPLE_CORNER

//class HeightfieldMap: public ndDemoMesh
class HeightfieldMap
{
	public:
	
	static dFloat32 Guassian (dFloat32 freq)
	{
		dFloat32 r = 0.0f;
		int maxCount = 2 * GAUSSIAN_BELL + 1;
		for (int i = 0; i < maxCount; i ++) {
			dFloat32 t = 2.0f * dFloat32 (dRand()) / dFloat32(dRAND_MAX) - 1.0f;
			r += t;
		}
freq *= 0.5f;
		return freq * r / maxCount;
	}

/*
	static void SetBaseHeight (dFloat32* const elevation, int size)
	{
		dFloat32 minhigh = 1.0e10;
		for (int i = 0; i < size * size; i ++) {
			if (elevation[i] < minhigh) {
				minhigh = elevation[i];
			}
		}
		for (int i = 0; i < size * size; i ++) {
			elevation[i] -= minhigh;
		}
	}

	static void MakeCalderas (dFloat32* const elevation, int size, dFloat32 baseHigh, dFloat32 roughness)
	{
		for (int i = 0; i < size * size; i ++) {
			//dFloat32 h =  baseHigh + Guassian (roughness);
			dFloat32 h =  baseHigh;
			if (elevation[i] > h) {
				elevation[i] = h - (elevation[i] - h);
			}
		}
	}
*/

	static void MakeMap (dFloat32* const elevation, dFloat32** const map, int size)
	{
		for (int i = 0; i < size; i ++) {
			map[i] = &elevation[i * size];
		}
	}

	static void MakeMap (dVector* const normal, dVector** const map, int size)
	{
		for (int i = 0; i < size; i ++) {
			map[i] = &normal[i * size];
		}
	}


/*
	static DemoMesh____* CreateVisualMesh (dFloat32* const elevation, int size, dFloat32 cellSize, dFloat32 texelsDensity)
	{
		dVector* const normals = new dVector [size * size];

		dFloat32* elevationMap[4096];
		dVector* normalMap[4096];

		MakeMap (elevation, elevationMap, size);
		MakeMap (normals, normalMap, size);

		memset (normals, 0, (size * size) * sizeof (dVector));
		for (int z = 0; z < size - 1; z ++) {
			for (int x = 0; x < size - 1; x ++) {
				dVector p0 ((x + 0) * cellSize, elevationMap[z + 0][x + 0], (z + 0) * cellSize);
				dVector p1 ((x + 1) * cellSize, elevationMap[z + 0][x + 1], (z + 0) * cellSize);
				dVector p2 ((x + 1) * cellSize, elevationMap[z + 1][x + 1], (z + 1) * cellSize);
				dVector p3 ((x + 0) * cellSize, elevationMap[z + 1][x + 0], (z + 1) * cellSize);

				dVector e10 (p1 - p0);
				dVector e20 (p2 - p0);
				dVector n0 (e20 * e10);
				n0 = n0.Scale ( 1.0f / dSqrt (n0 % n0));
				normalMap [z + 0][x + 0] += n0;
				normalMap [z + 0][x + 1] += n0;
				normalMap [z + 1][x + 1] += n0;

				dVector e30 (p3 - p0);
				dVector n1 (e30 * e20);
				n1 = n1.Scale ( 1.0f / dSqrt (n1 % n1));
				normalMap [z + 0][x + 0] += n1;
				normalMap [z + 1][x + 0] += n1;
				normalMap [z + 1][x + 1] += n1;
			}
		}

		for (int i = 0; i < size * size; i ++) {
			normals[i] = normals[i].Scale (1.0f / sqrtf (normals[i] % normals[i]));
		}


		HeightfieldMap* const mesh = new HeightfieldMap () ;
		mesh->AllocVertexData (size * size);

		dFloat32* const vertex = mesh->m_vertex;
		dFloat32* const normal = mesh->m_normal;
		dFloat32* const uv = mesh->m_uv;

		int index = 0;
		for (int z = 0; z < size; z ++) {
			for (int x = 0; x < size; x ++) {
				vertex[index * 3 + 0] = x * cellSize;
				vertex[index * 3 + 1] = elevationMap[z][x];
				vertex[index * 3 + 2] = z * cellSize;

				normal[index * 3 + 0] = normalMap[z][x].m_x;
				normal[index * 3 + 1] = normalMap[z][x].m_y;
				normal[index * 3 + 2] = normalMap[z][x].m_z;

				uv[index * 2 + 0] = x * texelsDensity;
				uv[index * 2 + 1] = z * texelsDensity;
				index ++;
			}
		}


		int segmentsCount = (size - 1) / TILE_SIZE;
		for (int z0 = 0; z0 < segmentsCount; z0 ++) {
			int z = z0 * TILE_SIZE;
			for (int x0 = 0; x0 < segmentsCount; x0 ++ ) {
				int x = x0 * TILE_SIZE;

				DemoSubMesh* const tile = mesh->AddSubMesh();
				tile->AllocIndexData (TILE_SIZE * TILE_SIZE * 6);
				unsigned* const indexes = tile->m_indexes;

				strcpy (tile->m_textureName, "grassanddirt.tga");
				tile->m_textureHandle = LoadTexture(tile->m_textureName);

				int index = 0;
				int x1 = x + TILE_SIZE;
				int z1 = z + TILE_SIZE;
				for (int z0 = z; z0 < z1; z0 ++) {
					for (int x0 = x; x0 < x1; x0 ++) {
						int i0 = x0 + 0 + (z0 + 0) * size;
						int i1 = x0 + 1 + (z0 + 0) * size;
						int i2 = x0 + 1 + (z0 + 1) * size;
						int i3 = x0 + 0 + (z0 + 1) * size;

						indexes[index + 0] = i0;
						indexes[index + 1] = i2;
						indexes[index + 2] = i1;

						indexes[index + 3] = i0;
						indexes[index + 4] = i3;
						indexes[index + 5] = i2;
						index += 6;
					}
				}
				index*=1;
			}
		}

		delete[] normals;
		return mesh;
	}
*/

	static void ApplySmoothFilter (dFloat32* const elevation, int size)
	{
		dFloat32* map0[4096 + 1];
		dFloat32* map1[4096 + 1];
		dFloat32* const buffer = new dFloat32 [size * size];

		MakeMap (elevation, map0, size);
		MakeMap (buffer, map1, size);

		for (int z = 0; z < size; z ++) {
			map1[z][0] = map0[z][0];
			map1[z][size - 1] = map0[z][size - 1];
			for (int x = 1; x < (size - 1); x ++) {
				map1[z][x] = map0[z][x-1] * 0.25f +  map0[z][x] * 0.5f + map0[z][x+1] * 0.25f; 
			}
		}

		for (int x = 0; x < size; x ++) {
			map0[0][x] = map1[0][x];
			map0[size - 1][x] = map1[size - 1][x];
			for (int z = 1; z < (size - 1); z ++) {
				map0[z][x] = map1[z-1][x] * 0.25f +  map1[z][x] * 0.5f + map1[z+1][x] * 0.25f; 
			}
		}

		delete[] buffer;
	}


	static dFloat32 GetElevation (int size, dFloat32 elevation, dFloat32 maxH, dFloat32 minH, dFloat32 roughness)
	{
		dFloat32 h = dFloat32 (pow (dFloat32 (size) * elevation, 1.0f + roughness));
		if (h > maxH) {
			h = maxH;	
		} else if (h < minH){
			h = minH;
		}
		return h;
	}

	static void MakeFractalTerrain (dFloat32* const elevation, int sizeInPowerOfTwos, dFloat32 elevationScale, dFloat32 roughness, dFloat32 maxElevation, dFloat32 minElevation)
	{
		dFloat32* map[4096 + 1];
		int size = (1 << sizeInPowerOfTwos) + 1;
		dAssert (size < int (sizeof (map) / sizeof map[0]));
		MakeMap (elevation, map, size);
		//for (int i = 0; i < (size + 1) * (size + 1); i++) {
		//	elevation[i] = -1.0e6f;
		//}

		for (int i = 0; i < size; i++) {
			map[0][i] = 0.0f;
			map[size - 1][i] = 0.0f;
			map[i][0] = 0.0f;
			map[i][size - 1] = 0.0f;
		}

		//dFloat32 f = GetElevation (size, elevationScale, maxElevation, minElevation, roughness);
		//map[0][0] = Guassian(f);
		//map[0][size-1] = Guassian(f);
		//map[size-1][0] = Guassian(f);
		//map[size-1][size-1] = Guassian(f);
		//map[0][0] = 0.0f;
		//map[0][size-1] = 0.0f;
		//map[size-1][0] = 0.0f;
		//map[size-1][size-1] = 0.0f;
		for (int frequency = size - 1; frequency > 1; frequency = frequency / 2 ) {
			dFloat32 h = GetElevation (frequency, elevationScale, maxElevation, minElevation, roughness);

			for(int y0 = 0; y0 < (size - frequency); y0 += frequency) {
				int y1 = y0 + frequency / 2;
				int y2 = y0 + frequency;

				for(int x0 = 0; x0 < (size - frequency); x0 += frequency) {
					int x1 = x0 + frequency / 2;
					int x2 = x0 + frequency;

					map[y1][x1] = (map[y0][x0] + map[y0][x2] + map[y2][x0] + map[y2][x2]) * 0.25f + Guassian(h);

					if (y0) {
						map[y0][x1] = (map[y0][x0] + map[y0][x2]) * 0.5f + Guassian(h);
					}
					if (y2 != size - 1) {
						map[y2][x1] = (map[y2][x0] + map[y2][x2]) * 0.5f + Guassian(h);
					}

					if (x0) {
						map[y1][x0] = (map[y0][x0] + map[y2][x0]) * 0.5f + Guassian(h);
					}

					if (x2 != size - 1) {
						map[y1][x2] = (map[y0][x2] + map[y2][x2]) * 0.5f + Guassian(h);
					}

					// this trick eliminate the creases 
					#ifdef RE_SAMPLE_CORNER
						//map[y0][x0] = (map[y0][x1] + map[y1][x0]) * 0.5f + Guassian(h);
						//map[y0][x2] = (map[y0][x1] + map[y1][x2]) * 0.5f + Guassian(h);
						//map[y2][x0] = (map[y1][x0] + map[y2][x1]) * 0.5f + Guassian(h);
						//map[y2][x2] = (map[y2][x1] + map[y1][x2]) * 0.5f + Guassian(h);
					#endif
				}
			}
		}
	}

	#ifdef USE_TEST_ALL_FACE_USER_RAYCAST_CALLBACK
		static dFloat32 AllRayHitCallback (const NewtonBody* const body, const NewtonCollision* const treeCollision, dFloat32 intersection, dFloat32* const normal, int faceId, void* const usedData)
		{
			return intersection;
		}
	#endif


	static NewtonBody* CreateHeightFieldTerrain (ndDemoEntityManager* const scene, int sizeInPowerOfTwos, dFloat32 cellSize, dFloat32 elevationScale, dFloat32 roughness, dFloat32 maxElevation, dFloat32 minElevation)
	{
		int size = (1 << sizeInPowerOfTwos) + 1 ;
		dFloat32* const elevation = new dFloat32 [size * size];
		//MakeFractalTerrain (elevation, sizeInPowerOfTwos, elevationScale, roughness, maxElevation, minElevation);
		MakeFractalTerrain (elevation, sizeInPowerOfTwos, elevationScale, roughness, maxElevation, minElevation);

		for (int i = 0; i < 4; i ++) {
			ApplySmoothFilter (elevation, size);
		}

		//SetBaseHeight (elevation, size);
		// apply simple calderas
		//		MakeCalderas (elevation, size, maxElevation * 0.7f, maxElevation * 0.1f);

		//	// create the visual mesh
		ndDemoMesh* const mesh = new ndDemoMesh ("terrain", scene->GetShaderCache(), elevation, size, cellSize, 1.0f/4.0f, TILE_SIZE);

		ndDemoEntity* const entity = new ndDemoEntity(dGetIdentityMatrix(), NULL);
		scene->Append (entity);
		entity->SetMesh(mesh, dGetIdentityMatrix());
		mesh->Release();

		// create the height field collision and rigid body

		// create the attribute map
		int width = size;
		int height = size;
		char* const attibutes = new char [size * size];
		memset (attibutes, 0, width * height * sizeof (char));
		NewtonCollision* collision = NewtonCreateHeightFieldCollision (scene->GetNewton(), width, height, 1, 0, elevation, attibutes, 1.0f, cellSize, cellSize, 0);

#ifdef USE_STATIC_MESHES_DEBUG_COLLISION
		NewtonStaticCollisionSetDebugCallback (collision, ShowMeshCollidingFaces);
#endif

		NewtonCollisionInfoRecord collisionInfo;
		// keep the compiler happy
		memset (&collisionInfo, 0, sizeof (NewtonCollisionInfoRecord));
		NewtonCollisionGetInfo (collision, &collisionInfo);

		width = collisionInfo.m_heightField.m_width;
		height = collisionInfo.m_heightField.m_height;
		//elevations = collisionInfo.m_heightField.m_elevation;

		dVector boxP0; 
		dVector boxP1; 
		// get the position of the aabb of this geometry
		dMatrix matrix (entity->GetCurrentMatrix());
		NewtonCollisionCalculateAABB (collision, &matrix[0][0], &boxP0.m_x, &boxP1.m_x); 
		matrix.m_posit = (boxP0 + boxP1).Scale (-0.5f);
		matrix.m_posit.m_w = 1.0f;
		//SetMatrix (matrix);
		entity->ResetMatrix (*scene, matrix);

		// create the terrainBody rigid body
		NewtonBody* const terrainBody = NewtonCreateDynamicBody(scene->GetNewton(), collision, &matrix[0][0]);

		// release the collision tree (this way the application does not have to do book keeping of Newton objects
		NewtonDestroyCollision (collision);

		// in newton 300 collision are instance, you need to ready it after you create a body, if you want to male call on the instance
		collision = NewtonBodyGetCollision(terrainBody);
#if 0
		// uncomment this to test horizontal displacement
		unsigned short* const horizontalDisplacemnet = new unsigned short[size * size];
		for (int i = 0; i < size * size; i++) {
			horizontalDisplacemnet[i] = dRand();
		}
		NewtonHeightFieldSetHorizontalDisplacement(collision, horizontalDisplacemnet, 0.02f);
		delete horizontalDisplacemnet;
#endif

		// save the pointer to the graphic object with the body.
		NewtonBodySetUserData (terrainBody, entity);

		// set the global position of this body
		//	NewtonBodySetMatrix (m_terrainBody, &matrix[0][0]); 

		// set the destructor for this object
		//NewtonBodySetDestructorCallback (terrainBody, Destructor);

		// get the position of the aabb of this geometry
		//NewtonCollisionCalculateAABB (collision, &matrix[0][0], &boxP0.m_x, &boxP1.m_x); 

#ifdef USE_TEST_ALL_FACE_USER_RAYCAST_CALLBACK
		// set a ray cast callback for all face ray cast 
		NewtonTreeCollisionSetUserRayCastCallback (collision, AllRayHitCallback);
		
		dVector p0 (0,  100, 0, 0);
		dVector p1 (0, -100, 0, 0);
		dVector normal;
		dLong id;
		dFloat32 parameter;
		parameter = NewtonCollisionRayCast (collision, &p0[0], &p1[0], &normal[0], &id);
#endif

		delete[] attibutes;
		delete[] elevation;
		return terrainBody;
	}
};

NewtonBody* CreateHeightFieldTerrain (ndDemoEntityManager* const scene, int sizeInPowerOfTwos, dFloat32 cellSize, dFloat32 elevationScale, dFloat32 roughness, dFloat32 maxElevation, dFloat32 minElevation)
{
	return HeightfieldMap::CreateHeightFieldTerrain (scene, sizeInPowerOfTwos, cellSize, elevationScale, roughness, maxElevation, minElevation);
}
