#ifndef SKELETON_H
#define SKELETON_H

#pragma once

#include "World.h"

#include "KinematicMotionState.h"

class Skeleton : public Object
{
    public:
        static const int NUM_BONES = 19;

    private:
        struct Head
        {
            btSphereShape* shape;
            btRigidBody* rigidBody;
            KinematicMotionState* motionState;
        } head;

        struct Bone
        {
            btCapsuleShape* shape;
            btRigidBody* rigidBody;
            KinematicMotionState* motionState;
        };

        Bone bones[NUM_BONES];

    public:
        Skeleton();

        ~Skeleton();
        
        void update(const NUI_SKELETON_FRAME& frame, int idx);

        void render(RenderManager& textures) override;

        void addToWorld(btDynamicsWorld* world) override
        {
            for (int i = 0; i < NUM_BONES; ++i) {
                world->addRigidBody(bones[i].rigidBody);
            }
            world->addRigidBody(head.rigidBody);
        }

        void removeFromWorld(btDynamicsWorld* world) override
        {
            for (int i = 0; i < NUM_BONES; ++i) {
                world->removeRigidBody(bones[i].rigidBody);
            }
            world->removeRigidBody(head.rigidBody);
        }
};

#endif
