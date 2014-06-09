#ifndef OBJECT_H
#define OBJECT_H

#pragma once

#include "../RenderUtils.h"

class btDynamicsWorld;

// Base class for all world physical (or not) objects
class Object
{
    public:
        virtual ~Object() { }

        // Render for projecting into 2D
        virtual void render(TextureManager& textures) = 0;

        // Render for debugging into a 3D scene
        virtual void render3D(TextureManager& textures)
        {
            render(textures);
        }

        virtual void addToWorld(btDynamicsWorld* world) = 0;

        virtual void removeFromWorld(btDynamicsWorld* world) = 0;

};

#endif
