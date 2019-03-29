#pragma once

#include "PhysicsEngine.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	///Compound class
	class CompoundObject : public DynamicActor
	{
	public:
		CompoundObject(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f) : DynamicActor(pose)
		{
			CreateShape(PxBoxGeometry(dimensions), density);
			CreateShape(PxBoxGeometry(dimensions), density);
			CreateShape(PxBoxGeometry(dimensions), density);
			CreateShape(PxBoxGeometry(dimensions), density);
			CreateShape(PxBoxGeometry(dimensions), density);
			CreateShape(PxBoxGeometry(dimensions * 2), density);

			GetShape(1)->setLocalPose(PxTransform(PxVec3(1.0f, 0.0f, 0.0f))); // box 1 to the right
			GetShape(2)->setLocalPose(PxTransform(PxVec3(2.0f, 0.0f, 0.0f))); // box 2 to the right
			GetShape(3)->setLocalPose(PxTransform(PxVec3(2.0f, 1.0f, 0.0f))); // box 2 to the rigth and 1 up 
			GetShape(4)->setLocalPose(PxTransform(PxVec3(2.0f, 2.0f, 0.0f))); // box 2 to the right and 2 up

			GetShape(5)->setLocalPose(PxTransform(PxVec3(2.0f, 3.0f, 0.0f))); // bigger box 2 to the right and 3 up
			//GetShape(5)->setLocalPose(PxTransform(PxQuat(1.0f, PxVec3(0.0f, 1.0f, 0.0f)))); // rotate box 1 radian about Y axis
		}
	};

	///Compound Capsule
	class CompoundCapsule : public DynamicActor
	{
	public:
		CompoundCapsule(const PxTransform& pose = PxTransform(PxIdentity), PxVec2 dimensions = PxVec2(1.f, 1.f), PxReal density = 1.f) : DynamicActor(pose)
		{
			CreateShape(PxCapsuleGeometry(dimensions.x, dimensions.y), density);
			CreateShape(PxCapsuleGeometry(dimensions.x, dimensions.y), density);

			GetShape(1)->setLocalPose(PxTransform(PxVec3(0.0f, 2.0f, 0.0f))); // capsule 2 to the right
		}
	};

	class CompoundGun : public DynamicActor
	{
	public:
		CompoundGun(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(0.5f, 0.5f, 0.5f), PxReal density = 1.f) : DynamicActor(pose)
		{
			CreateShape(PxBoxGeometry(PxVec3(dimensions.x * 3, dimensions.y, dimensions.z)), density);
			
			for (int i = 0; i < 4; i++)
			{
				CreateShape(PxBoxGeometry(dimensions), density);
			}

			GetShape(1)->setLocalPose(PxTransform(PxVec3(1.0f, 0.0f, -1.0f)));
			GetShape(2)->setLocalPose(PxTransform(PxVec3(-1.0f, 0.0f, -1.0f)));
			GetShape(3)->setLocalPose(PxTransform(PxVec3(0.0f, -1.0f, 0.0f)));
			
		}
	};

	///Plane class
	class Plane : public StaticActor
	{
	public:
		//A plane with default paramters: XZ plane centred at (0,0,0)
		Plane(PxVec3 normal=PxVec3(0.f, 1.f, 0.f), PxReal distance=0.f) 
			: StaticActor(PxTransformFromPlaneEquation(PxPlane(normal, distance)))
		{
			CreateShape(PxPlaneGeometry());
		}
	};

	///Sphere class
	class Sphere : public DynamicActor
	{
	public:
		//a sphere with default parameters:
		// - pose in 0,0,0
		// - dimensions: 1m
		// - denisty: 1kg/m^3
		Sphere(const PxTransform& pose=PxTransform(PxIdentity), PxReal radius=1.f, PxReal density=1.f) 
			: DynamicActor(pose)
		{ 
			CreateShape(PxSphereGeometry(radius), density);
		}
	};

	///Box class
	class Box : public DynamicActor
	{
	public:
		//a Box with default parameters:
		// - pose in 0,0,0
		// - dimensions: 1m x 1m x 1m
		// - denisty: 1kg/m^3
		Box(const PxTransform& pose=PxTransform(PxIdentity), PxVec3 dimensions=PxVec3(.5f,.5f,.5f), PxReal density=1.f) 
			: DynamicActor(pose)
		{ 
			CreateShape(PxBoxGeometry(dimensions), density);
		}
	};

	class Capsule : public DynamicActor
	{
	public:
		Capsule(const PxTransform& pose=PxTransform(PxIdentity), PxVec2 dimensions=PxVec2(1.f,1.f), PxReal density=1.f) 
			: DynamicActor(pose)
		{
			CreateShape(PxCapsuleGeometry(dimensions.x, dimensions.y), density);
		}
	};

	///The ConvexMesh class
	class ConvexMesh : public DynamicActor
	{
	public:
		//constructor
		ConvexMesh(const std::vector<PxVec3>& verts, const PxTransform& pose=PxTransform(PxIdentity), PxReal density=1.f)
			: DynamicActor(pose)
		{
			PxConvexMeshDesc mesh_desc;
			mesh_desc.points.count = (PxU32)verts.size();
			mesh_desc.points.stride = sizeof(PxVec3);
			mesh_desc.points.data = &verts.front();
			mesh_desc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
			mesh_desc.vertexLimit = 256;

			CreateShape(PxConvexMeshGeometry(CookMesh(mesh_desc)), density);
		}

		//mesh cooking (preparation)
		PxConvexMesh* CookMesh(const PxConvexMeshDesc& mesh_desc)
		{
			PxDefaultMemoryOutputStream stream;

			if(!GetCooking()->cookConvexMesh(mesh_desc, stream))
				throw new Exception("ConvexMesh::CookMesh, cooking failed.");

			PxDefaultMemoryInputData input(stream.getData(), stream.getSize());

			return GetPhysics()->createConvexMesh(input);
		}
	};

	///The TriangleMesh class
	class TriangleMesh : public StaticActor
	{
	public:
		//constructor
		TriangleMesh(const std::vector<PxVec3>& verts, const std::vector<PxU32>& trigs, const PxTransform& pose=PxTransform(PxIdentity))
			: StaticActor(pose)
		{
			PxTriangleMeshDesc mesh_desc;
			mesh_desc.points.count = (PxU32)verts.size();
			mesh_desc.points.stride = sizeof(PxVec3);
			mesh_desc.points.data = &verts.front();
			mesh_desc.triangles.count = (PxU32)trigs.size();
			mesh_desc.triangles.stride = 3*sizeof(PxU32);
			mesh_desc.triangles.data = &trigs.front();

			CreateShape(PxTriangleMeshGeometry(CookMesh(mesh_desc)));
		}

		//mesh cooking (preparation)
		PxTriangleMesh* CookMesh(const PxTriangleMeshDesc& mesh_desc)
		{
			PxDefaultMemoryOutputStream stream;

			if(!GetCooking()->cookTriangleMesh(mesh_desc, stream))
				throw new Exception("TriangleMesh::CookMesh, cooking failed.");

			PxDefaultMemoryInputData input(stream.getData(), stream.getSize());

			return GetPhysics()->createTriangleMesh(input);
		}
	};
}