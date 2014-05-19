#include "World.h"

#include <btBulletDynamicsCommon.h>

#include <Windows.h>

//#include "GLDebugDrawer.h"

#ifdef _DEBUG
    #pragma comment(lib, "linearmath_vs2010_debug")
    #pragma comment(lib, "bulletcollision_vs2010_debug")
    #pragma comment(lib, "bulletdynamics_vs2010_debug")
#else
    #pragma comment(lib, "linearmath_vs2010_release")
    #pragma comment(lib, "bulletcollision_vs2010_release")
    #pragma comment(lib, "bulletdynamics_vs2010_release")
#endif


bool ticksPerSecondInitialized = false;
static LARGE_INTEGER ticksPerSecond;

static unsigned int getTicks()
{
    if (!ticksPerSecondInitialized) {
        QueryPerformanceFrequency(&ticksPerSecond);
        ticksPerSecondInitialized = true;
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    now.QuadPart *= 1000;
    now.QuadPart /= ticksPerSecond.QuadPart;

    return (unsigned int) now.QuadPart;
}


class BasicObject : public World::Object
{
    protected:
        btCollisionShape* shape;
        btRigidBody* rigidBody;
        btDefaultMotionState* motionState;

    public:
        virtual ~BasicObject()
        {

        }

        void addToWorld(btDynamicsWorld* world)
        {
            world->addRigidBody(rigidBody);
        }

        void removeFromWorld(btDynamicsWorld* world)
        {
            world->removeRigidBody(rigidBody);
        }
};

class Floor : public BasicObject
{
    public:

        Floor()
        {
            shape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);

            motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -0.5f, 0)));

            btRigidBody::btRigidBodyConstructionInfo rbci(0, motionState, shape, btVector3(0, 0, 0));
            rigidBody = new btRigidBody(rbci);
        }

        ~Floor()
        {

        }


        void render() override
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


class Ball : public BasicObject
{
    public:

        Ball()
        {
            shape = new btSphereShape(0.1f);

            motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 1)));

            btScalar mass = 1;
            btVector3 inertia(0, 0, 0);
            shape->calculateLocalInertia(mass, inertia);

            btRigidBody::btRigidBodyConstructionInfo rbci(mass, motionState, shape, inertia);
            rigidBody = new btRigidBody(rbci);
            rigidBody->setRestitution(0.9f);
        }

        ~Ball()
        {
            
        }

        void render() override
        {
            float transform[16];
            btTransform tf = rigidBody->getWorldTransform();
            tf.getOpenGLMatrix(transform);

            glColor3f(1.0f, 0.0f, 0.0f);
            GLUquadric* q = gluNewQuadric();
            glPushMatrix();
                glMultMatrixf(transform);
                gluSphere(q, ((btSphereShape*)shape)->getRadius(), 32, 32);
            glPopMatrix();
            gluDeleteQuadric(q);
        }
};


const NUI_SKELETON_POSITION_INDEX BONES[][2] =
{
    // Head/Torso
    { NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER },
    { NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE },
    { NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER },

    // Left arm
    { NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT },
    { NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT },
    { NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT },
    { NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT },

    // Left leg
    { NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT },
    { NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT },
    { NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT },
    { NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT },

    // Right arm
    { NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT },
    { NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT },
    { NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT },
    { NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT },

    // Right leg
    { NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT },
    { NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT },
    { NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT },
    { NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT },
};

const int NUM_BONES = sizeof(BONES) / 2 / sizeof(NUI_SKELETON_POSITION_INDEX);


class Skeleton : public World::Object
{
    private:
        class BoneMotionState : public btMotionState
        {
            private:
                btTransform transform;

            public:
                BoneMotionState(const btTransform& initialTransform) : transform(initialTransform) { }

                virtual void getWorldTransform(btTransform& wt) const override
                {
                    wt = transform;
                }

                virtual void setWorldTransform(const btTransform& wt) override
                {
                    transform = wt;
                }
        };

        struct Bone
        {
            btCapsuleShape* shape;
            btRigidBody* rigidBody;
            BoneMotionState* motionState;
        };

        Bone bones[NUM_BONES];

    public:
        Skeleton()
        {
            for (int i = 0; i < NUM_BONES; ++i) {
                Bone& b = bones[i];

                b.shape = new btCapsuleShape(0.02f, 0.2f);

                b.motionState = new BoneMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

                btRigidBody::btRigidBodyConstructionInfo rbci(0, b.motionState, b.shape, btVector3(0, 0, 0));
                b.rigidBody = new btRigidBody(rbci);
                b.rigidBody->setCollisionFlags(b.rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
                b.rigidBody->setActivationState(DISABLE_DEACTIVATION);
            }
        }

        ~Skeleton()
        {
            for (int i = 0; i < NUM_BONES; ++i) {
            }
        }

        static int findSkeleton(const NUI_SKELETON_FRAME& frame)
        {
            for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
                if (frame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED) return i;
            }
            return -1;
        }

        void update(const NUI_SKELETON_FRAME& frame)
        {
            int idx = findSkeleton(frame);
            if (idx == -1) {
                for (int i = 0; i < NUM_BONES; ++i) bones[i].rigidBody->setActivationState(DISABLE_SIMULATION);
                return;
            }

            const NUI_SKELETON_DATA& data = frame.SkeletonData[idx];

            for (int i = 0; i < NUM_BONES; ++i) {
                NUI_SKELETON_POSITION_TRACKING_STATE
                    s1 = data.eSkeletonPositionTrackingState[BONES[i][0]],
                    s2 = data.eSkeletonPositionTrackingState[BONES[i][1]];

                if (s1 == NUI_SKELETON_POSITION_NOT_TRACKED || s2 == NUI_SKELETON_POSITION_NOT_TRACKED) continue;

                const Vector4& u0 = data.SkeletonPositions[BONES[i][0]];
                const Vector4& v0 = data.SkeletonPositions[BONES[i][1]];

                btVector3 up(0, 1, 0);
                btVector3 u(-u0.x, u0.y, u0.z);
                btVector3 v(-v0.x, v0.y, v0.z);
                btVector3 uv = v - u;

                btVector3 right = uv.cross(up);
                up = right.cross(uv);
                btMatrix3x3 rotation;
                rotation[0] = right.normalized();
                rotation[1] = uv.normalized();
                rotation[2] = up.normalized();

                btScalar length = uv.length();
                //btScalar angle = uv.angle(up);
                //btVector3 axis = uv.cross(up);
                /*btQuaternion(axis, angle)*/

                bones[i].shape->setLocalScaling(btVector3(1, length, 1));
                btTransform transform(rotation.transpose(), u);

                bones[i].motionState->setWorldTransform(transform);

                //if (bones[i].rigidBody->getActivationState() == DISABLE_SIMULATION) {
                    bones[i].rigidBody->setActivationState(DISABLE_DEACTIVATION);
                //}
            }
        }

        void render() override
        {
            GLUquadric* q = gluNewQuadric();
            float transform[16];
            for (int i = 0; i < NUM_BONES; ++i) {
                if (bones[i].rigidBody->getActivationState() == DISABLE_SIMULATION) continue;
                btScalar length = bones[i].shape->getLocalScaling().getY();
                btTransform tf = bones[i].rigidBody->getWorldTransform();
                tf.getOpenGLMatrix(transform);
                glPushMatrix();
                    glMultMatrixf(transform);
                    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                    gluCylinder(q, 0.02, 0.02, length, 12, 1);
                glPopMatrix();
            }
            gluDeleteQuadric(q);
        }

        void addToWorld(btDynamicsWorld* world)
        {
            for (int i = 0; i < NUM_BONES; ++i) {
                world->addRigidBody(bones[i].rigidBody);
            }
        }

        void removeFromWorld(btDynamicsWorld* world)
        {
            for (int i = 0; i < NUM_BONES; ++i) {
                world->removeRigidBody(bones[i].rigidBody);
            }
        }
};

World::World() :
    running(false),
    skeletonCurrentA(false)
{
    skeletonA.dwFrameNumber = 0;
    skeletonB.dwFrameNumber = 0;
    newSkeleton.dwFrameNumber = 0;

    broadphase = new btDbvtBroadphase();
    
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    
    //btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
    
    solver = new btSequentialImpulseConstraintSolver;
    
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    
    dynamicsWorld->setGravity(btVector3(0, -10, 0));
    //dynamicsWorld->setDebugDrawer(new GLDebugDrawer());
    
    floor = new Floor();
    floor->addToWorld(dynamicsWorld);


    ball = new Ball();
    ball->addToWorld(dynamicsWorld);

    skeleton = new Skeleton();
    skeleton->addToWorld(dynamicsWorld);

    running = true;
    this->start();
}

World::~World()
{
    if (this->isRunning()) {
        running = false;
        this->wait();
    }

    delete skeleton;
    delete floor;
    delete ball;

    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
}


void World::updateSkeleton()
{
    skeletonCurrentA = !skeletonCurrentA;

    NUI_SKELETON_FRAME* current = skeletonCurrentA ? &skeletonA : &skeletonB;
    //NUI_SKELETON_FRAME* last = skeletonCurrentA ? &skeletonB : &skeletonA;

    skeletonMutex.lock();
    memcpy(current, &newSkeleton, sizeof(NUI_SKELETON_FRAME));
    skeletonMutex.unlock();

    skeleton->update(*current);
    /*if (current->dwFrameNumber == 0) {
        if (last->dwFrameNumber != 0) {}


    }*/
}


void World::run()
{
    unsigned int start, ticks;
    while (running)
    {
        start = getTicks();
        mutex.lock();

        runIteration();

        waitCondition.wakeAll();
        mutex.unlock();

        start += (1000 / SIMULATION_FPS);
        while ((ticks = getTicks()) < start) Sleep(start - ticks);
    }
}

void World::runIteration()
{
    updateSkeleton();

    dynamicsWorld->stepSimulation((1000 / SIMULATION_FPS) / 1000.0f, 10);

    //btTransform trans;
    //fallRigidBody->getMotionState()->getWorldTransform(trans);
    
    //qDebug() << "sphere height: " << trans.getOrigin().getY() << std::endl;
}

void World::render()
{
    mutex.lock();
        //dynamicsWorld->debugDrawWorld();
        floor->render();
        ball->render();
        skeleton->render();
    mutex.unlock();
}


void World::setSkeleton(const NUI_SKELETON_FRAME* skeleton)
{
    skeletonMutex.lock();
    memcpy(&newSkeleton, skeleton, sizeof(NUI_SKELETON_FRAME));
    skeletonMutex.unlock();
}
