#ifndef CUBE_H
#define CUBE_H

#pragma once

#include "BasicObject.h"

extern const char SHADER_NORMALMAP_VERTEX[];
extern const char SHADER_NORMALMAP_FRAGMENT[];

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
            //rigidBody->setRestitution(0.9f);
        }

        ~Cube()
        {
            
        }

        void render(RenderManager& manager) override
        {
            float transform[16];
            btTransform tf = rigidBody->getWorldTransform();
            tf.getOpenGLMatrix(transform);
            
            glColor3f(1.0f, 1.0f, 1.0f);
            glEnable(GL_TEXTURE_2D);

            manager.gl.glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, manager.getTexture("crate2.jpg"));

            manager.gl.glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, manager.getTexture("crate2Normal.jpg"));

            manager.gl.glActiveTexture(GL_TEXTURE0);

            
            //QGLShaderProgram* shader = manager.getShader(SHADER_NORMALMAP_VERTEX, SHADER_NORMALMAP_FRAGMENT);
            //shader->bind();

            glPushMatrix();
                glMultMatrixf(transform);
                btVector3 he = ((btBoxShape*)shape)->getHalfExtentsWithoutMargin();
                RenderUtils::drawCube(he.x(), he.y(), he.z());
            glPopMatrix();

            //shader->release();

            glDisable(GL_TEXTURE_2D);
        }
};

#endif
