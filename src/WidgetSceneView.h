#ifndef WIDGETSCENEVIEW_H
#define WIDGETSCENEVIEW_H

#pragma once

#include "globals.h"

#include "Calibrator.h"

#include "RenderUtils.h"
#include "DataStream.h"
#include "WidgetOpenGL.h"

class WidgetSceneView : public WidgetOpenGL
{
    private:
        std::vector< Ptr<DataStream> > streams;

        std::vector<cv::Mat> transforms;

        GLUquadric* quadric;

        struct
        {
            float x, y, z;
            float pitch, yaw;
        } cam;


        struct
        {
            char w, a, s, d;
            char up, down, left, right;
        } keys;

        struct
        {
            int x, y;
        } mouse;

    public:
        WidgetSceneView(MainWindow& mainWindow) :
            WidgetOpenGL(mainWindow),
            cam(),
            keys(),
            mouse()
        {
            type = DepthFrameView;

            cam.y = 2.0f;
            cam.z = 4.0f;
            cam.pitch = -30.0f;

            auto& streamList = mainWindow.getStreams();
            
            auto& calib = mainWindow.getCalibration().getCalibratedWith(0);
            for (int i = 0; i < int(calib.size()); ++i) {
                streams.push_back(streamList[calib[i].first]);
                transforms.push_back(calib[i].second.getTransformMatrix().t());
                //qDebug() << QString::fromStdString(Utils::matToString<float>(calib[i].second.getTransformMatrix()));
            }

            setFocusPolicy(Qt::StrongFocus);
        }

        virtual ~WidgetSceneView()
        {
            makeCurrent();

            gluDeleteQuadric(quadric);
        }

        void initialize() override
        {
            quadric = gluNewQuadric();

            glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);

            glEnable(GL_LIGHT0);

            //texture = RenderUtils::createTexture(DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT);
        }

        bool render()
        {
            float ratio = float(width()) / float(height());

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluPerspective(70.0f, ratio, 0.01f, 100.0f);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            
            // Set the scene camera position
            glRotatef(-cam.pitch, 1.0f, 0.0f, 0.0f);
            glRotatef(cam.yaw, 0.0f, 1.0f, 0.0f);
            glTranslatef(-cam.x, -cam.y, -cam.z);

            const float LIGHT_POS[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
            glLightfv(GL_LIGHT0, GL_POSITION, LIGHT_POS);


            // Draw horizontal plane grid and axes
            drawGrid(5);
            drawAxes();

            glEnable(GL_LIGHTING);

            // Draw the base camera
            glColor3f(1.0f, 1.0f, 1.0f);
            drawCam();

            // Draw the other cameras
            for (int i = 0; i < int(streams.size()); ++i) {
                glColor3f(0.0f, 1.0f, 1.0f);
                glPushMatrix();
                    glMultMatrixf((GLfloat*)transforms[i].ptr());
                    drawCam();
                glPopMatrix();
            }

            glDisable(GL_LIGHTING);

            return true;
        }


    protected:

        void setKey(int key, int x)
        {
            switch (key) {
                case Qt::Key_W: keys.w = x; break;
                case Qt::Key_A: keys.a = x; break;
                case Qt::Key_S: keys.s = x; break;
                case Qt::Key_D: keys.d = x; break;
                case Qt::Key_Left: keys.left = x; break;
                case Qt::Key_Up: keys.up = x; break;
                case Qt::Key_Right: keys.right = x; break;
                case Qt::Key_Down: keys.down = x; break;
            }
        }

        void keyPressEvent(QKeyEvent* ev) override
        {
            setKey(ev->key(), 1);
        }

        void keyReleaseEvent(QKeyEvent* ev) override
        {
            setKey(ev->key(), 0);
        }

        void focusOutEvent(QFocusEvent* ev) override
        {
            memset(&keys, 0, sizeof(keys));
        }

        void mousePressEvent(QMouseEvent* ev) override
        {
            if (ev->button() == Qt::LeftButton) {
                mouse.x = ev->pos().x();
                mouse.y = ev->pos().y();
            }
        }

        void mouseMoveEvent(QMouseEvent* ev) override
        {
            const float ANGLE_STEP = 2.0f;

            if (ev->buttons() & Qt::LeftButton) {
                int x = ev->pos().x();
                int y = ev->pos().y();
                cam.pitch -= ANGLE_STEP * (y - mouse.y);
                cam.yaw += ANGLE_STEP * (x - mouse.x);
                mouse.x = x;
                mouse.y = y;
            }
        }

        void onTick() override
        {
            const float POS_STEP = 0.2f;
            const float ANGLE_STEP = 2.0f;

            int dx = keys.d - keys.a;
            int dy = keys.w - keys.s;

            if (dx != 0 || dy != 0) {
                const float sinYaw = sin(DEG2RAD * cam.yaw);
                const float cosYaw = cos(DEG2RAD * cam.yaw);
                const float sinPitch = sin(DEG2RAD * cam.pitch);
                const float cosPitch = cos(DEG2RAD * cam.pitch);

                cam.x += POS_STEP * (dx * cosYaw + dy * sinYaw * cosPitch);
                cam.y += POS_STEP * (dy * sinPitch);
                cam.z += POS_STEP * (dx * sinYaw - dy * cosYaw * cosPitch);
            }


            int dPitch = keys.up - keys.down;
            int dYaw = keys.right - keys.left;

            if (dPitch != 0 || dYaw != 0) {
                cam.pitch += ANGLE_STEP * dPitch;
                cam.yaw += ANGLE_STEP * dYaw;
            }
        }

        void drawGrid(int range, float step = 1.0f)
        {
            float limit = (range + 1) * step;
            glColor3f(0.25f, 0.25f, 0.25f);
            glBegin(GL_LINES);
            for (int i = -range; i <= range; ++i) {
                float pos = i * step;

                glVertex3f(pos, 0.0f, limit);
                glVertex3f(pos, 0.0f, -limit);

                glVertex3f(limit, 0.0f, pos);
                glVertex3f(-limit, 0.0f, pos);
            }
            glEnd();
        }

        void drawAxes()
        {
            glDisable(GL_DEPTH_TEST);
            glBegin(GL_LINES);
            {
                glColor3f(1.0f, 0.0f, 0.0f);
                glVertex3f(0.0f, 0.0f, 0.0f);
                glVertex3f(1.0f, 0.0f, 0.0f);

                glColor3f(0.0f, 1.0f, 0.0f);
                glVertex3f(0.0f, 0.0f, 0.0f);
                glVertex3f(0.0f, 1.0f, 0.0f);

                glColor3f(0.0f, 0.0f, 1.0f);
                glVertex3f(0.0f, 0.0f, 0.0f);
                glVertex3f(0.0f, 0.0f, 1.0f);
            }
            glEnd();
            glEnable(GL_DEPTH_TEST);
            //gluCylinder(quadric, 0.1f, 0.0f, 0.1f, 8, 8);
        }

        void drawCam()
        {
            //gluSphere(quadric, 0.1f, 8, 8);
            glPushMatrix();
            {
                glTranslatef(0.0f, 0.0f, -0.1f);
                gluCylinder(quadric, 0.05f, 0.0f, 0.1f, 12, 1);
            }
            glPopMatrix();
        }


};

#endif
