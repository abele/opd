#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "openglscene.h"

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* Setup central widget */
    m_view = new QGraphicsView();
    m_view->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_view->setScene(new OpenGLScene());
    setCentralWidget(m_view);

    /* Set main menu actions */
    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(ui->actionAbout_programm, SIGNAL(triggered()), this, SLOT(about()));

    /* Connect control widgets with scene */
    connect(ui->checkBox_orthographic_projection, SIGNAL(toggled(bool)), m_view->scene(), SLOT(enableOrthographicProjection(bool)));
    connect(ui->wireframe, SIGNAL(toggled(bool)), m_view->scene(), SLOT(enableWireframe(bool)));
    connect(ui->normals, SIGNAL(toggled(bool)), m_view->scene(), SLOT(enableNormals(bool)));
    connect(ui->chooseModelColor, SIGNAL(clicked()), m_view->scene(), SLOT(setModelColor()));
    connect(ui->chooseBgColor, SIGNAL(clicked()), m_view->scene(), SLOT(setBackgroundColor()));

    /* Connect main menu actions */
    connect(ui->action_Load_model, SIGNAL(triggered(bool)), m_view->scene(), SLOT(loadModel()));
    /* Disable Load model buton when loading model */
    connect(m_view->scene(), SIGNAL(loading(bool)), ui->action_Load_model, SLOT(setDisabled(bool)));
    /* Set model info */
    connect(m_view->scene(), SIGNAL(loading(bool)), this, SLOT(setModelInfo(bool)));
    connect(m_view->scene(), SIGNAL(cameraCoordinatesChanged()), this, SLOT(setCameraCoordinates()));

}


void MainWindow::setModelInfo(bool b)
{
    if (!b)
    {
        ui->file->setText(tr("File: %0").arg(((OpenGLScene *)m_view->scene())->modelFileName()));
        ui->points->setText(tr("Points: %0").arg(((OpenGLScene *)m_view->scene())->modelPoints()));
        ui->edges->setText(tr("Edges: %0").arg(((OpenGLScene *)m_view->scene())->modelEdges()));
        ui->faces->setText(tr("Faces: %0").arg(((OpenGLScene *)m_view->scene())->modelFaces()));
    }
}


void MainWindow::setCameraCoordinates()
{
    ui->vp_x->setText(tr("x = %0").arg(((OpenGLScene *)m_view->scene())->cameraX()));
    ui->vp_y->setText(tr("y = %0").arg(((OpenGLScene *)m_view->scene())->cameraY()));
    ui->vp_z->setText(tr("z = %0").arg(((OpenGLScene *)m_view->scene())->cameraZ()));
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::about()
{
    QMessageBox::about(this, tr("About Application"),
        tr("The <b>Application</b> demonstrates Orthographic projection from variable oserver point."));
}
