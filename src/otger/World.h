#ifndef WORLD_H
#define WORLD_H

#pragma once

#include "../globals.h"

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
        NUI_SKELETON_FRAME skeletonA, skeletonB;

        NUI_SKELETON_FRAME newSkeleton;

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

        void setSkeleton(const NUI_SKELETON_FRAME* skeleton);

};

#endif
