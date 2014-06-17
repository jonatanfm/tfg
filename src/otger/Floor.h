#ifndef FLOOR_H
#define FLOOR_H

#pragma once

#include "BasicObject.h"

class Floor : public BasicObject
{
    private:
        btScalar planeD;

    public:

        Floor() : planeD(1.5f)
        {
            shape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);

            motionState = new KinematicMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -planeD, 0)));

            btRigidBody::btRigidBodyConstructionInfo rbci(0, motionState, shape, btVector3(0, 0, 0));
            rigidBody = new btRigidBody(rbci);
            rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            rigidBody->setActivationState(DISABLE_DEACTIVATION);
        }

        ~Floor()
        {

        }

        void updatePlane(float a, float b, float c, float d)
        {
            d = 0.75f; // TODO: Remove

            if (abs(d - planeD) < 0.015f) return;

            planeD = d;
            btTransform transform(btQuaternion(0, 0, 0, 1), btVector3(0, -d, 0));

            //qDebug("Set d to %.4f", d);
            
            motionState->setWorldTransform(transform);
        }

        void render(RenderManager& textures) override
        {
            float transform[16];
            btTransform tf = rigidBody->getWorldTransform();
            tf.getOpenGLMatrix(transform);

            btVector3 n = ((btStaticPlaneShape*)shape)->getPlaneNormal();
            btScalar d = ((btStaticPlaneShape*)shape)->getPlaneConstant();

            float y = d / n[1]; // Assuming vertical plane

            glColor3f(230 / 255.0f, 230 / 255.0f, 250 / 255.0f);
            glPushMatrix();
                glMultMatrixf(transform);
                glBegin(GL_TRIANGLE_STRIP);
                    glVertex3f(-1000.0f, y, -1000.0f);
                    glVertex3f(-1000.0f, y,  1000.0f);
                    glVertex3f(1000.0f, y, -1000.0f);
                    glVertex3f( 1000.0f, y,  1000.0f);
                glEnd();
            glPopMatrix();
        }
};

#endif
