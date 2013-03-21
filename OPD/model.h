#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QVector>
#include <QColor>

#include <math.h>

#include "point3d.h"

/**
  Model class defines 3D object model.
  */
class Model
{
    public:
        Model() {}
        /**
          Construct 3d model from file
          **/
        Model(const QString &filePath);

        /**
          Calls all OpenGl rendering functions.
          */
        void render(QColor m_modelColor, bool wireframe = false, bool normals = false) const;

        /**
          Returns file name of model.
          */
        QString fileName() const { return m_fileName; }
        /**
          Returns number of faces in model
          */
        int faces() const { return m_pointIndices.size() / 3; }
        /**
          Returns number of edges in model.
          */
        int edges() const { return m_edgeIndices.size() / 2; }
        /**
          Returns number of vertices in model.
          */
        int points() const { return m_points.size(); }

    private:
        /**
          Model file name
          */
        QString m_fileName;
        /**
          Loaded points
          */
        QVector<Point3d> m_points;
        /**
         Loaded normals
         */
        QVector<Point3d> m_normals;
        QVector<int> m_edgeIndices;
        QVector<int> m_pointIndices;
};
#endif
