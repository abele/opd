#include "openglscene.h"
#include "model.h"
#include "common.h"
#include <math.h>

#include <QtGui>
#include <QtOpenGL>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

using namespace Common;

void
gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    GLdouble xmin, xmax, ymin, ymax;

    ymax = zNear * tan(fovy * M_PI / 360.0);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    glFrustum(xmin, xmax, ymin, ymax, zNear, zFar);
}


const GLfloat DEG = M_PI/180;
QDialog *OpenGLScene::createDialog(const QString &windowTitle) const
{
    QDialog *dialog = new QDialog(0, Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    dialog->setWindowOpacity(0.8);
    dialog->setWindowTitle(windowTitle);
    dialog->setLayout(new QVBoxLayout);

    return dialog;
}


OpenGLScene::OpenGLScene()
: projectionMethodPtr(&OpenGLScene::perspectiveProjection)
, m_wireframeEnabled(false)
, m_normalsEnabled(false)
, m_modelColor(255, 255, 255)
, m_backgroundColor(0, 0, 0)
, m_model(0)
, m_lastTime(0)
, m_camera(0.0, 0.0, 0.0)
, m_distance(1.4f)
, m_angularMomentum(0, 40, 0)
, FmN(FAR-NEAR)
, FpN(FAR+NEAR)
, F(tan(FOVY * M_PI / 360.0))
{

    /*
      0  4  8  12
      1  5  9  13
      2  6  10 14
      3  7  11 15
    */
    /* Identity matrix */
    m_aspect = width() / height();
    for (int i=0; i < MATRIX_ELEMENT_COUNT; ++i)
    {
        CAMERA_TRANSFORMATION[i] = IDENTITY_MATRIX[i];
        ORTOGRAPHIC_PROJECTION[i] = IDENTITY_MATRIX[i];
    }

    /* Constant elements */
    ORTOGRAPHIC_PROJECTION[10] = -2.0/FmN;
    ORTOGRAPHIC_PROJECTION[14] = -FpN/FmN;

    #ifndef QT_NO_CONCURRENT
    connect(&m_modelLoader, SIGNAL(finished()), this, SLOT(modelLoaded()));
    #endif

    QPointF pos(10, 10);
    foreach (QGraphicsItem *item, items())
    {
        item->setFlag(QGraphicsItem::ItemIsMovable);
        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        const QRectF rect = item->boundingRect();
        item->setPos(pos.x() - rect.x(), pos.y() - rect.y());
        pos += QPointF(0, 10 + rect.height());
    }

    QRadialGradient gradient(40, 40, 40, 40, 40);
    gradient.setColorAt(0.2, Qt::yellow);
    gradient.setColorAt(1, Qt::transparent);

    m_lightItem = new QGraphicsRectItem(0, 0, 80, 80);
    m_lightItem->setPen(Qt::NoPen);
    m_lightItem->setBrush(gradient);
    //m_lightItem->setFlag(QGraphicsItem::ItemIsMovable);
    m_lightItem->hide();
    m_lightItem->setPos(400, 200);
    addItem(m_lightItem);

    /* Default model */
    loadModel(QLatin1String("./qt.obj"));
    m_time.start();
}


void OpenGLScene::drawBackground(QPainter *painter, const QRectF &)
{
    if (painter->paintEngine()->type() != QPaintEngine::OpenGL
        && painter->paintEngine()->type() != QPaintEngine::OpenGL2)
    {
        qWarning("OpenGLScene: drawBackground needs a QGLWidget to be set as viewport on the graphics view");
        return;
    }

    painter->beginNativePainting();

    // Background color
    glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), m_backgroundColor.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // const GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    if (m_model)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        m_aspect = width() / height();
        // Pointer to projection method
        (this->*projectionMethodPtr)();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        const int delta = m_time.elapsed() - m_lastTime;
        m_rotation += m_angularMomentum * (delta / 1000.0);
        m_lastTime += delta;

        float sp = sin(m_rotation.x*DEG);
        float cp = cos(m_rotation.x*DEG);

        float sy = sin(m_rotation.y*DEG);
        float cy = cos(m_rotation.y*DEG);

        float sr = sin(m_rotation.z*DEG);
        float cr = cos(m_rotation.z*DEG);

        m_camera.x = m_distance*cp*sy;
        m_camera.y = -m_distance*sp;
        m_camera.z = m_distance*cp*cy;

        emit cameraCoordinatesChanged();

        CAMERA_TRANSFORMATION[0] = sp*sr*sy+cr*cy;
        CAMERA_TRANSFORMATION[1] = sp*cr*sy-sr*cy;
        CAMERA_TRANSFORMATION[2] = cp*sy;
        CAMERA_TRANSFORMATION[4] = cp*sr;
        CAMERA_TRANSFORMATION[5] = cp*cr;
        CAMERA_TRANSFORMATION[6] = -sp;
        CAMERA_TRANSFORMATION[8] = sp*sr*cy-cr*sy;
        CAMERA_TRANSFORMATION[9] = sr*sy+sp*cr*cy;
        CAMERA_TRANSFORMATION[10] = cp*cy;
        CAMERA_TRANSFORMATION[12] = sr*(sp*(-m_camera.x*sy-m_camera.z*cy)-cp*m_camera.y)+cr*(m_camera.z*sy-m_camera.x*cy);
        CAMERA_TRANSFORMATION[13] = cr*(sp*(-m_camera.x*sy-m_camera.z*cy)-cp*m_camera.y)+sr*(m_camera.z*sy-m_camera.x*cy);
        CAMERA_TRANSFORMATION[14] = cp*(-m_camera.x*sy-m_camera.z*cy)+sp*m_camera.y;

        glMultMatrixd(CAMERA_TRANSFORMATION);

        //const float pos[] = { m_lightItem->x() - width() / 2, height() / 2 - m_lightItem->y(), 512, 0 };
        //glLightfv(GL_LIGHT0, GL_POSITION, pos);

        //Light position
        const GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
        const GLfloat position2[] = { -2.0f, 2.0f, +4.0f, 1.0f };

        // Create light components GL_LIGHT0
        const GLfloat diffuseLight2[] = { 0.3f, 0.3f, 0.3, 1.0f };

        // Assign created components to GL_LIGHT0
        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight2);
        glLightfv(GL_LIGHT0, GL_POSITION, position);

        // Create light components to GL_LIGHT1
        const GLfloat ambientLight[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        const GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
        const GLfloat specularLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };

        // Assign created components to GL_LIGHT1
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambientLight);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuseLight);
        glLightfv(GL_LIGHT1, GL_SPECULAR, specularLight);
        glLightfv(GL_LIGHT1, GL_POSITION, position2);

        // Model color
        glColor4f(m_modelColor.redF(), m_modelColor.greenF(), m_modelColor.blueF(), 1.0f);
        // Drawing model
        glEnable(GL_MULTISAMPLE);
        m_model->render(m_modelColor, m_wireframeEnabled, m_normalsEnabled);
        glDisable(GL_MULTISAMPLE);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        // Draw axis in corner
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        perspectiveProjection();

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        drawAxes();
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }

    painter->endNativePainting();

    QTimer::singleShot(20, this, SLOT(update()));
}


static Model *loadModel(const QString &filePath)
{
    return new Model(filePath);
}


void OpenGLScene::loadModel()
{
    loadModel(QFileDialog::getOpenFileName(0, tr("Choose model"), QString(), tr("Wavefront OBJ File (*.obj)")));
}


void OpenGLScene::loadModel(const QString &filePath)
{
    if (filePath.isEmpty())
        return;

    emit loading(true);
    QApplication::setOverrideCursor(Qt::BusyCursor);

    #ifndef QT_NO_CONCURRENT
    m_modelLoader.setFuture(QtConcurrent::run(::loadModel, filePath));
    #else
    setModel(::loadModel(filePath));
    modelLoaded();
    #endif
}


void OpenGLScene::modelLoaded()
{
    #ifndef QT_NO_CONCURRENT
    setModel(m_modelLoader.result());
    #endif

    emit loading(false);
    QApplication::restoreOverrideCursor();
}


void OpenGLScene::setModel(Model *model)
{
    delete m_model;
    m_model = model;

    update();
}


void OpenGLScene::enableOrthographicProjection(bool enabled)
{
    if(enabled)
        projectionMethodPtr = &OpenGLScene::orthographicProjection;
    else
        projectionMethodPtr = &OpenGLScene::perspectiveProjection;
    update();
}


void OpenGLScene::enableWireframe(bool enabled)
{
    m_wireframeEnabled = enabled;
    update();
}


void OpenGLScene::enableNormals(bool enabled)
{
    m_normalsEnabled = enabled;
    update();
}


void OpenGLScene::setModelColor()
{
    const QColor color = QColorDialog::getColor(m_modelColor);
    if (color.isValid())
    {
        m_modelColor = color;
        update();
    }
}


void OpenGLScene::setBackgroundColor()
{
    const QColor color = QColorDialog::getColor(m_backgroundColor);
    if (color.isValid())
    {
        m_backgroundColor = color;
        update();
    }
}


void OpenGLScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
    if (event->isAccepted())
    {
        return;
    }

    if (event->buttons() & Qt::LeftButton)
    {
        const QPointF delta = event->scenePos() - event->lastScenePos();
        const Point3d angularImpulse = Point3d(delta.y(), delta.x(), 0) * 0.1;

        m_rotation += angularImpulse;
        m_accumulatedMomentum += angularImpulse;

        event->accept();
        update();
    }
}


void OpenGLScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
    if (event->isAccepted())
    {
        return;
    }

    m_mouseEventTime = m_time.elapsed();
    m_angularMomentum = m_accumulatedMomentum = Point3d();
    event->accept();
}


void OpenGLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    if (event->isAccepted())
    {
        return;
    }

    const int delta = m_time.elapsed() - m_mouseEventTime;
    m_angularMomentum = m_accumulatedMomentum * (1000.0 / qMax(1, delta));
    event->accept();
    update();
}


void OpenGLScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QGraphicsScene::wheelEvent(event);
    if (event->isAccepted())
    {
        return;
    }

    m_distance *= qPow(1.2, -event->delta() / 120);
    event->accept();
    update();
}


QString OpenGLScene::modelFileName()
{
    return m_model->fileName();
}


int OpenGLScene::modelFaces()
{
    return m_model->faces();
}


int OpenGLScene::modelEdges()
{
    return m_model->edges();
}


int OpenGLScene::modelPoints()
{
    return m_model->points();
}


float OpenGLScene::cameraX()
{
    return m_camera.x;
}


float OpenGLScene::cameraY()
{
    return m_camera.y;
}


float OpenGLScene::cameraZ()
{
    return m_camera.z;
}


/**
  |2/(right-left)*m_distance 0               0            -(right+left)/(right-left)|
  |0              2/(top-bottom)*m_distance  0            -(top+bottom)/(top-bottom)|
  |0              0              -2/(far-near) -(far+near)/(far-near)    |
  |0              0               0               1                      |
  */
inline  void OpenGLScene::orthographicProjection()
{
    GLdouble top(F*m_distance);
    GLdouble bottom(-top);
    GLdouble left (bottom * m_aspect);
    GLdouble right(top * m_aspect);

    // Cache calculations
    GLdouble a(right-left);
    GLdouble b(top-bottom);
    GLdouble c(right+left);
    GLdouble d(top+bottom);

    // Main diognal
    ORTOGRAPHIC_PROJECTION[0] = 2.0/(a);
    ORTOGRAPHIC_PROJECTION[5] = 2.0/(b);
    ORTOGRAPHIC_PROJECTION[15] = 1.0;
    // 4th column
    ORTOGRAPHIC_PROJECTION[12] = -c/a;
    ORTOGRAPHIC_PROJECTION[13] = -d/b;

    glLoadMatrixd(ORTOGRAPHIC_PROJECTION);
}


inline void  OpenGLScene::perspectiveProjection()
{

    gluPerspective(FOVY, m_aspect, NEAR, FAR);
}


inline void OpenGLScene::drawAxes (void)
{
    //    1. Change viewport to cover only corner of a screen
    //    2. Reinitialize modelview and projection matrices - apply all rotations and perspective you need (do not apply any scaling or translation).
    //    3. Draw your axes around (0,0,0) point
    //    4. Restore viewport and matrices
    glPushMatrix ();

    glTranslatef (-2.4, -1.5, -5);
    glScalef (0.25, 0.25, 0.25);

    glLineWidth (1.0);
    drawGizmo();

    glPushMatrix();
    /* Rounded point */
    glEnable( GL_POINT_SMOOTH );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glLineWidth (1.0);
    glBegin(GL_LINE);
    glColor3f (0.5, 0.5, 0.5);
    glVertex3f(m_camera.x, m_camera.x, m_camera.x);
    glVertex3f(0, 0, 0);
    glEnd();

    glPointSize( 6.0 );
    glBegin(GL_POINTS);

    glColor3f (1.0, 1.0, 1.0);
    glVertex3f(m_camera.x, m_camera.x, m_camera.x);
    glEnd( );

    glPopMatrix();

    glPopMatrix ();
}
