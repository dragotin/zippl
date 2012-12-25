#ifndef ZIPPLXMLREADER_H
#define ZIPPLXMLREADER_H

#include <QXmlStreamReader>
#include <QIODevice>
#include <QList>
#include <QMap>

#define ID 0
#define TOCENTRY 1

class GraphWidget;
class QGraphicsItem;

typedef QList<QGraphicsItem*> GraphicsItemList;

class ZipplXmlReader : public QXmlStreamReader
{
public:

    ZipplXmlReader( GraphWidget* );

    bool read( QIODevice * );
    QList<QGraphicsItem*> spotItems();

    QMap<QGraphicsItem*, GraphicsItemList> hiddenItems();

    QString mode();
    QGraphicsItem *toc();

private:
    QPointF position();
    qreal qrealAttrib( const QString& );
    QPen pen( const QPen&, const QColor& defaultCol = QColor("#000000"));
    QColor color( const QColor& defaultCol = QColor("#000000") );
    QBrush brush( const QBrush& );

    void createToc( QGraphicsScene* );
    bool handleImg( QGraphicsScene* );
    QGraphicsItem *mCurrParent;
    QGraphicsItem *mCurrSpot;

    GraphWidget *mGraphWidget;

    QGraphicsItem* mToc;
    QList<QGraphicsItem*> mSpots;
    QMap<QGraphicsItem*, GraphicsItemList> mHiddenItems;
    QString mMode;
    QString mPath;
    QString mPresentationTitle;
    QString mPresentationDescr;
    QString mPresentationDate;
    QString mAuthorName;
    QString mAuthorEmail;

};

#endif // ZIPPLXMLREADER_H
