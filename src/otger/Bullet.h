#ifndef BULLET_H
#define BULLET_H

#pragma once

#ifdef REPORT_MEMORY_LEAKS
    #undef new
#endif

#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>

#ifdef REPORT_MEMORY_LEAKS
    //#define new DEBUG_NEW
#endif

#endif
