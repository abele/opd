#ifndef OPENGLSCENE_H
#define OPENGLSCENE_H

#include "point3d.h"
#include "common.h"

#include <QGraphicsScene>
#include <QLabel>
#include <QTime>

#ifndef QT_NO_CONCURRENT
#include <QFutureWatcher>
#endif

class Model;

class OpenGLScene : public QGraphicsScene
{
    Q_OBJECT

        public:
        OpenGLScene();

        /**
         Iformation about loaded model
         */
        QString modelFileName();
        int modelFaces();
        int modelEdges();
        int modelPoints();

        /**
          Camera coordinates
          */
        float cameraX();
        float cameraY();
        float cameraZ();

        /**
          Draw scene
          */
        void drawBackground(QPainter *painter, const QRectF &rect);

    public slots:
        void enableWireframe(bool enabled);
        void enableNormals(bool enabled);
        void setModelColor();
        void setBackgroundColor();
        void loadModel();
        void loadModel(const QString &filePath);
        void modelLoaded();

        void enableOrthographicProjection(bool enabled);

        signals:
        void loading(bool);
        void cameraCoordinatesChanged();

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void wheelEvent(QGraphicsSceneWheelEvent * wheelEvent);

    private:
        QDialog *createDialog(const QString &windowTitle) const;

        void setModel(Model *model);

        typedef void (OpenGLScene::*ProjectionMethod)();
        /** Pointer to projection method. Default is perspectiveProjection() */
        ProjectionMethod projectionMethodPtr;
        inline void perspectiveProjection();
        inline void orthographicProjection();

        inline void drawAxes();

        bool m_wireframeEnabled;
        bool m_normalsEnabled;

        QColor m_modelColor;
        QColor m_backgroundColor;

        Model *m_model;

        QTime m_time;
        int m_lastTime;
        int m_mouseEventTime;

        double CAMERA_TRANSFORMATION[Common::MATRIX_ELEMENT_COUNT];
        double ORTOGRAPHIC_PROJECTION[Common::MATRIX_ELEMENT_COUNT];

        /** Viever point */
        Point3d m_camera;

        float m_distance;
        Point3d m_rotation;
        Point3d m_angularMomentum;
        Point3d m_accumulatedMomentum;

        double m_aspect;

        static const GLdouble NEAR = 0.01;
        static const GLdouble FAR = 1000;
        const GLdouble FmN;
        const GLdouble FpN;
        const GLdouble F;                         //F=ctg(FOVY/2);
        static const GLdouble FOVY = 70;
        static const GLdouble EPSILON = 0.000001;

        QGraphicsRectItem *m_lightItem;

    #ifndef QT_NO_CONCURRENT
        QFutureWatcher<Model *> m_modelLoader;
    #endif
};
#endif
