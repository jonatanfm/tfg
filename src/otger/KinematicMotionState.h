#ifndef KINEMATICMOTIONSTATE_H
#define KINEMATICMOTIONSTATE_H

#pragma once

#include "Bullet.h"

class KinematicMotionState : public btMotionState
{
    private:
        btTransform transform;

    public:
        KinematicMotionState(const btTransform& initialTransform) : transform(initialTransform) { }

        virtual void getWorldTransform(btTransform& wt) const override
        {
            wt = transform;
        }

        virtual void setWorldTransform(const btTransform& wt) override
        {
            transform = wt;
        }
    };

#endif
