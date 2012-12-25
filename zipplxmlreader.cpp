#include <QtGui>
#include <QString>
#include <QtXml>

#include "zipplxmlreader.h"
#include "graphwidget.h"

ZipplXmlReader::ZipplXmlReader( GraphWidget *widget )
  :mGraphWidget( widget ),
  mToc(0),
  mMode( "production")
{

}

qreal ZipplXmlReader::qrealAttrib( const QString& name )
{
  qreal r = 0;
  if( attributes().hasAttribute(name)) {
    bool ok;
    r = attributes().value( name ).toString().toDouble(&ok);
    if( ! ok ) {
      qDebug() << "ERR: real conversion fails for " << attributes().value(name).toString();
    }
  } else {
    qDebug() << "No such attribute " << name;
  }
  return r;
}

QPointF ZipplXmlReader::position()
{
  qreal x = qrealAttrib("x");
  qreal y = qrealAttrib("y");
  return QPointF(x,y);
}

QPen ZipplXmlReader::pen( const QPen& p, const QColor& defaultCol )
{
  QPen pen( p );
  qreal r = qrealAttrib( "linewidth" );
  if( r > 0 ) {
    pen.setWidthF( r );
  }
  pen.setColor( color( defaultCol ) );
  return pen;
}

QBrush ZipplXmlReader::brush( const QBrush& brush )
{
  QBrush b(brush);

  if( attributes().hasAttribute("backgroundcolor")) {
    b.setStyle( Qt::SolidPattern );
    b.setColor( attributes().value("backgroundcolor").toString());
  }
  return b;
}

QColor ZipplXmlReader::color( const QColor& defaultCol )
{
  QColor col( defaultCol );

  if( attributes().hasAttribute( "color")) {
    QString cStr = attributes().value( "color" ).toString();

    if( cStr.startsWith('#')) {
      qDebug() << "Setting background-color: " << col;
      col.setNamedColor( cStr );
    }
  }
  return col;
}

bool ZipplXmlReader::read( QIODevice *dev )
{
  setDevice( dev );
  bool res = true;
  bool metaMode = false;
  mCurrParent = 0;
  mCurrSpot = 0;

  QGraphicsScene *scene = mGraphWidget->scene();
  int spotID = 0;

  while (!atEnd()) {
    readNext();

    if( isStartElement() ) {
      qDebug() << "XML name: " << name();

      if( name() == "presentation") {
        // presentation mode: debug & presentation
        QString mode = attributes().value("mode").toString();
        if( !mode.isEmpty() ) mMode = mode;

        mPath = attributes().value("path").toString();
        if( !mPath.endsWith('/') ) mPath += "/";


        qreal dx = qrealAttrib("width") / -2.0;
        qreal dy = qrealAttrib("height") / -2.0;
        QRectF rect( dx, dy, -2.0*dx, -2.0*dy );
        scene->setSceneRect( rect );
      } else if( name() == "meta" ) {
        metaMode = true;
      } else if( name() == "title" && metaMode ) {
        mPresentationTitle = readElementText();
      } else if( name() == "description" && metaMode ) {
        mPresentationDescr = readElementText();
      } else if( name() == "date" && metaMode ) {
        mPresentationDate = readElementText();
      } else if( name() == "name" && metaMode ) {
        mAuthorName = readElementText();
      } else if( name() == "email" && metaMode ) {
        mAuthorEmail = readElementText();
      } else if( name() == "tocentry" ) {
        if( mCurrSpot ) {
          mCurrSpot->setData( TOCENTRY, readElementText() );
        }
      } else if( name() == "spot" ) {
        if( mCurrParent != 0 ) {
          qDebug() << "Strange: Current Parent should be zero here!";
        }
        QGraphicsRectItem *rectItem = new QGraphicsRectItem( );

        rectItem->setPen( pen( rectItem->pen(), QColor("#aeaeae") ));

        mCurrParent = rectItem;
        mCurrSpot = rectItem;
        mCurrParent->setData( ID, QVariant( spotID++ ));

        mCurrParent->setPos( position() );

        rectItem->setBrush( brush( rectItem->brush() ) );

        scene->addItem( mCurrParent );
        mSpots.append( mCurrParent );

        // Prepare the hidden items list
        GraphicsItemList list;
        mHiddenItems.insert( mCurrParent, list );

      } else if( name() == "hidden" ) {
        QGraphicsRectItem *rectItem = new QGraphicsRectItem( mCurrParent, scene );
        rectItem->setPen( QPen( QColor( 240, 240, 240 )));

        // append this hidden item to the list of hiddens of the parent spot.
        GraphicsItemList list = mHiddenItems[mCurrSpot];
        list.append( rectItem );
        mHiddenItems[mCurrSpot] = list;

        mCurrParent = rectItem;
        mCurrParent->setData( ID, QVariant( spotID++ ));

      } else if( name() == "rect" ) {
        if( mCurrParent ) { // within a spot
          qDebug() << "Creating a rectangle!";
          QGraphicsRectItem *rectItem = new QGraphicsRectItem( mCurrParent, scene );

          qreal width = qrealAttrib( "width" );
          qreal height = qrealAttrib( "height" );

          QPointF pos = position();
          if( width > 0 && height > 0 ) {
            rectItem->setRect( pos.x(), pos.y(), width, height );
          } else {
            rectItem->setPos( pos );
          }
          rectItem->setPen( pen( rectItem->pen() ) );

          mCurrParent = rectItem;
        }
      } else if( name() == "circle" ) {
        QPointF pos = position();
        QGraphicsEllipseItem *ellipse = new QGraphicsEllipseItem( mCurrParent, scene );
        // ellipse->setBrush( getBrush() );
        qreal r = 2.0 * qrealAttrib( "r" );

        QRectF rect( pos, QSizeF( r, r ) );

        ellipse->setPen( pen( ellipse->pen() ) );

        ellipse->setRect( rect );


      } else if( name() == "text" ) {
        QGraphicsSimpleTextItem *textItem = new QGraphicsSimpleTextItem( mCurrParent, scene );

        QString font = attributes().value("font").toString();
        QString size = attributes().value("size").toString();


        QFont currFont = textItem->font();
        if( !font.isEmpty() ) {
          currFont.setFamily( font );
          textItem->setFont( currFont );
        }
        if( !size.isEmpty() ) {
          currFont.setPointSize( size.toInt() );
          textItem->setFont( currFont );
        }

        textItem->setPos( position() );

        // set the brush
        QBrush b( textItem->brush() );
        b.setColor( color() );

        textItem->setBrush( b );

        QString text = readElementText();
        textItem->setText( text );

      } else if( name() == "image" ) {
        if( handleImg( scene ) ) {

        }
      }
    } else if( isEndElement() ) {
      qDebug( ) << "XML CLOSE: " << name().toString();
      if( name() == "spot" || name() == "toc" ) {
        QRectF rect = mCurrParent->childrenBoundingRect();
        rect.setX(0);
        rect.setY(0);
        qgraphicsitem_cast<QGraphicsRectItem*>(mCurrParent)->setRect( rect);
        mCurrParent = 0;
      } else if( name() == "rect" ) {
        QGraphicsRectItem *item = qgraphicsitem_cast<QGraphicsRectItem*>(mCurrParent);

        if( item->rect().isEmpty() )
          item->setRect( mCurrParent->childrenBoundingRect() );
        mCurrParent = mCurrParent->parentItem();
      } else if( name() == "hidden") {
        mCurrParent->setOpacity( 0.0 );
        mCurrParent = mCurrParent->parentItem();
      } else if( name() == "meta" ) {
        metaMode = false;
      }
    }
  }

  createToc( scene );

  return res;
}

void ZipplXmlReader::createToc( QGraphicsScene *scene )
{
   // now create the toc entry.
  QString tocHtml;
  if( !mPresentationTitle.isEmpty() ) tocHtml += QString("<h1>%1</h1>").arg( mPresentationTitle );
  if( !mPresentationDescr.isEmpty() ) tocHtml += QString("<h2>%1</h2>").arg( mPresentationDescr);

  tocHtml += "<p>Table of Contents:</p>";
  tocHtml += "<ol>";
  foreach( QGraphicsItem *spot, mSpots ) {
    QString tocString = spot->data(TOCENTRY).toString();
    tocHtml += QString( "<li>%1</li>").arg( tocString );
  }
  tocHtml += "</ol>";

  if( !mAuthorName.isEmpty() ) {
    tocHtml += QString( "<p>%1" ).arg( mAuthorName );
    if( !mAuthorEmail.isEmpty() ) {
      tocHtml += QString( " &lt;%1&gt;" ).arg( mAuthorEmail );
    }
    if( !mPresentationDate.isEmpty() )  tocHtml += QString(" - %1").arg( mPresentationDate );
    tocHtml += "</p>";
  }

  QGraphicsRectItem *tocRectItem = new QGraphicsRectItem( );
  tocRectItem->setPen( pen( tocRectItem->pen(), QColor("#aeaeae") ));

  QBrush brush( tocRectItem->brush() );
  brush.setColor( QColor( 255,255,255 ));
  brush.setStyle( Qt::Dense1Pattern );
  tocRectItem->setBrush( brush );

  int margin = 20;
  QGraphicsTextItem *tocItem = new QGraphicsTextItem( tocRectItem, scene );
  tocItem->setHtml( tocHtml );
  QRectF r = tocRectItem->childrenBoundingRect();
  r.setWidth( r.width() + margin );
  r.setHeight( r.height() + margin );
  tocRectItem->setRect( r );
  tocItem->moveBy( margin/2, margin/2 );

  mToc = tocRectItem;
  mToc->moveBy( r.width() /-2, r.height() / -2 );
  scene->addItem( mToc );
}

bool ZipplXmlReader::handleImg( QGraphicsScene *scene )
{
  if( ! scene ) return false;
  qreal width = qrealAttrib( "width" );
  qreal height = qrealAttrib( "height" );

  QGraphicsPixmapItem *item = new QGraphicsPixmapItem( mCurrParent, scene );
  item->setPos( position() );

  QString imgName = mPath + readElementText();
  bool ret = true;

  if(!QFile::exists( imgName )) {
    qDebug() << "Image file does not exist!";
    ret = false;
  }
  if( ret ) {
    QPixmap pixmap( imgName );
    if( width < 1.0 && height > 0 ) {
      pixmap = pixmap.scaledToHeight( height );
    }
    if( height < 1.0 && width > 0 ) {
      pixmap = pixmap.scaledToWidth( width );
    }

    if( height > 0 && width > 0 ) {
      pixmap = pixmap.scaled( QSize( width, height ) );
    }
    item->setPixmap( pixmap );
  }
  return ret;
}

QString ZipplXmlReader::mode()
{
  return mMode;
}

QGraphicsItem* ZipplXmlReader::toc()
{
  return mToc;
}

QList<QGraphicsItem*> ZipplXmlReader::spotItems()
{
  return mSpots;
}

QMap<QGraphicsItem*, GraphicsItemList> ZipplXmlReader::hiddenItems()
{
  return mHiddenItems;
}
