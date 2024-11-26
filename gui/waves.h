#pragma once
#include <QGraphicsObject>
#include <QWidget>

class Waves: public QGraphicsObject {

  public: 
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;


    int test;

} ;
