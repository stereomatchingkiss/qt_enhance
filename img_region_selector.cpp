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

    QLabel::keyPressEvent(e);
}

void img_region_selector::keyReleaseEvent(QKeyEvent *e)
{
    ctrl_key_press_ = false;
    shift_key_press_ = false;

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

void img_region_selector::mousePressEvent(QMouseEvent *e)
{
    if(shift_key_press_ && ctrl_key_press_){
        return;
    }

    bool const click_left_mouse = e->button() == Qt::LeftButton;
    if(click_left_mouse && shift_key_press_){
        origin_ = e->pos();
        cur_rband = select_rubber_band(origin_);

        if(cur_rband != std::rend(rubber_band_)){
            rubber_band_offset_ = e->pos() - (*cur_rband)->pos();
            move_rubber_band_ = true;
            change_cur_band_color(QColor("red"));
        }else{
            create_rubber_band(origin_);
        }
    }

    QLabel::mousePressEvent(e);
}

void img_region_selector::mouseMoveEvent(QMouseEvent *e)
{
    if(shift_key_press_ && ctrl_key_press_){
        return;
    }

    if(shift_key_press_){
        if(move_rubber_band_){
            (*cur_rband)->move(e->pos() - rubber_band_offset_);
        }else{
            rubber_band_.back()->setGeometry(QRect(origin_, e->pos()).normalized());
        }
    }

    QLabel::mouseMoveEvent(e);
}

void img_region_selector::mouseReleaseEvent(QMouseEvent *e)
{
    move_rubber_band_ = false;
    change_cur_band_color(QColor("blue"));
    QLabel::mouseReleaseEvent(e);
}

void img_region_selector::change_cur_band_color(const QColor &color)
{
    auto *g_effect =
            qobject_cast<QGraphicsColorizeEffect*>((*cur_rband)->graphicsEffect());
    if(g_effect){
        g_effect->setColor(color);
    }
}

}
