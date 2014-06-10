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

        static int findSkeleton(const NUI_SKELETON_FRAME& frame);
        
        void update(const NUI_SKELETON_FRAME& frame);

        void render(TextureManager& textures) override;

        void addToWorld(btDynamicsWorld* world) override
        {
            for (int i = 0; i < NUM_BONES; ++i) {
                world->addRigidBody(bones[i].rigidBody);
            }
        }

        void removeFromWorld(btDynamicsWorld* world) override
        {
            for (int i = 0; i < NUM_BONES; ++i) {
                world->removeRigidBody(bones[i].rigidBody);
            }
        }
};

#endif
