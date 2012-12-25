#include "mainwindow.h"
#include "graphwidget.h"
#include "zipplcontroler.h"

#include <QtGui>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
  mGraphWidget = new GraphWidget;
  setCentralWidget( mGraphWidget );
  mController = new ZipplControler( mGraphWidget );
 }

void MainWindow::openFile( int argc, char *argv[] )
{
  QString file;
  qDebug() << "ARGC: " << argc;
  qDebug() << "ARGV: " << argv;

  bool res = false;
  if( argc > 1 ) {
    file = argv[1];
  } else {
    file = autoFile();
    qDebug() << "Auto-File found: " << file;
  }
  res = mController->loadFile( file );

  if( ! res ) {
    file = QFileDialog::getOpenFileName( this, tr("Pick a Zippl file to Open"),
                                         QDir::homePath(), tr("Zippl Files (*xml)"));
    res = mController->loadFile( file );
  }
  if( ! res ) exit(0);
}

QString MainWindow::autoFile() const
{
  QFileInfo info( "zippl.xml" );
  if( info.exists() ) {
    return info.absoluteFilePath();
  }
  return QString();
}

MainWindow::~MainWindow()
{

}
