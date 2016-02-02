#include "rubber_band.hpp"

#include <QDebug>
#include <QMouseEvent>
#include <QRect>
#include <QRubberBand>

namespace qte{

rubber_band::rubber_band(QWidget *parent) :
    QRubberBand(QRubberBand::Rectangle, parent),
    tweaking_part_{tweak_part::none}
{
    cursor_map_.emplace_back(Qt::SizeFDiagCursor);
    cursor_map_.emplace_back(Qt::SizeBDiagCursor);
    cursor_map_.emplace_back(Qt::SizeBDiagCursor);
    cursor_map_.emplace_back(Qt::SizeFDiagCursor);
    cursor_map_.emplace_back(Qt::SizeVerCursor);
    cursor_map_.emplace_back(Qt::SizeVerCursor);
    cursor_map_.emplace_back(Qt::SizeHorCursor);
    cursor_map_.emplace_back(Qt::SizeHorCursor);
}

void rubber_band::mousePressEvent(QMouseEvent *e)
{    
    QPoint const pt = e->pos();
    QRect const rg = geometry();
    if(rg.isValid()){
        tl_ = rg.topLeft();
        tr_ = rg.topRight();
        bl_ = rg.bottomLeft();
        br_ = rg.bottomRight();
        QPoint const off(10, 10), offx(10, -10), offy(-10, 10);

        if(QRect(tl_-off , tl_+off).contains(pt)){
            tweaking_part_ = tweak_part::top_left;
        }else if(QRect(tr_-off, tr_+off).contains(pt)){
            tweaking_part_ = tweak_part::top_right;
        }else if(QRect(bl_-off, bl_+off ).contains(pt)){
            tweaking_part_ = tweak_part::bottom_left;
        }else if(QRect(br_-off, br_+off).contains(pt)){
            tweaking_part_ = tweak_part::bottom_right;
        }else if(QRect(tl_+offx, tr_-offx).contains(pt)){
            tweaking_part_ = tweak_part::top;
        }else if(QRect(bl_+offx, br_-offx).contains(pt)){
            tweaking_part_ = tweak_part::bottom;
        }else if(QRect(tl_+offy, bl_-offy).contains(pt)){
            tweaking_part_ = tweak_part::left;
        }else if(QRect(tr_+offy, br_-offy).contains(pt)){
            tweaking_part_ = tweak_part::right;
        }

        auto const cursor = cursor_map_[static_cast<int>(tweaking_part_)];
        setCursor(cursor);
        emit cursor_changed(cursor);

        //qDebug()<<"set "<<tweaking_part_<<" cursor";

        if(tweaking_part_ != tweak_part::none){
            return;
        }
    }

    origin_ = pt ;
    setGeometry(QRect(origin_, QSize()));
    show();
}

void rubber_band::mouseMoveEvent(QMouseEvent *e)
{
    QPoint const pt = e->pos();
    if(tweaking_part_ != tweak_part::none){
        QRect rg = geometry();
        if(tweaking_part_ == tweak_part::top_left){
            rg.setTopLeft(pt);
        }else if(tweaking_part_ == tweak_part::top_right){
            rg.setTopRight(pt);
        }else if(tweaking_part_ == tweak_part::bottom_left){
            rg.setBottomLeft(pt);
        }else if(tweaking_part_ == tweak_part::bottom_right){
            rg.setBottomRight(pt);
        }else if(tweaking_part_ == tweak_part::top){
            rg.setTop(pt.y());
        }else if(tweaking_part_ == tweak_part::bottom){
            rg.setBottom(pt.y());
        }else if(tweaking_part_ == tweak_part::left){
            rg.setLeft(pt.x());
        }else if(tweaking_part_ == tweak_part::right){
            rg.setRight(pt.x());
        }
        if(rg.width() >= 50 && rg.height() >= 50){
            setGeometry(rg);
        }
    }else{
        setGeometry(QRect(origin_, pt).normalized());
    }
}

void rubber_band::mouseReleaseEvent(QMouseEvent*)
{    
    tweaking_part_ = tweak_part::none;
    unsetCursor();
    emit unset_cursor();
}

}
