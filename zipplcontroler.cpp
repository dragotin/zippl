#include "zipplcontroler.h"
#include "zipplxmlreader.h"

#include <QtGui>

ZipplControler::ZipplControler( GraphWidget *graphWidget )
  :QObject(),
  mGraphWidget( graphWidget ),
  mCurrSpotNo(-1),
  mCurrStationNo(0),
  mWatcher( new QFileSystemWatcher(this))
{

  connect( mGraphWidget, SIGNAL( moveToNextSpot()), SLOT( slotNextSpot() ) );
  connect( mGraphWidget, SIGNAL( moveToNextStation()), SLOT( slotNextHidden() ) );
  connect( mGraphWidget, SIGNAL( moveToPreviousSpot()), SLOT( slotPreviousSpot() ) );

  connect( mWatcher, SIGNAL(fileChanged( const QString&)), SLOT( slotFileChanged( const QString& )));

}

bool ZipplControler::loadFile( const QString& filePath )
{
  QFile file( filePath );

  if( !file.open( QIODevice::ReadOnly ) ) {
    qDebug() << "Cannot open File: " <<
        qPrintable(file.errorString()) << endl;
    return false;
  }

  ZipplXmlReader xmlReader( mGraphWidget );
  bool res = xmlReader.read( &file );

  if( res ) {
    qDebug() << "Successfully read zippl in file!";
    mSpotList = xmlReader.spotItems();
    mHiddenItems = xmlReader.hiddenItems();
    mWatcher->addPath( filePath );
    mTocItem = xmlReader.toc();
  } else {
    qDebug() << "Failed to read zippl in file!";
  }
  return res;
}

void ZipplControler::slotFileChanged( const QString& file )
{
  qDebug() << "File changed - reload!";
  mGraphWidget->scene()->clear();
  loadFile( file );
}

QGraphicsItem *ZipplControler::currentSpot()
{

  if( mCurrSpotNo >= mSpotList.count() || mCurrSpotNo < 0 ) {
    mCurrSpotNo = 0;
  }
  return mSpotList[mCurrSpotNo];
}

void ZipplControler::slotNextSpot()
{
  qDebug() << "Move to next spot, current spot no is " << mCurrSpotNo;

  if( mCurrSpotNo < 0 ) {
    // Currently the toc is shown. Hide it and move on.
    qDebug() << "HIDING the TOC";
    mTocItem->hide();
  }
  mCurrSpotNo++;
  QGraphicsItem *currItem = currentSpot();

  // turn to next Spot.

  centerOnItem( currItem );
}


void ZipplControler::slotNextHidden()
{
  // take the current Spot
  QGraphicsItem *spotItem = currentSpot();

  qDebug() << "XXX -> Undiscover a hidden one , current Spot is " << currentSpot();

  // search the current item in the hidden spot list.
  QList<QGraphicsItem*> hiddenItems = mHiddenItems[spotItem];
  QGraphicsItem* itemFound = 0;

  foreach( QGraphicsItem *item, hiddenItems ) {
    qDebug() << "Hidden-Loop!";
    // the next hidden item which is not visible is going to be shown.
    if( item->opacity() < qreal(1.0) ) {
      itemFound = item;
      break;
    }
  }
  if( ! itemFound )  {
    // switch to next spot
    slotNextSpot();
  } else {
    itemFound->setOpacity( qreal(1.0) );
    qDebug() << "XXXXX Setting Opacity to a good value!" << itemFound;
  }

}

void ZipplControler::slotPreviousSpot()
{
  qDebug() << "Move to previous spot, current spot no is " << mCurrSpotNo;

  mCurrSpotNo--;
  QGraphicsItem *currItem = currentSpot();

  // turn to next Spot.

  centerOnItem( currItem );
}

void ZipplControler::centerOnItem( QGraphicsItem* item )
{
  if( !item ) {
    qDebug() << "No current item found!";
  } else {
    qDebug() << "Centering on item " << item->data(ID).toInt();

    qreal width  = mGraphWidget->viewport()->width();
    qreal height = mGraphWidget->viewport()->height();
    qDebug() << "Viewport width and height: " << width << "x" << height;
    QPointF viewPoint = item->scenePos();
    qDebug() << "ViewPoint: " << viewPoint.x() << "x" << viewPoint.y();


    qDebug() << "== Animation! ==";
    QPropertyAnimation *animHor = new QPropertyAnimation( mGraphWidget->horizontalScrollBar(), "value" );
    // Set up horizontal scroll animation
    animHor->setDuration(1000);

    int startValHor = mGraphWidget->horizontalScrollBar()->value();
    int minimum = mGraphWidget->horizontalScrollBar()->minimum();
    int maximum = mGraphWidget->horizontalScrollBar()->maximum();
    int endValHor = viewPoint.x() - width / 2.0 + item->boundingRect().width()/2.0;

    qDebug() << "Min/Max: " << minimum << "x" << maximum;
    qDebug() << "Startvalue horizontal: " << startValHor;
    qDebug() << "Item Bounding Rect: " << item->boundingRect();
    qDebug() << "Endvalue horizontal: " << endValHor;

    animHor->setStartValue( QVariant( startValHor ) );
    animHor->setEndValue( QVariant(endValHor) );
    animHor->setEasingCurve(QEasingCurve::OutBack);

    QPropertyAnimation *animVer = new QPropertyAnimation( mGraphWidget->verticalScrollBar(), "value" );
    animVer->setDuration(1000);
    animVer->setStartValue( QVariant( mGraphWidget->verticalScrollBar()->value()) );
    animVer->setEasingCurve(QEasingCurve::OutBack);
    int endValVer = viewPoint.y() - height/ 2.0 + item->boundingRect().height()/2.0;
    animVer->setEndValue( QVariant( endValVer ) );


    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation( animHor );
    group->addAnimation( animVer );

    group->start();
  }
}

