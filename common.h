#ifndef COMMON_H
#define COMMON_H

#include <QtOpenGL>

namespace Common
{
    /**
      Element count in tranformation matrix
      */
    const int MATRIX_ELEMENT_COUNT = 16;

    const GLdouble  IDENTITY_MATRIX [MATRIX_ELEMENT_COUNT] =
    {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    };
    /**
      Draw x, y, z gizmo
      */
    void drawGizmo();
};
#endif                                            // COMMON_H
