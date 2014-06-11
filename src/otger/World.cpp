#include "World.h"

#include <Windows.h>

#include "BasicObject.h"
#include "Skeleton.h"
#include "Ball.h"
#include "Floor.h"

//#include "GLDebugDrawer.h"

bool ticksPerSecondInitialized = false;
static LARGE_INTEGER ticksPerSecond;

static unsigned int getTicks()
{
    if (!ticksPerSecondInitialized) {
        QueryPerformanceFrequency(&ticksPerSecond);
        ticksPerSecondInitialized = true;
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    now.QuadPart *= 1000;
    now.QuadPart /= ticksPerSecond.QuadPart;

    return (unsigned int) now.QuadPart;
}



World::World() :
    running(false),
    skeletonCurrentA(false)
{
    broadphase = new btDbvtBroadphase();
    
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    
    //btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
    
    solver = new btSequentialImpulseConstraintSolver;
    
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    //dynamicsWorld = new btSoftRigidDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    
    dynamicsWorld->setGravity(btVector3(0, -0.3f /*-10*/, 0));
    //dynamicsWorld->setDebugDrawer(new GLDebugDrawer());
    
    floor = new Floor();
    floor->addToWorld(dynamicsWorld);

    skeleton = new Skeleton();
    skeleton->addToWorld(dynamicsWorld);


    addObject(new Ball(0, 0, 1.5f, 0.4f, 1.0f));


    running = true;
    this->start();
}

World::~World()
{
    if (this->isRunning()) {
        running = false;
        this->wait();
    }

    clearObjects();

    skeleton->removeFromWorld(dynamicsWorld);
    delete skeleton;

    floor->removeFromWorld(dynamicsWorld);
    delete floor;

    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
}


void World::updateSkeleton()
{
    skeletonCurrentA = !skeletonCurrentA;

    SkeletonFrame* current = skeletonCurrentA ? &skeletonA : &skeletonB;
    //SkeletonFrame* last = skeletonCurrentA ? &skeletonB : &skeletonA;

    skeletonMutex.lock();
    *current = newSkeleton;
    skeletonMutex.unlock();

    if (current->isValid()) {
        Vector4 floorPlane = current->frame.vFloorClipPlane;
        floor->updatePlane(floorPlane.x, floorPlane.y, floorPlane.z, floorPlane.w);

        //static int f = 0;
        //if (++f >= 60) { f = 0; qDebug("%.3f %.3f %.3f %.3f\n", floorPlane.x, floorPlane.y, floorPlane.z, floorPlane.w); }

        skeleton->update(current->frame);
    }
}


void World::run()
{
    unsigned int start, ticks;
    while (running)
    {
        start = getTicks();
        mutex.lock();

        runIteration();

        waitCondition.wakeAll();
        mutex.unlock();

        start += (1000 / SIMULATION_FPS);
        while ((ticks = getTicks()) < start) Sleep(start - ticks);
    }
}

void World::runIteration()
{
    updateSkeleton();

    dynamicsWorld->stepSimulation((1000 / SIMULATION_FPS) / 1000.0f, 10);

    //btTransform trans;
    //fallRigidBody->getMotionState()->getWorldTransform(trans);
    
    //qDebug() << "sphere height: " << trans.getOrigin().getY() << std::endl;
}

void World::render(RenderManager& textures)
{
    mutex.lock();
        for (int i = 0; i < int(objects.size()); ++i) {
            objects[i]->render(textures);
        }
    mutex.unlock();
}

void World::render3D(RenderManager& textures)
{
    mutex.lock();
        //dynamicsWorld->debugDrawWorld();
        floor->render(textures);

        for (int i = 0; i < int(objects.size()); ++i) {
            objects[i]->render3D(textures);
        }

        glColor3f(1.0f, 0.0f, 0.0f);
        skeleton->render3D(textures);

    mutex.unlock();
}

void World::setSkeleton(const SkeletonFrame* skeleton)
{
    skeletonMutex.lock();
    newSkeleton = *skeleton;
    skeletonMutex.unlock();
}

void World::addObject(Object* object)
{
    mutex.lock();
    object->addToWorld(dynamicsWorld);
    objects.push_back(object);
    mutex.unlock();
}

void World::clearObjects()
{
    mutex.lock();
    for (int i = 0; i < int(objects.size()); ++i) {
        objects[i]->removeFromWorld(dynamicsWorld);
        delete objects[i];
    }
    objects.clear();
    mutex.unlock();
}
