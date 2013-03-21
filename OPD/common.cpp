#include "common.h"
#include <QtOpenGL>
void Common::drawGizmo()
{
    glBegin (GL_LINES);
    // X axis - red
    glColor3f (1.0, 0.0, 0.0);
    glVertex3f (0.0, 0.0, 0.0);
    glVertex3f (1.0, 0.0, 0.0);

    glVertex3f (1.0, 0.0, 0.0);
    glVertex3f (0.95, 0.05, 0.0);

    glVertex3f (1.0, 0.0, 0.0);
    glVertex3f (0.95, -0.05, 0.0);

    // Y axis - green
    glColor3f (0.0, 1.0, 0.0);
    glVertex3d (0.0, 0.0, 0.0);
    glVertex3d (0.0, 1.0, 0.0);

    glVertex3f (0.0, 1.0, 0.0);
    glVertex3f (0.05, 0.95, 0.0);

    glVertex3f (0.0, 1.0, 0.0);
    glVertex3f (-0.05, 0.95, 0.0);

    // Z axis - blue
    glColor3f (0.0, 0.0, 1.0);
    glVertex3d (0.0, 0.0, 0.0);
    glVertex3d (0.0, 0.0, 1.0);

    glVertex3f (0.0, 0.0, 1.0);
    glVertex3f (0.0, 0.05, 0.95);

    glVertex3f (0.0, 0.0, 1.0);
    glVertex3f (0.0, -0.05, 0.95);

    glEnd();
};
