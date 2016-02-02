#include "img_region_selector.hpp"

#include <QDebug>
#include <QGraphicsEffect>
#include <QMouseEvent>
#include <QRubberBand>

#include <algorithm>

namespace qte{

img_region_selector::img_region_selector(QWidget *parent) :
    QLabel(parent),
    ctrl_key_press_{false},
    cur_rband{std::rend(rubber_band_)},
    delete_key_press_{false},
    move_rubber_band_{false},
    origin_(0, 0),
    rubber_band_offset_(0, 0),
    shift_key_press_{false}
{        
}

void img_region_selector::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Shift){
        shift_key_press_ = true;
    }
    if(e->key() == Qt::Key_Control){
        ctrl_key_press_ = true;
    }
    if(e->key() == Qt::Key_Delete){
        delete_key_press_ = true;
    }

    QLabel::keyPressEvent(e);
}

void img_region_selector::keyReleaseEvent(QKeyEvent *e)
{
    ctrl_key_press_ = false;
    shift_key_press_ = false;
    delete_key_press_ = false;

    QLabel::keyReleaseEvent(e);
}

img_region_selector::rband_iter
img_region_selector::select_rubber_band(QPoint const &pos)
{
    auto func = [=](QRubberBand *b)
    {
        return b->geometry().contains(pos);
    };
    return std::find_if(std::rbegin(rubber_band_),
                        std::rend(rubber_band_),
                        func);
}

void img_region_selector::set_resize_info()
{
    QPoint const pt = origin_;
    QRect const rg = (*cur_rband)->geometry();
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
            return;
        }
    }
}

void img_region_selector::create_rubber_band(QPoint const &pos)
{
    auto *rb = new QRubberBand(QRubberBand::Rectangle, this);
    rb->setGeometry(QRect(pos, QSize()));
    auto *g_effect = new QGraphicsColorizeEffect(this);
    rb->setGraphicsEffect(g_effect);
    g_effect->setColor(QColor("red"));
    rubber_band_.emplace_back(rb);
    rb->show();
    cur_rband = std::rbegin(rubber_band_);
}

bool img_region_selector::is_valid_ker_press() const
{
    return !(shift_key_press_ && ctrl_key_press_ &&
             delete_key_press_);
}

void img_region_selector::mousePressEvent(QMouseEvent *e)
{
    if(!is_valid_ker_press()){
        return;
    }

    bool const click_left_mouse = e->button() == Qt::LeftButton;
    if(click_left_mouse){
        origin_ = e->pos();
        cur_rband = select_rubber_band(origin_);

        if(shift_key_press_){
            if(cur_rband != std::rend(rubber_band_)){
                rubber_band_offset_ = e->pos() - (*cur_rband)->pos();
                move_rubber_band_ = true;
                change_cur_band_color(QColor("red"));
            }else{
                create_rubber_band(origin_);
            }
        }else if(delete_key_press_){
            if(cur_rband != std::rend(rubber_band_)){
                delete *cur_rband;
                rubber_band_.erase((cur_rband+1).base());
                cur_rband = std::rend(rubber_band_);
            }
        }else if(ctrl_key_press_){
            if(cur_rband != std::rend(rubber_band_)){
                set_resize_info();
                (*cur_rband)->setGeometry(QRect(origin_, QSize()));
            }
        }
    }

    QLabel::mousePressEvent(e);
}

void img_region_selector::mouseMoveEvent(QMouseEvent *e)
{
    if(!is_valid_ker_press()){
        return;
    }

    if(shift_key_press_){
        if(move_rubber_band_){
            (*cur_rband)->move(e->pos() - rubber_band_offset_);
        }else{
            rubber_band_.back()->setGeometry(QRect(origin_, e->pos()).normalized());
        }
    }else if(ctrl_key_press_){

    }

    QLabel::mouseMoveEvent(e);
}

void img_region_selector::mouseReleaseEvent(QMouseEvent *e)
{
    if(!is_valid_ker_press()){
        return;
    }

    move_rubber_band_ = false;
    change_cur_band_color(QColor("blue"));
    QLabel::mouseReleaseEvent(e);
}

void img_region_selector::change_cur_band_color(const QColor &color)
{
    if(cur_rband != std::rend(rubber_band_)){
        auto *g_effect =
                qobject_cast<QGraphicsColorizeEffect*>((*cur_rband)->graphicsEffect());
        if(g_effect){
            g_effect->setColor(color);
        }
    }
}

}
