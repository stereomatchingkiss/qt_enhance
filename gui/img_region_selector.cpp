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

void img_region_selector::clear_rubber_band()
{
    for(auto *rb : rubber_band_){
        delete rb;
    }
    rubber_band_.clear();
}

void img_region_selector::set_pixmap(const QPixmap &pix)
{
    if(pixmap()){
        auto const pre_size = pixmap()->size();
        auto const cur_size = pix.size();
        if(pre_size != pix.size()){
            for(auto it = std::begin(rubber_band_);
                it != std::end(rubber_band_);){
                float const width_ratio = cur_size.width() / static_cast<float>(pre_size.width());
                float const height_ratio = cur_size.height() / static_cast<float>(pre_size.height());
                int const width = static_cast<int>(pre_size.width() * width_ratio);
                int const height = static_cast<int>(pre_size.height() * height_ratio);
                if(width >= 50 && height >= 50){
                    (*it)->resize(width, height);
                    ++it;
                }else{
                    delete *it;
                    it = rubber_band_.erase(it);
                }
            }
        }
        setPixmap(pix);
    }
}

void img_region_selector::keyPressEvent(QKeyEvent *e)
{
    //qDebug()<<__func__;
    if(e->key() == Qt::Key_Shift){
        shift_key_press_ = true;
    }else if(e->key() == Qt::Key_Control){
        ctrl_key_press_ = true;
    }else if(e->key() == Qt::Key_Delete){
        delete_key_press_ = true;
    }else{
        QLabel::keyPressEvent(e);
    }
}

void img_region_selector::keyReleaseEvent(QKeyEvent*)
{    
    ctrl_key_press_ = false;
    shift_key_press_ = false;
    delete_key_press_ = false;
}

img_region_selector::rband_iter
img_region_selector::select_rubber_band(QPoint const &pos)
{
    auto func = [=](rubber_band *b)
    {
        return b->geometry().contains(pos);
    };
    return std::find_if(std::rbegin(rubber_band_),
                        std::rend(rubber_band_),
                        func);
}

void img_region_selector::create_rubber_band(QPoint const &pos)
{
    auto *rb = new rubber_band(this);
    rb->setGeometry(QRect(pos, QSize()));
    auto *g_effect = new QGraphicsColorizeEffect(this);
    rb->setGraphicsEffect(g_effect);
    g_effect->setColor(QColor("red"));
    rubber_band_.emplace_back(rb);
    rb->show();
    connect(rb, SIGNAL(cursor_changed(Qt::CursorShape)),
            this, SLOT(cursor_changed(Qt::CursorShape)));
    connect(rb, SIGNAL(unset_cursor()),
            this, SLOT(unset_cursor()));
    cur_rband = std::rbegin(rubber_band_);
}

bool img_region_selector::is_valid_ker_press() const
{
    return !(shift_key_press_ && ctrl_key_press_ &&
             delete_key_press_);
}

void img_region_selector::mousePressEvent(QMouseEvent *e)
{
    //qDebug()<<"mouse press";
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
                (*cur_rband)->mousePressEvent(e);
            }
        }
    }

    //QLabel::mousePressEvent(e);
}

void img_region_selector::mouseMoveEvent(QMouseEvent *e)
{
    //qDebug()<<__func__;
    if(!is_valid_ker_press()){
        return;
    }

    if(cur_rband != std::rend(rubber_band_)){
        if(shift_key_press_){
            if(move_rubber_band_){
                (*cur_rband)->move(e->pos() - rubber_band_offset_);
            }else{
                (*cur_rband)->setGeometry(QRect(origin_, e->pos()).normalized());
            }
        }else if(ctrl_key_press_){
            (*cur_rband)->mouseMoveEvent(e);
        }
    }

    //QLabel::mouseMoveEvent(e);
}

void img_region_selector::mouseReleaseEvent(QMouseEvent *e)
{
    //qDebug()<<__func__;
    if(!is_valid_ker_press()){
        return;
    }

    move_rubber_band_ = false;
    if(cur_rband != std::rend(rubber_band_)){
        (*cur_rband)->mouseReleaseEvent(e);
        change_cur_band_color(QColor("blue"));
        cur_rband = std::rend(rubber_band_);
    }
    //QLabel::mouseReleaseEvent(e);
}

void img_region_selector::cursor_changed(Qt::CursorShape shape)
{
    setCursor(shape);
}

void img_region_selector::unset_cursor()
{
    unsetCursor();
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
