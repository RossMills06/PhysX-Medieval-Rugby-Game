#include <iostream> //cout, cerr
#include <iomanip> //stream formatting
#include <windows.h> //delay function
#include <PxPhysicsAPI.h> //PhysX

using namespace std;
using namespace physx;

//PhysX objects
PxPhysics* physics;
PxFoundation* foundation;
debugger::comm::PvdConnection* vd_connection;

//simulation objects
PxScene* scene;
PxRigidDynamic* boxes[10];

PxRigidStatic* plane;

PxRigidDynamic* CreateBox(PxVec3 position, PxVec3 dimensions, PxReal density, PxMaterial* material)
{
	PxRigidDynamic* box;

	// create and set position of box
	box = physics->createRigidDynamic(PxTransform(position));

	// set size of the box (values in half)
	box->createShape(PxBoxGeometry(dimensions), *material);

	// set density of box (density in kg/m^3)
	PxRigidBodyExt::updateMassAndInertia(*box, density); //density of 1kg/m^3
	scene->addActor(*box);

	return box;
}

///Initialise PhysX objects
bool PxInit()
{
	//default error and allocator callbacks
	static PxDefaultErrorCallback gDefaultErrorCallback;
	static PxDefaultAllocator gDefaultAllocatorCallback;

	//Init PhysX
	//foundation
	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);

	if(!foundation)
	{
		return false;
	}
		
	//physics
	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale());

	if(!physics)
	{
		return false;
	}
		

	//connect to an external visual debugger (if exists)
	vd_connection = PxVisualDebuggerExt::createConnection(physics->getPvdConnectionManager(), "localhost", 5425, 100, PxVisualDebuggerExt::getAllConnectionFlags());

	//create a default scene
	PxSceneDesc sceneDesc(physics->getTolerancesScale());

	if(!sceneDesc.cpuDispatcher)
	{
		PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
		sceneDesc.cpuDispatcher = mCpuDispatcher;
	}

	if (!sceneDesc.filterShader)
	{
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	}

	scene = physics->createScene(sceneDesc);

	if (!scene)
	{
		return false;
	}
	
	return true;
}

/// Release all allocated resources
void PxRelease()
{
	if (scene)
	{
		scene->release();
	}
		
	if (vd_connection)
	{
		vd_connection->release();
	}
		
	if (physics)
	{
		physics->release();
	}
		
	if (foundation)
	{
		foundation->release();
	}	
}

///Initialise the scene
void InitScene()
{
	//default gravity
	scene->setGravity(PxVec3(0.0f, -9.81f, 0.0f));

	//materials
	PxMaterial* default_material = physics->createMaterial(0.1f, 0.1f, 0.5f);   //static friction, dynamic friction, restitution

	//create a static plane (XZ)
	plane = PxCreatePlane(*physics, PxPlane(PxVec3(0.0f, 1.0f, 0.0f), 0.0f), *default_material);
	scene->addActor(*plane);

	// create 10 boxes
	for (int i = 0; i < 10; i++)
	{
		boxes[i] = CreateBox(PxVec3(0.0f, 0.5f, 0.0f), PxVec3(0.5, 0.5f, 0.5f), 1.0f, default_material);
	}
}

/// Perform a single simulation step
void Update(PxReal delta_time)
{
	scene->simulate(delta_time);
	scene->fetchResults(true);
}

/// The main function
int main()
{
	//initialise PhysX	
	if (!PxInit())
	{
		cerr << "Could not initialise PhysX." << endl;
		return 0;
	}

	//initialise the scene
	InitScene();

	//set the simulation step to 1/60th of a second
	PxReal delta_time = 1.0f/60.0f;
	int index = 0;

	// add force to the box
	boxes[0]->addForce(PxVec3(2000.0f, 250.0f, 0.0f));
	//box2->addForce(PxVec3(-2000.0f, 0.0f, 0.0f));

	//simulate until the 'Esc' is pressed
	while (!GetAsyncKeyState(VK_ESCAPE))
	{
		//'visualise' position and velocity of the box
		PxVec3 positionBox = boxes[0]->getGlobalPose().p;
		PxVec3 velocityBox = boxes[0]->getLinearVelocity();

		// output values
		cout << setiosflags(ios::fixed) << setprecision(2) << index << " " << "x=" << positionBox.x <<  ", y=" << positionBox.y << ", z=" << positionBox.z << ",  ";
		cout << setiosflags(ios::fixed) << setprecision(2) << "vx=" << velocityBox.x << ", vy=" << velocityBox.y << ", vz=" << velocityBox.z << ", mass=" << boxes[0]->getMass() <<endl;

		index++;

		//if (index % 500 == 0)
		//{
		//	box->addForce(PxVec3(2000.0f, 0.0f, 0.0f));
		//	box2->addForce(PxVec3(-2000.0f, 0.0f, 0.0f));
		//}
		//// add force every 500 frames

		//perform a single simulation step
		Update(delta_time);

		//introduce 100ms delay for easier visual analysis of the results
		Sleep(10);
	}

	//Release all resources
	PxRelease();

	return 0;
}
