#ifndef BALL_H
#define BALL_H

#pragma once

#include "BasicObject.h"

class Ball : public BasicObject
{
    public:

        Ball(btScalar x, btScalar y, btScalar z, btScalar radius = 0.5f, btScalar mass = 1.0f)
        {
            shape = new btSphereShape(radius);

            motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(x, y, z)));

            btVector3 inertia(0, 0, 0);
            shape->calculateLocalInertia(mass, inertia);

            btRigidBody::btRigidBodyConstructionInfo rbci(mass, motionState, shape, inertia);
            rigidBody = new btRigidBody(rbci);
            rigidBody->setRestitution(0.9f);
        }

        ~Ball()
        {
            
        }

        void render(TextureManager& textures) override
        {
            float transform[16];
            btTransform tf = rigidBody->getWorldTransform();
            tf.getOpenGLMatrix(transform);

            GLUquadric* q = gluNewQuadric();
            gluQuadricTexture(q, GLU_TRUE);

            glColor3f(1.0f, 1.0f, 1.0f);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textures.getTexture("earth.jpg"));
            glPushMatrix();
                glMultMatrixf(transform);
                glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
                gluSphere(q, ((btSphereShape*)shape)->getRadius(), 32, 32);
            glPopMatrix();
            glDisable(GL_TEXTURE_2D);

            gluDeleteQuadric(q);
        }
};

#endif
