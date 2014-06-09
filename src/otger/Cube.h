#ifndef CUBE_H
#define CUBE_H

#pragma once

#include "BasicObject.h"

class Cube : public BasicObject
{
    public:

        Cube(btScalar x, btScalar y, btScalar z, btScalar size = 0.5f, btScalar mass = 1.0f)
        {
            shape = new btBoxShape(btVector3(size, size, size));

            motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(x, y, z)));

            btVector3 inertia(0, 0, 0);
            shape->calculateLocalInertia(mass, inertia);

            btRigidBody::btRigidBodyConstructionInfo rbci(mass, motionState, shape, inertia);
            rigidBody = new btRigidBody(rbci);
            rigidBody->setRestitution(0.9f);
        }

        ~Cube()
        {
            
        }

        void render(TextureManager& textures) override
        {
            float transform[16];
            btTransform tf = rigidBody->getWorldTransform();
            tf.getOpenGLMatrix(transform);
            
            glColor3f(0.5f, 0.9f, 0.5f);
            glPushMatrix();
                glMultMatrixf(transform);
                btVector3 he = ((btBoxShape*)shape)->getHalfExtentsWithoutMargin();
                RenderUtils::drawCube(he.x(), he.y(), he.z());
            glPopMatrix();
        }
};

#endif
