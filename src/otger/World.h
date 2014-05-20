#ifndef WORLD_H
#define WORLD_H

#pragma once

#include "../globals.h"

#include "../Data.h"

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionShape;

class Floor;
class Ball;
class Skeleton;

class World : private QThread
{
    public:

        class Object
        {
            public:
                virtual ~Object() { }

                virtual void render() = 0;
        };


    private:

        static const int SIMULATION_FPS = 60;

        volatile bool running;

        QMutex mutex;
        QWaitCondition waitCondition;

        QMutex skeletonMutex;

        bool skeletonCurrentA;
        SkeletonFrame skeletonA, skeletonB;

        SkeletonFrame newSkeleton;

        btBroadphaseInterface* broadphase;
    
        btDefaultCollisionConfiguration* collisionConfiguration;
        btCollisionDispatcher* dispatcher;
        
        btSequentialImpulseConstraintSolver* solver;
        
        btDiscreteDynamicsWorld* dynamicsWorld;


        Floor* floor;
        Ball* ball;

        Skeleton* skeleton;
    
        void run() override;

        void updateSkeleton();

    public:
    
        World();
        ~World();
        
        void render();

        void runIteration();

        void setSkeleton(const SkeletonFrame* skeleton);

};

#endif
