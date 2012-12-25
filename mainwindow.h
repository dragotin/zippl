#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

class GraphWidget;
class ZipplControler;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();
  void openFile( int argc, char *argv[] );
private:
  QString autoFile() const;

  GraphWidget *mGraphWidget;
  ZipplControler *mController;
};

#endif // MAINWINDOW_H
