#pragma once

#include "BasicActors.h"
#include "Timer.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	using namespace std;

	//pyramid vertices
	static PxVec3 pyramid_verts[] = {PxVec3(0,1,0), PxVec3(1,0,0), PxVec3(-1,0,0), PxVec3(0,0,1), PxVec3(0,0,-1)};

	//pyramid triangles: a list of three vertices for each triangle e.g. the first triangle consists of vertices 1, 4 and 0
	//vertices have to be specified in a counter-clockwise order to assure the correct shading in rendering
	static PxU32 pyramid_trigs[] = {1, 4, 0, 3, 1, 0, 2, 3, 0, 4, 2, 0, 3, 2, 1, 2, 4, 1};

	class Pyramid : public ConvexMesh
	{
	public:
		Pyramid(PxTransform pose=PxTransform(PxIdentity), PxReal density=1.f) : ConvexMesh(vector<PxVec3>(begin(pyramid_verts),end(pyramid_verts)), pose, density)
		{
		}
	};

	class PyramidStatic : public TriangleMesh
	{
	public:
		PyramidStatic(PxTransform pose=PxTransform(PxIdentity)) : TriangleMesh(vector<PxVec3>(begin(pyramid_verts),end(pyramid_verts)), vector<PxU32>(begin(pyramid_trigs),end(pyramid_trigs)), pose)
		{
		}
	};

	///Custom scene class
	class MyScene : public Scene
	{
		Plane* plane;
		Box* boxes[100];
		Sphere* ball;
		CompoundObject* compound;
		CompoundCapsule* compCapsule;
		CompoundGun* compGun;
		Box* bullet;

		SZ_HighResTimer* timer;
		
		PxRigidDynamic* proj;

		Box* createBox(PxTransform transform, PxVec3 dimensions = PxVec3(0.5f, 0.5f, 0.5f), PxReal density = 1.0f)
		{
			Box* box = new Box(transform, dimensions, density);
			box->Color(PxVec3(0.0f, 0.1f, 1.0f));
			Add(box);

			return box;
		}

	public:
		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit() 
		{
			SetVisualisation();			

			GetMaterial()->setDynamicFriction(.2f);

			// create plane
			plane = new Plane();
			plane->Color(PxVec3(210.f/255.f,210.f/255.f,210.f/255.f));
			Add(plane);

			// create ball
			//ball = new Sphere(PxTransform(PxVec3(-2.0f, 0.5, 1.0f)));
			//ball->Color(PxVec3(0.0f, 1.0f, 0.0f));
			//Add(ball);

			// wall of boxes
			float PositionX = -5.0f;
			float PositionY = 0.5f;
			float PositionZ = -5.0f;
			PxVec3 colour = PxVec3(0.0f, 0.0f, 1.0f);
			int boxIndex = 0;
			for (int i = 1; i <= 100; i++)
			{
				boxes[boxIndex] = createBox(PxTransform(PxVec3(PositionX, PositionY, PositionZ)));
				boxes[boxIndex]->Color(colour);
				PositionX += 1.0f;
			
				if (i % 10 == 0)
				{
					PositionY += 1.0f;
					PositionX = -5.0f;
					//colour = colour + PxVec3(0.0f, 0.05f, -0.05f);

					if (colour == PxVec3(0.0f, 0.0f, 1.0f))
					{
						colour = PxVec3(1.0f, 0.0f, 0.0f);
					}
					else if (colour == PxVec3(1.0f, 0.0f, 0.0f))
					{
						colour = PxVec3(0.0f, 0.0f, 1.0f);
					}
				}
				boxIndex++;
			}

			// create compound object
			compound = new CompoundObject(PxTransform(PxVec3(2.0f, 0.5f, 3.0f)));
			compound->Color(PxVec3(1.0f, 0.5f, 0.0f));
			Add(compound);

			// create compound capsule
			//compCapsule = new CompoundCapsule(PxTransform(PxVec3(0.0f, 1.0f, -10.0f)));
			//Add(compCapsule);

			// create compound gun
			compGun = new CompoundGun(PxTransform(-0.5f, 1.5f, 5.0f));
			compGun->Color(PxVec3(1.0f, 0.5f, 0.0f));
			Add(compGun);

			// create bullet
			bullet = new Box(PxTransform(-0.5f, 1.5f, 4.0f));
			bullet->Color(PxVec3(0.0f, 1.0f, 0.0f));
			((PxActor*)bullet->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
			Add(bullet);
		}

		// ********** UPDATE **********

		//PxVec3 compoundMove = PxVec3(0.0f, 0.0f, 0.0f);

		int index = 0;

		//Custom udpate function
		virtual void CustomUpdate() 
		{

			//PxTransform pose = ((PxRigidBody*)compound->Get())->getGlobalPose();
			//pose = PxTransform(compoundMove);
			//((PxRigidBody*)compound->Get())->setGlobalPose(pose);
			////getting and setting global pose of compound shape
			//compoundMove = compoundMove + PxVec3(0.0f, 0.0f, -0.1f);
			//// moving the vec3 backward

			((PxRigidBody*)compound->Get())->addForce(PxVec3(0.0f, 200.0f, -500.0f));
			// getting the rigid body of compound and adding force

			//((PxRigidBody*)ball->Get())->addForce(PxVec3(0.0f, 0.0f, -500.0f));

			//shooting the bullet
			((PxRigidBody*)bullet->Get())->addForce(PxVec3(0.0f, 0.0f, -100.0f));

			//PxTransform bulletPose = ((PxRigidBody*)bullet->Get())->getGlobalPose();

			if (index > 200)
			{
				 PxTransform bulletPose = PxTransform(PxVec3(-0.5f, 1.5, 4.0f));
				((PxRigidBody*)bullet->Get())->setGlobalPose(bulletPose);
				//timer->resetChronoTimer();

				index = 0;
			}

			index++;
		}

		
	};
}



// PhysX Documentation
// https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/apireference/files/main.html