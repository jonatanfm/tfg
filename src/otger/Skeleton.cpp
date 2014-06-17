
#include "Skeleton.h"

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

//const int NUM_BONES = sizeof(BONES) / 2 / sizeof(NUI_SKELETON_POSITION_INDEX);

Skeleton::Skeleton()
{
    // Head
    {
        head.shape = new btSphereShape(0.08f);

        head.motionState = new KinematicMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

        btRigidBody::btRigidBodyConstructionInfo rbci(0, head.motionState, head.shape, btVector3(0, 0, 0));
        head.rigidBody = new btRigidBody(rbci);
        head.rigidBody->setCollisionFlags(head.rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        head.rigidBody->setActivationState(DISABLE_DEACTIVATION);
    }

    // Bones
    for (int i = 0; i < NUM_BONES; ++i) {
        Bone& b = bones[i];

        b.shape = new btCapsuleShape(0.02f, 0.2f);

        b.motionState = new KinematicMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

        btRigidBody::btRigidBodyConstructionInfo rbci(0, b.motionState, b.shape, btVector3(0, 0, 0));
        b.rigidBody = new btRigidBody(rbci);
        b.rigidBody->setCollisionFlags(b.rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        b.rigidBody->setActivationState(DISABLE_DEACTIVATION);
    }
}

Skeleton::~Skeleton()
{
    delete head.rigidBody;
    delete head.motionState;
    delete head.shape;

    for (int i = 0; i < NUM_BONES; ++i) {
        delete bones[i].rigidBody;
        delete bones[i].motionState;
        delete bones[i].shape;
    }
}

void Skeleton::update(const NUI_SKELETON_FRAME& frame, int idx)
{
    //qDebug("%.2f %.2f %.2f %.2f\n", frame.vFloorClipPlane.x, frame.vFloorClipPlane.y, frame.vFloorClipPlane.z, frame.vFloorClipPlane.w);

    if (idx == -1) {
        for (int i = 0; i < NUM_BONES; ++i) bones[i].rigidBody->setActivationState(DISABLE_SIMULATION);
        head.rigidBody->setActivationState(DISABLE_SIMULATION);
        return;
    }

    const NUI_SKELETON_DATA& data = frame.SkeletonData[idx];

    if (data.eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD] != NUI_SKELETON_POSITION_NOT_TRACKED) {
        const Vector4& u0 = data.SkeletonPositions[NUI_SKELETON_POSITION_HEAD];
        const Vector4& v0 = data.SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER];
        btVector3 u(-u0.x, u0.y, u0.z), v(-v0.x, v0.y, v0.z);
        head.motionState->setWorldTransform(btTransform(btQuaternion(0, 0, 0, 1), (u + u + v) / 3));
        head.rigidBody->setActivationState(DISABLE_DEACTIVATION);
    }

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
        
        // If "uv" == "up", change "up"
        if (uv.x() == 0 && uv.y() == 0) up.setValue(0, 0, 1);

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

void Skeleton::render(RenderManager& textures)
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
    
    if (head.rigidBody->getActivationState() != DISABLE_SIMULATION) {
        btTransform tf = head.rigidBody->getWorldTransform();
        tf.getOpenGLMatrix(transform);
        glPushMatrix();
            glMultMatrixf(transform);
            gluSphere(q, head.shape->getRadius(), 16, 16);
        glPopMatrix();
    }

    gluDeleteQuadric(q);
}
