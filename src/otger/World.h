#ifndef WORLD_H
#define WORLD_H

#pragma once

#include "../globals.h"

#include "Bullet.h"

#include "../Data.h"

#include "../RenderUtils.h"

#include "KinematicMotionState.h"

#include "Object.h"

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionShape;

class Floor;
class Skeleton;


// Represents a physical world containing virtual objects
// as well as users and other real scene information.
class World : private QThread
{
    private:

        // FPS of the simulation
        static const int SIMULATION_FPS = 60;

        // Set to true while running.
        volatile bool running;

        // Mutex to protect cross-thread accesses to the simulation state
        QMutex mutex;

        // Enables notification of simulation advances
        QWaitCondition waitCondition;

        // Mutex to protect cross-thread accesses to the skeleton frame
        QMutex skeletonMutex;

        // Double-buffered skeleton frames 
        SkeletonFrame skeletonA, skeletonB;
        bool skeletonCurrentA;

        // Incoming skeleton frame
        SkeletonFrame newSkeleton;


        // Physics simulation objects

        btBroadphaseInterface* broadphase;
    
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;
        
        btSequentialImpulseConstraintSolver* solver;
        
        btDiscreteDynamicsWorld* dynamicsWorld;



        // List of objects
        std::vector<Object*> objects;

        // The floor object
        Floor* floor;

        // The physics user skeleton object
        Skeleton* skeleton;
    
        // Runs the simulation
        void run() override;

        // Updates the physics user skeleton
        void updateSkeleton();

    public:
    
        World();
        ~World();
        
        // Renders the world projected to 2D
        void render(TextureManager& textures);

        // Renders the world for debugging in a 3D scene
        void render3D(TextureManager& textures);

        // Runs a single iteration of the simulation
        void runIteration();

        // Updates the skeleton frame
        void setSkeleton(const SkeletonFrame* skeleton);

        // Get a list of all world objects
        /*std::vector<Object*>& getObjects()
        {
            return objects;
        }*/

        void addObject(Object* object);

        void clearObjects();

};

#endif
