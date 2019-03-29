#pragma once

#include "BasicActors.h"
#include "Timer.h"
#include "HighResTimer.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	using namespace std;

	static int score = 0;
	static int updateLoopTime;

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = {PxVec3(46.f/255.f,9.f/255.f,39.f/255.f),PxVec3(217.f/255.f,0.f/255.f,0.f/255.f),
		PxVec3(255.f/255.f,45.f/255.f,0.f/255.f),PxVec3(255.f/255.f,140.f/255.f,54.f/255.f),PxVec3(4.f/255.f,117.f/255.f,111.f/255.f)};

	//pyramid vertices
	static PxVec3 pyramid_verts[] = {PxVec3(0,1,0), PxVec3(1,0,0), PxVec3(-1,0,0), PxVec3(0,0,1), PxVec3(0,0,-1)};
	//pyramid triangles: a list of three vertices for each triangle e.g. the first triangle consists of vertices 1, 4 and 0
	//vertices have to be specified in a counter-clockwise order to assure the correct shading in rendering
	static PxU32 pyramid_trigs[] = {1, 4, 0, 3, 1, 0, 2, 3, 0, 4, 2, 0, 3, 2, 1, 2, 4, 1};
	class Pyramid : public ConvexMesh
	{
	public:
		Pyramid(PxTransform pose=PxTransform(PxIdentity), PxReal density=1.f) :	ConvexMesh(vector<PxVec3>(begin(pyramid_verts),end(pyramid_verts)), pose, density)
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

	struct FilterGroup
	{
		enum Enum
		{
			ACTOR0		= (1 << 0),
			ACTOR1		= (1 << 1),
			ACTOR2		= (1 << 2),
			ACTOR3		= (1 << 3)
			//add more if you need
		};
	};

	///An example class showing the use of springs (distance joints).
	class Trampoline
	{
		vector<DistanceJoint*> springs;
		Box *bottom, *top;

	public:
		Trampoline(const PxVec3& dimensions=PxVec3(1.0f,1.0f,1.0f), PxReal stiffness=1.0f, PxReal damping=1.0f)
		{
			PxReal thickness = 0.1f;
			bottom = new Box(PxTransform(PxVec3(-35.0f, thickness, -75.0f)),PxVec3(dimensions.x, thickness, dimensions.z));
			bottom->SetKinematic(true);
			top = new Box(PxTransform(PxVec3(-35.0f, dimensions.y + thickness, -75.0f)),PxVec3(dimensions.x, thickness, dimensions.z));
			springs.resize(4);
			springs[0] = new DistanceJoint(bottom, PxTransform(PxVec3(dimensions.x, thickness, dimensions.z)), top, PxTransform(PxVec3(dimensions.x, -dimensions.y, dimensions.z)));
			springs[1] = new DistanceJoint(bottom, PxTransform(PxVec3(dimensions.x, thickness, -dimensions.z)), top, PxTransform(PxVec3(dimensions.x, -dimensions.y, -dimensions.z)));
			springs[2] = new DistanceJoint(bottom, PxTransform(PxVec3(-dimensions.x, thickness, dimensions.z)), top, PxTransform(PxVec3(-dimensions.x, -dimensions.y, dimensions.z)));
			springs[3] = new DistanceJoint(bottom, PxTransform(PxVec3(-dimensions.x, thickness, -dimensions.z)), top, PxTransform(PxVec3(-dimensions.x, -dimensions.y, -dimensions.z)));

			for (unsigned int i = 0; i < springs.size(); i++)
			{
				springs[i]->Stiffness(stiffness);
				springs[i]->Damping(damping);
			}
		}

		void AddToScene(Scene* scene)
		{
			scene->Add(bottom);
			scene->Add(top);
		}

		~Trampoline()
		{
			for (unsigned int i = 0; i < springs.size(); i++)
				delete springs[i];
		}
	};

	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		bool goal;
		bool wallHit;
		//an example variable that will be checked in the main simulation loop
		bool trigger;
		PxTransform ballPos;

		MySimulationEventCallback() : trigger(false), goal(false), wallHit(false) {}

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) 
		{
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
				{
					//check if eNOTIFY_TOUCH_FOUND trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					{
						//cerr << "onTrigger::eNOTIFY_TOUCH_FOUND" << endl;
						// get name of other actor
						string actorName = pairs[i].otherActor->getName();

						// check for ball collision
						if (actorName == "BALL")
						{
							//cout << "GOOOALLLLLLLAAAAZZZZZZZOOOOOOOO!!!!!!!!!!!!!!!!" << endl;
							goal = true;
							score++;
						}

						trigger = true;
					}
					//check if eNOTIFY_TOUCH_LOST trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
					{
						//cerr << "onTrigger::eNOTIFY_TOUCH_LOST" << endl;
						trigger = false;

						goal = false;
					}
				}
			}
		}

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs) 
		{
			cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;

			string actor0 = pairHeader.actors[0]->getName();
			string actor1 = pairHeader.actors[1]->getName();

			if ((actor0 == "WALL" && actor1 == "BALL") || (actor0 == "BALL" && actor1 == "WALL"))
			{
				if (actor0 == "BALL")
				{
					ballPos = pairHeader.actors[0]->getGlobalPose();
				}
				else
				{
					ballPos = pairHeader.actors[1]->getGlobalPose();
				}
				// get ball pos
				
				
				wallHit = true;
			}

			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				//check eNOTIFY_TOUCH_FOUND
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					cerr << "onContact::eNOTIFY_TOUCH_FOUND" << endl;
				}
				//check eNOTIFY_TOUCH_LOST
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					cerr << "onContact::eNOTIFY_TOUCH_LOST" << endl;

				}
			}
		}

		virtual void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
		virtual void onWake(PxActor **actors, PxU32 count) {}
		virtual void onSleep(PxActor **actors, PxU32 count) {}
	};

	//A simple filter shader based on PxDefaultSimulationFilterShader - without group filtering
	static PxFilterFlags CustomFilterShader( PxFilterObjectAttributes attributes0,	PxFilterData filterData0, PxFilterObjectAttributes attributes1,	PxFilterData filterData1, PxPairFlags& pairFlags,	const void* constantBlock,	PxU32 constantBlockSize)
	{
		// let triggers through
		if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT;
		//enable continous collision detection
		//pairFlags |= PxPairFlag::eCCD_LINEAR;
		
		
		//customise collision filtering here
		//e.g.

		// trigger the contact callback for pairs (A,B) where 
		// the filtermask of A contains the ID of B and vice versa.
		if((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			//trigger onContact callback for this pair of objects
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
			//pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
		}

		return PxFilterFlags();
	};

	///Custom scene class
	class MyScene : public Scene
	{
		Box* goalCollision;
		MySimulationEventCallback* my_callback;
		Plane* plane;

		// timers
		SecTimer fgTimer;
		SecTimer ballTimer;
		SecTimer doorTimer;
		SecTimer cannonTimer;
		SecTimer goalEventTimer;
		SecTimer joustTimer;
		SecTimer dropTimer;

		// player
		CompoundPlayer* player;
		// goal
		CompoundGoal* goal;
		// field
		CompoundField* gameField;
		// door + wall
		CompoundWall* wall;
		Box* door;
		RevoluteJoint* doorHinge;
		Box* wallBox;

		// destroyable wall
		Box* destroyWall[400];
		int destroyWallSize = 400;
		
		// jousters
		int teamSize = 4;
		CompoundJoust* joustTeam1[4];
		CompoundJoust* joustTeam2[4];

		// ball
		Box* boxBall;
		compoundRugbyBall* rugbyBall;
		bool ballIsThere = false;

		// catapult
		CompoundCatapult* catapultBase;
		CompoundCatapultThrow* catapultThrow;
		RevoluteJoint* catapultJoint;
		bool fieldGoalBool = false;

		// cannons
		CompoundGun* cannon1;
		Sphere* cannon1proj;
		CompoundGun* cannon2;
		Sphere* cannon2proj;
		CompoundGun* cannon3;
		Sphere* cannon3proj;

		// flags
		Cloth* flag;
		Box* flagPole;

		// goal event objects
		Box* goalEventObjects1[5];
		Box* goalEventObjects2[5];

		// trampoline
		Trampoline* tramp;
		Box* drop;

		// materials
		PxMaterial* grass = CreateMaterial(0.1f, 0.1f, 0.5f); //static friction, dynamic friction, restitution
		PxMaterial* rugbyBallMat = CreateMaterial(0.1f, 0.1f, 1.2f); // higher restitituion for a bouncy ball
		PxMaterial* catapultMat = CreateMaterial(0.2f, 0.3f, 0.6f);
		// restitution of wood 
		// https://hypertextbook.com/facts/2006/restitution.shtml
		PxMaterial* trampMat = CreateMaterial(0.1f, 0.1f, 1.0f);
		PxMaterial* wallMat = CreateMaterial(0.1f, 0.1f, 0.2f);


	public:
		//specify your custom filter shader here
		//PxDefaultSimulationFilterShader by default
		MyScene() : Scene(CustomFilterShader) 
		{

		};

		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit()
		{
			fgTimer.resetChronoTimer();
			ballTimer.resetChronoTimer();
			cannonTimer.resetChronoTimer();
			joustTimer.resetChronoTimer();
			doorTimer.resetChronoTimer();
			goalEventTimer.resetChronoTimer();
			dropTimer.resetChronoTimer();
			// resetting timers

			SetVisualisation();

			//GetMaterial()->setDynamicFriction(.2f);

			///Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);


			// ********** RUGBY GAME **********
			plane = new Plane();
			plane->Material(grass); // adding material to plane
			plane->Color(PxVec3(0.0f, 0.4f, 0.0f));
			plane->Name("PLANE");
			Add(plane);

			player = new CompoundPlayer(PxTransform(PxVec3(10.0f, 3.0f, 0.0f)));
			player->Color(PxVec3(0.6f, 0.0f, 0.0f));
			player->Name("PLAYER");
			Add(player);
			

			// ********** GAME FIELD **********
			//createField();
			gameField = new CompoundField(PxTransform(PxVec3(0.0f, 0.0f, 0.0f)));
			gameField->SetKinematic(true);
			gameField->Name("FIELD");
			Add(gameField);

			// ********** GOAL **********
			goal = new CompoundGoal(PxTransform(PxVec3(0.0f, 6.0f, -100.0f)));
			goal->Color(PxVec3(0.0f, 1.0f, 1.0f));
			goal->SetKinematic(true);
			goal->Name("GOAL");
			Add(goal);

			// setting the trigger object
			goalCollision = new Box(PxTransform(PxVec3(0.0f, 18.0f, -100.0f)), PxVec3(4.0f, 6.0f, 0.5f));
			goalCollision->Color(PxVec3(0.0f, 0.0f, 0.0f), 0.0f);
			goalCollision->SetKinematic(true);
			goalCollision->GetShape(0)->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			goalCollision->GetShape(0)->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			goalCollision->SetTrigger(true);
			goalCollision->Name("GOALCOLLISION");
			//((PxShape*)goalCollision->Get())->setFlag(PxShapeFlag::eVISUALIZATION, false);
			Add(goalCollision);

			// ********** BALL **********
			rugbyBall = new compoundRugbyBall(PxTransform(PxVec3(0.0f, 6.5f, -1.0f)));
			rugbyBall->Color(PxVec3(0.7f, 0.0f, 0.7f));
			rugbyBall->Material(rugbyBallMat);
			rugbyBall->Name("BALL");
			rugbyBall->SetupFiltering(FilterGroup::ACTOR1, FilterGroup::ACTOR0);
			rugbyBall->SetKinematic(true);
			ballIsThere = true;
			Add(rugbyBall);
			//volume of rugby ball approx, 0.3m x 0.15m x 0.15m = 0.00675 kg/m^3
			//mass of rugby ball approx, 0.5kg 
			//density of rugby ball, m / v = p, 0.5kg / 0.00675m^3 = 74 kg/m^3

			// ********** CATAPULT **********
			catapultBase = new CompoundCatapult(PxTransform(PxVec3(0.0f, 5.5f, -5.0f)), PxVec3(0.5f, 0.5f, 0.5f), 100.0f);
			catapultBase->Color(PxVec3(1.0f, 0.0f, 0.0f));
			catapultBase->Material(catapultMat);
			catapultBase->SetKinematic(true);
			catapultBase->Name("BASE");
			Add(catapultBase);

			catapultThrow = new CompoundCatapultThrow(PxTransform(PxVec3(0.0f, 5.5f, -1.0f)), PxVec3(0.5f, 0.5f, 0.5f));
			catapultThrow->Color(PxVec3(0.6f, 0.2f, 0.0f));
			catapultThrow->SetKinematic(true);
			catapultThrow->Material(catapultMat);
			catapultThrow->Name("THROW");
			((PxActor*)catapultThrow->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
			Add(catapultThrow);

			// ********** JOINT **********
			catapultJoint = new RevoluteJoint(catapultBase, PxTransform(PxVec3(0.0f, 0.0f, 0.0f), PxQuat(2 * PxPi, PxVec3(1.0f, 0.0f, 0.0f))), catapultThrow, PxTransform(PxVec3(0.0f, 0.0f, -4.0f)));
			//catapultJoint->DriveVelocity(-12.0f);
			catapultJoint->SetLimits(180.0f, 90.0f);
		

			// ********** CANNONS **********
			cannon1 = new CompoundGun(PxTransform(PxVec3(-40.0f, 4.0f, -50.0f), PxQuat(3 * PxPi / 2, PxVec3(0.0f, 1.0f, 0.0f))), PxVec3(2.0f, 2.0f, 2.0f), 100.0f);
			cannon1->SetKinematic(true);
			cannon1->Color(PxVec3(0.7f, 0.1f, 0.2f));
			cannon1->Name("CANNON");
			Add(cannon1);

			cannon1proj = new Sphere(PxTransform(PxVec3(-36, 4.0f, -50.0f)), 2.0f, 5.0f);
			cannon1proj->Color(PxVec3(1.0f, 0.5f, 0.5f));
			cannon1proj->Name("PROJ");
			Add(cannon1proj);

			cannon2 = new CompoundGun(PxTransform(PxVec3(40.0f, 4.0f, -40.0f), PxQuat(PxPi / 2, PxVec3(0.0f, 1.0f, 0.0f))), PxVec3(2.0f, 2.0f, 2.0f), 100.0f);
			cannon2->SetKinematic(true);
			cannon2->Color(PxVec3(0.7f, 0.1f, 0.2f));
			cannon2->Name("CANNON");
			Add(cannon2);

			cannon2proj = new Sphere(PxTransform(PxVec3(36, 4.0f, -40.0f)), 2.0f, 5.0f);
			cannon2proj->Color(PxVec3(1.0f, 0.5f, 0.5f));
			cannon2proj->Name("PROJ");
			Add(cannon2proj);

			cannon3 = new CompoundGun(PxTransform(PxVec3(-40.0f, 4.0f, -30.0f), PxQuat(3 * PxPi / 2, PxVec3(0.0f, 1.0f, 0.0f))), PxVec3(2.0f, 2.0f, 2.0f), 100.0f);
			cannon3->SetKinematic(true);
			cannon3->Color(PxVec3(0.7f, 0.1f, 0.2f));
			cannon3->Name("CANNON");
			Add(cannon3);

			cannon3proj = new Sphere(PxTransform(PxVec3(-36, 4.0f, -30.0f)), 2.0f, 5.0f);
			cannon3proj->Color(PxVec3(1.0f, 0.5f, 0.5f));
			cannon3proj->Name("PROJ");
			Add(cannon3proj);

			// ********** AI PLAYERS **********
			float xPos1 = -15.0f; float yPos1 = 1.0f; float zPos1 = -10.0f;

			for (int i = 0; i < teamSize; i++)
			{
				joustTeam1[i] = new CompoundJoust(PxTransform(PxVec3(xPos1, yPos1, zPos1), PxQuat(0, PxVec3(0.0f, 1.0f, 0.0f))));
				joustTeam1[i]->Color(PxVec3(0.2f, 0.2, 0.6f));
				joustTeam1[i]->Name("JOUST");
				Add(joustTeam1[i]);

				xPos1 += 10.0f;
			}
			// creating blue team

			float xPos2 = -15.0f; float yPos2 = 1.0f; float zPos2 = -30.0f;

			for (int i = 0; i < teamSize; i++)
			{
				joustTeam2[i] = new CompoundJoust(PxTransform(PxVec3(xPos2, yPos2, zPos2), PxQuat(0, PxVec3(0.0f, 1.0f, 0.0f))));
				joustTeam2[i]->Color(PxVec3(0.6f, 0.2, 0.2f));
				joustTeam2[i]->Name("JOUST");
				Add(joustTeam2[i]);

				xPos2 += 10.0f;
			}
			// creating red team

			// ********** WALLS + DOOR **********
			wall = new CompoundWall(PxTransform(PxVec3(0.0f, 10.0f, -95.0f)));
			wall->Color(PxVec3(0.44f, 0.5f, 0.56f)); //grey colour
			wall->SetKinematic(true);
			wall->Name("WALL");
			wall->Material(wallMat);
			wall->SetupFiltering(FilterGroup::ACTOR0, FilterGroup::ACTOR1);
			Add(wall);

			door = new Box(PxTransform(PxVec3(0.0f, 0.0f, -80.0f)), PxVec3(20.0f, 0.2f, 15.0f));
			door->Color(PxVec3(0.8f, 0.1f, 0.0f));
			door->Name("DOOR");
			Add(door);

			doorHinge = new RevoluteJoint(nullptr, PxTransform(PxVec3(0.0f, 0.0f, -95.0f), PxQuat(2 * PxPi, PxVec3(1.0f, 0.0f, 0.0f))), door, PxTransform(PxVec3(0.0f, 0.0f, -15.0f)));
			//doorHinge->DriveVelocity(-0.5f);

			// ********** FLAG **********
			flag = new Cloth(PxTransform(PxVec3(0.0f, 38.0f, -95.0f), PxQuat(PxPi / 2, PxVec3(0.0f, 0.0f, 1.0f))), PxVec2(10.0f, 10.0f), 10, 10);
			flag->Color(color_palette[2]);
			flag->Name("FLAG");
			Add(flag);

			flagPole = new Box(PxTransform(PxVec3(0.0f, 42.0f, -95.0f)), PxVec3(0.5, 5.0f, 0.5f));
			flagPole->Color(PxVec3(0.44f, 0.5f, 0.56f));
			flagPole->SetKinematic(true);
			flagPole->Name("FLAGPOLE");
			Add(flagPole);

			// ********** GOAL EVENT OBJECTS **********
			float xPosGE = -45.0f; float yPosGE = 1.0f; float zPosGE = -95.0f;
			for (int i = 0; i < 5; i++)
			{
				goalEventObjects1[i] = new Box(PxTransform(PxVec3(xPosGE, yPosGE, zPosGE)), PxVec3(1.0f, 1.0f, 1.0f));
				goalEventObjects1[i]->Color(PxVec3(0.8f, 0.1f, 0.1f));
				goalEventObjects1[i]->Name("GOALEVENT");
				Add(goalEventObjects1[i]);

				goalEventObjects2[i] = new Box(PxTransform(PxVec3(xPosGE + 90.0f, yPosGE, zPosGE)), PxVec3(1.0f, 1.0f, 1.0f));
				goalEventObjects2[i]->Color(PxVec3(0.8f, 0.1f, 0.1f));
				goalEventObjects2[i]->Name("GOALEVENT");
				Add(goalEventObjects2[i]);

				yPosGE += 2.0f;
			}

			// ********** DESTROYABLE WALL **********
			/*float xPosDW = -20.0f; float yPosDW = 0.5f; float zPosDW = -60.0f;

			for (int i = 1; i <= destroyWallSize; i++)
			{
				destroyWall[i - 1] = new Box(PxTransform(PxVec3(xPosDW, yPosDW, zPosDW)), PxVec3(1.0f, 1.0f, 1.0f), 10.0f);
				Add(destroyWall[i - 1]);

				if (i % 20 == 0)
				{
					yPosDW += 2.0f;
					xPosDW = -20.0f;
				}
				else
				{
					xPosDW += 2.0f;
				}
			}*/

			// ********** TRAMPOLINE **********
			tramp = new Trampoline(PxVec3(4.0f, 4.0f, 4.0f), 100.0f, 0.5f);
			tramp->AddToScene(this);

			drop = new Box(PxTransform(PxVec3(-35.0f, 15.5f, -75.0f)), PxVec3(2.0f, 2.0f, 2.0f));
			drop->Color(color_palette[4]);
			Add(drop);

			//other workshop code (joints + trampoline)
			{
				//box = new Box(PxTransform(PxVec3(-30.0f, 10.5f, -5.0f)));
				//box->Color(color_palette[0]);
				////set collision filter flags
				//box->SetupFiltering(FilterGroup::ACTOR0, FilterGroup::ACTOR1);
				////use | operator to combine more actors e.g.
				//box->SetupFiltering(FilterGroup::ACTOR0, FilterGroup::ACTOR1 | FilterGroup::ACTOR2 | FilterGroup::ACTOR3);
				//box->Name("Box1");
				//box->SetKinematic(true);
				//Add(box);

				//box2 = new Box(PxTransform(PxVec3(-30.0f, 7.5f, -5.0f)));
				//box2->Color(color_palette[2]);
				////don't forget to set your flags for the matching actor as well, e.g.:
				//box2->SetupFiltering(FilterGroup::ACTOR1, FilterGroup::ACTOR0);
				//box2->SetupFiltering(FilterGroup::ACTOR1, FilterGroup::ACTOR2);
				//box2->Name("Box2");
				//Add(box2);

				//box3 = new Box(PxTransform(PxVec3(-30.0f, 5.5f, -5.0f)));
				//box3->Color(color_palette[0]);
				//box3->SetupFiltering(FilterGroup::ACTOR2, FilterGroup::ACTOR0);
				//box3->Name("Box3");
				//Add(box3);

				//box4 = new Box(PxTransform(PxVec3(-30.0f, 2.5f, -5.0f)));
				//box4->Color(color_palette[3]);
				//box4->SetupFiltering(FilterGroup::ACTOR3, FilterGroup::ACTOR0);
				//box4->Name("Box4");
				//Add(box4);

				////joint two boxes together
				////the joint is fixed to the centre of the first box, oriented by 90 degrees around the Y axis
				////and has the second object attached 5 meters away along the Y axis from the first object.
				//RevoluteJoint joint(box, PxTransform(PxVec3(0.0f, 0.0f, -5.0f),PxQuat(PxPi/2,PxVec3(0.0f, 1.0f, 0.0f))), box2, PxTransform(PxVec3(0.0f, 3.0f, -5.0f)));
				//((PxJoint*)joint.Get())->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
				//joint.DriveVelocity(2.0f);
				////joint.SetLimits(0.0f, 360.0f);

				//RevoluteJoint joint2(box, PxTransform(PxVec3(0.0f, 0.0f, -5.0f), PxQuat(PxPi / 2, PxVec3(1.0f, 0.0f, 0.0f))), box3, PxTransform(PxVec3(0.0f, 5.0f, -5.0f)));
				//((PxJoint*)joint2.Get())->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
				//joint2.DriveVelocity(4.0f);

				//RevoluteJoint joint3(box, PxTransform(PxVec3(0.0f, 0.0f, -5.0f), PxQuat(PxPi / 2, PxVec3(0.0f, 0.0f, 1.0f))), box4, PxTransform(PxVec3(0.0f, 8.0f, -5.0f)));
				//((PxJoint*)joint3.Get())->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);
				//joint3.DriveVelocity(3.0f);

				//((PxActor*)box->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
				//((PxActor*)box2->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
				//((PxActor*)box3->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
				//((PxActor*)box4->Get())->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
				////turning of rgavity for the actors
				//
				//tramp = new Trampoline(PxVec3(1.0f, 1.0f, 1.0f), 20.0f, 0.5f);
				////tramp->AddToScene(this);

				//drop = new Box(PxTransform(PxVec3(0.0f, 5.5f, 0.0f)));
				//drop->Color(color_palette[4]);
				////Add(drop);
			}
		}

		// ****************************

		// ********** UPDATE ********** 

		// ****************************


		//Custom udpate function
		virtual void CustomUpdate() 
		{
			// ********** RESETTING DROP BOX **********
			if (dropTimer.getChronoTime() > 2)
			{
				((PxRigidBody*)drop->Get())->setGlobalPose(PxTransform(PxVec3(-35.0f, 15.5f, -75.0f)));
				((PxRigidBody*)drop->Get())->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
				dropTimer.resetChronoTimer();
			}

			// ********** OPEN AND CLOSE DOOR **********
			if (doorTimer.getChronoTime() > 4)
			{
				doorHinge->DriveVelocity(-1.0f);
			}

			if (doorTimer.getChronoTime() > 7)
			{
				doorHinge->DriveVelocity(1.0f);
				doorTimer.resetChronoTimer();
			}

			// ********** MOVING JOUSTERS **********
			for (int i = 0; i < teamSize; i++)
			{
				((PxRigidBody*)joustTeam1[i]->Get())->addForce(PxVec3(0.0f, 0.0f, -101.0f));
			}
			// adding force to blue team

			for (int i = 0; i < teamSize; i++)
			{
				((PxRigidBody*)joustTeam2[i]->Get())->addForce(PxVec3(0.0f, 0.0f, 100.0f));
			}
			// adding force to red team

			if (joustTimer.getChronoTime() > 2)
			{
				float xPos1 = -15.0f; float yPos1 = 1.0f; float zPos1 = -10.0f;

				for (int i = 0; i < teamSize; i++)
				{
					((PxRigidBody*)joustTeam1[i]->Get())->setGlobalPose(PxTransform(PxVec3(xPos1, yPos1, zPos1), PxQuat(0, PxVec3(0.0f, 1.0f, 0.0f))));
					((PxRigidBody*)joustTeam1[i]->Get())->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));

					xPos1 += 10.0f;
				}
				// resetting blue team

				float xPos2 = -15.0f; float yPos2 = 1.0f; float zPos2 = -30.0f;

				for (int i = 0; i < teamSize; i++)
				{
					((PxRigidBody*)joustTeam2[i]->Get())->setGlobalPose(PxTransform(PxVec3(xPos2, yPos2, zPos2), PxQuat(PxPi, PxVec3(0.0f, 1.0f, 0.0f))));
					((PxRigidBody*)joustTeam2[i]->Get())->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
					
					xPos2 += 10.0f;
				}
				// resetting blue team

				joustTimer.resetChronoTimer();
			}

			// ********** FIRING CANNONS **********
			if (cannonTimer.getChronoTime() == 1)
			{
				cannonForce();
			}
			else if (cannonTimer.getChronoTime() > 5)
			{
				cannonReset();
				cannonTimer.resetChronoTimer();
			}

			// ********** RESETING CATAPULT *********
			if (fgTimer.getChronoTime() > 1)
			{
				PxTransform basePos = ((PxRigidActor*)catapultBase->Get())->getGlobalPose();
				PxVec3 newThrowPos = basePos.p - PxVec3(0.0f, 0.0f, -4.0f);
				((PxRigidBody*)catapultThrow->Get())->setGlobalPose(PxTransform(newThrowPos));
				// resetting the catapult position
				catapultJoint->DriveVelocity(0.0f);
				// stopping the catapult

				catapultThrow->SetKinematic(true);

				//if (fieldGoalBool == true)
				{
					fieldGoalBool = false;
					fgTimer.resetChronoTimer();
				}

				if (ballIsThere == false)
				{
					// spawning in a new ball only if there is not a ball there
					PxTransform throwPos = ((PxRigidActor*)catapultThrow->Get())->getGlobalPose();
					PxVec3 newBallPos = throwPos.p + PxVec3(0.0f, 0.0f, 1.0f);
					rugbyBall = new compoundRugbyBall(PxTransform(PxVec3(0.0f, 6.5f, -1.0f)));
					rugbyBall->Color(PxVec3(0.7f, 0.0f, 0.7f));
					rugbyBall->Material(rugbyBallMat);
					rugbyBall->Name("BALL");
					rugbyBall->SetupFiltering(FilterGroup::ACTOR1, FilterGroup::ACTOR0);
					rugbyBall->SetKinematic(true);
					ballIsThere = true;
					Add(rugbyBall);
				}
			}

			// ********** BALL FOLLOW CATAPULT **********
			if (fieldGoalBool == false)
			{
				// only follow when not in field goal
				PxTransform throwPos = ((PxRigidActor*)catapultThrow->Get())->getGlobalPose();
				PxVec3 newBallPos = throwPos.p + PxVec3(0.0f, 1.0f, 0.0f);

				((PxRigidActor*)rugbyBall->Get())->setGlobalPose(PxTransform(newBallPos));
			}

			// ********** COLLISION EVENTS **********
			// if the player scores a goal
			if (my_callback->goal == true)
			{
				//cout << "GGOOOAAAALLLLLLAAZZZZOOOOOOOOO!!!!!!!!!!!!!" << endl;
				// do stuff when goal is scored

				PxVec3 GEforce1 = PxVec3(0.0f, 10000.0f, 0.0f);

				for (int i = 0; i < 5; i++)
				{

					if (i == 1)
						GEforce1 = PxVec3(-2000.0f, 10000.0f, 0.0f);
					else if (i == 2)
						GEforce1 = PxVec3(2000.0f, 10000.0f, 0.0f);
					else if (i == 3)
						GEforce1 = PxVec3(-5000.0f, 10000.0f, 0.0f);
					else if (i == 3)
						GEforce1 = PxVec3(5000.0f, 10000.0f, 0.0f);
					else
						GEforce1 = PxVec3(0.0f, 10000.0f, 0.0f);
			

					((PxRigidBody*)goalEventObjects1[i]->Get())->addForce(GEforce1);
					((PxRigidBody*)goalEventObjects2[i]->Get())->addForce(GEforce1);
					goalEventTimer.resetChronoTimer();
				}
			}

			// if ball hits wall
			if (my_callback->wallHit == true)
			{
				my_callback->wallHit = false;
				PxVec3 wallBoxPos = my_callback->ballPos.p + PxVec3(0.0f, -2.0f, 0.0f);

				wallBox = new Box(PxTransform(wallBoxPos), PxVec3(0.4f, 0.4f, 0.4f));
				Add(wallBox);// spawn new ball to act as wall being chipped away
			}

			

			// ********** RESETTING GOAL EVENT OBJECTS **********
			if (goalEventTimer.getChronoTime() > 2)
			{
				float xPosGE = -45.0f; float yPosGE = 1.0f; float zPosGE = -95.0f;
				for (int i = 0; i < 5; i++)
				{
					((PxRigidBody*)goalEventObjects1[i]->Get())->setGlobalPose(PxTransform(PxVec3(xPosGE, yPosGE, zPosGE)));
					((PxRigidBody*)goalEventObjects2[i]->Get())->setGlobalPose(PxTransform(PxVec3(xPosGE + 90.0f, yPosGE, zPosGE)));

					yPosGE += 2.0f;
				}
			}
		}

		// *************************************

		// ********** OTHER FUNCTIONS **********

		// *************************************

		void moveCatapultLeft() // on J key
		{
			catapultBase->SetKinematic(false);
			catapultThrow->SetKinematic(false);
			((PxRigidBody*)catapultBase->Get())->addForce(PxVec3(-100000.0f, 0.0f, 0.0f));
		}

		void moveCatapultRight() // on L key
		{
			catapultBase->SetKinematic(false);
			catapultThrow->SetKinematic(false);
			((PxRigidBody*)catapultBase->Get())->addForce(PxVec3(100000.0f, 0.0f, 0.0f));
		}

		void fieldGoal() // on F key
		{
			if (fieldGoalBool == false)
			{
				fgTimer.resetChronoTimer();
				ballTimer.resetChronoTimer();
				// resetting timers

				fieldGoalBool = true;
				catapultBase->SetKinematic(true);
				catapultThrow->SetKinematic(false);
				rugbyBall->SetKinematic(false);
				ballIsThere = false;

				int speed = rand() % (28 - 19) + 19;
				// generate random number between 19 and 28
				catapultJoint->DriveVelocity(-speed);
				//catapultJoint->DriveVelocity(-23.0f);
				cout << speed << endl;
			}
		}

		void cannonForce()
		{
			((PxRigidBody*)cannon1proj->Get())->addForce(PxVec3(4000.0f, 3800.0f, 0.0f));
			((PxRigidBody*)cannon2proj->Get())->addForce(PxVec3(-4000.0f, 3600.0f, 0.0f));
			((PxRigidBody*)cannon3proj->Get())->addForce(PxVec3(4000.0f, 3400.0f, 0.0f));
			// adding force to the cannons projectiles
		}

		void cannonReset()
		{
			((PxRigidBody*)cannon1proj->Get())->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
			((PxRigidBody*)cannon1proj->Get())->setGlobalPose(PxTransform(PxVec3(-36, 4.0f, -50.0f)));

			((PxRigidBody*)cannon2proj->Get())->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
			((PxRigidBody*)cannon2proj->Get())->setGlobalPose(PxTransform(PxVec3(36, 4.0f, -40.0f)));

			((PxRigidBody*)cannon3proj->Get())->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
			((PxRigidBody*)cannon3proj->Get())->setGlobalPose(PxTransform(PxVec3(-36, 4.0f, -30.0f)));
			// resetting the cannon projectiles
		}

		void reset()
		{
			//catapultThrow->SetKinematic(false);
			fieldGoalBool = false;
		}


		/// An example use of key release handling
		void ExampleKeyReleaseHandler()
		{
			cerr << "I am realeased!" << endl;
		}

		/// An example use of key presse handling
		void ExampleKeyPressHandler()
		{
			cerr << "I am pressed!" << endl;
		}

		// ********** PERFORMANCE TEST CASES **********

		void spawnBalls()
		{
			compoundRugbyBall* Testball = new compoundRugbyBall(PxTransform(PxVec3(0.0f, 20.0f, -10.0f)));
			Testball->Color(PxVec3(0.4f, 0.2f, 0.7f));
			Add(Testball);
			
		}

		void spawn1000Balls()
		{
			for (int i = 0; i < 1000; i++)
			{
				compoundRugbyBall* Testball = new compoundRugbyBall(PxTransform(PxVec3(0.0f, 20.0f, -10.0f)));
				Testball->Color(PxVec3(0.4f, 0.2f, 0.7f));
				Add(Testball);
			}
		}

		void spawn100Balls()
		{
			for (int i = 0; i < 100; i++)
			{
				compoundRugbyBall* Testball = new compoundRugbyBall(PxTransform(PxVec3(0.0f, 20.0f, -10.0f)));
				Testball->Color(PxVec3(0.4f, 0.2f, 0.7f));
				Add(Testball);
			}
		}

		void spawnJoust()
		{
			float xPos1 = -250.0f; float yPos1 = 1.0f; float zPos1 = 5.0f;

			for (int i = 0; i < 100; i++)
			{
				CompoundJoust* joust = new CompoundJoust(PxTransform(PxVec3(xPos1, yPos1, zPos1), PxQuat(0, PxVec3(0.0f, 1.0f, 0.0f))));
				joust->Color(PxVec3(0.2f, 0.2, 0.6f));
				joust->Name("JOUST");
				Add(joust);

				xPos1 += 5.0f;
			}
			// creating jouters
		}
	};
}
