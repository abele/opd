#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QGraphicsView>
#include <QGLWidget>

namespace Ui
{
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

        public:
        MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private:
        QGraphicsView* m_view;
        Ui::MainWindow *ui;
    private slots:
        void setModelInfo(bool);
        void setCameraCoordinates();
        void about();
};
#endif                                            // MAINWINDOW_H
