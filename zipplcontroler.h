#ifndef ZIPPLCONTROLER_H
#define ZIPPLCONTROLER_H

#include "graphwidget.h"
#include "zipplxmlreader.h"
class QFileSystemWatcher;



class ZipplControler:public QObject
{
  Q_OBJECT
public:
  ZipplControler( GraphWidget* );
  bool loadFile( const QString& );
  QGraphicsItem *currentSpot();

protected slots:
  void slotNextSpot();
  void slotNextHidden();
  void slotPreviousSpot( );

  void centerOnItem( QGraphicsItem* );
  void slotFileChanged( const QString& );


private:
  GraphWidget *mGraphWidget;
  QList<QGraphicsItem*> mSpotList;
  // hidden items for each spot
  QMap<QGraphicsItem*, GraphicsItemList> mHiddenItems;
  int          mCurrSpotNo;
  int          mCurrStationNo;
  QFileSystemWatcher *mWatcher;
  QGraphicsItem *mTocItem;
};

#endif // ZIPPLCONTROLER_H
