#include "rubber_band.hpp"

#include <QMouseEvent>
#include <QRubberBand>

namespace qte{

rubber_band::rubber_band(QWidget *parent) :
    QRubberBand(QRubberBand::Rectangle, parent),
    tweaking_{false}
{

}

void rubber_band::mousePressEvent(QMouseEvent *e)
{
    QPoint pt = e->pos() ;
    QRect rg = geometry();

    if(rg.isValid()){
        tl_ = rg.topLeft();
        tr_ = rg.topRight();
        bl_ = rg.bottomLeft();
        br_ = rg.bottomRight();
        QPoint const off(3, 3), offx(4, -3), offy(-3, 4);

        if(QRect(tl_-off , tl_+off).contains(pt)){
            tweaking_part_ = "topLeft"; setCursor(Qt::SizeFDiagCursor);
        }else if(QRect(tr_-off, tr_+off).contains(pt)){
            tweaking_part_ = "topRight"; setCursor(Qt::SizeBDiagCursor);
        }else if(QRect(bl_-off, bl_+off ).contains(pt)){
            tweaking_part_ = "bottomLeft"; setCursor(Qt::SizeBDiagCursor);
        }else if(QRect(br_-off, br_+off).contains(pt)){
            tweaking_part_ = "bottomRight"; setCursor(Qt::SizeFDiagCursor);
        }else if(QRect(tl_+offx, tr_-offx).contains(pt)){
            tweaking_part_ = "top"; setCursor(Qt::SizeVerCursor);
        }else if(QRect(bl_+offx, br_-offx).contains(pt)){
            tweaking_part_ = "bottom"; setCursor(Qt::SizeVerCursor);
        }else if(QRect(tl_+offy, bl_-offy).contains(pt)){
            tweaking_part_ = "left"; setCursor(Qt::SizeHorCursor);
        }else if(QRect(tr_+offy, br_-offy).contains ( pt ) ){
            tweaking_part_ = "right"; setCursor(Qt::SizeHorCursor);
        }

        if(!tweaking_part_.isEmpty()){
            tweaking_ = true;
            return;
        }
    }

    origin_ = pt ;
    setGeometry(QRect(origin_, QSize()));
    show() ;
}

void rubber_band::mouseMoveEvent(QMouseEvent *e)
{
    QPoint pt = e->pos();
    if(tweaking_){
        QRect rg = geometry();
        if      (tweaking_part_ == "topLeft"     ) rg.setTopLeft(pt);
        else if (tweaking_part_ == "topRight"    ) rg.setTopRight(pt);
        else if (tweaking_part_ == "bottomLeft"  ) rg.setBottomLef(pt);
        else if (tweaking_part_ == "bottomRight" ) rg.setBottomRight(pt);
        else if (tweaking_part_ == "top"         ) rg.setTop(pt.y());
        else if (tweaking_part_ == "bottom"      ) rg.setBottom(pt.y());
        else if (tweaking_part_ == "left"        ) rg.setLeft(pt.x());
        else if (tweaking_part_ == "right"       ) rg.setRight(pt.x());
        setGeometry(rg) ;
    }else{
        setGeometry(QRect(origin_, pt).normalized());
    }
}

void rubber_band::mouseReleaseEvent(QMouseEvent*)
{
    tweaking_ = false ;
    tweaking_part_ = "" ;
    unsetCursor();
}

}
