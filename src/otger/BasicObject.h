#ifndef BASICOBJECT_H
#define BASICOBJECT_H

#pragma once

#include "Object.h"
#include "World.h"

class BasicObject : public Object
{
    protected:
        btCollisionShape* shape;
        btRigidBody* rigidBody;
        btMotionState* motionState;

    public:
        virtual ~BasicObject()
        {

        }

        void addToWorld(btDynamicsWorld* world) override
        {
            world->addRigidBody(rigidBody);
        }

        void removeFromWorld(btDynamicsWorld* world) override
        {
            world->removeRigidBody(rigidBody);
        }
};

#endif
