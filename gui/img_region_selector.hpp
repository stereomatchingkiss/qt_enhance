#ifndef IMG_REGION_SELECTOR_HPP
#define IMG_REGION_SELECTOR_HPP

#include "rubber_band.hpp"

#include <QGraphicsView>
#include <QLabel>

#include <vector>

class QGraphicsPixmapItem;
class QGraphicsScene;

/*!
 *  \addtogroup qte
 *  @{
 */
namespace qte{

class img_region_selector : public QGraphicsView
{
    Q_OBJECT
public:
    explicit img_region_selector(QWidget *parent = nullptr);

    /**
     * Remove all of the rubber band
     */
    void clear_rubber_band();

    std::vector<QRect> selected_regions() const;
    void selected_regions(std::vector<QRect> &inout) const;

    void set_enable_focus(bool value);
    /**
     * This api will resize the rubber band if current
     * pixmap size do not equal to the old one
     * @param pix current pixmap
     */
    void set_pixmap(QPixmap const &pix);

protected:    
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private slots:
    void cursor_changed(Qt::CursorShape shape);
    void unset_cursor();

private:
    using rband_iter = std::vector<rubber_band*>::reverse_iterator;

    void change_cur_band_color(QColor const &color);
    void create_rubber_band(QPoint const &pos);

    bool is_valid_ker_press() const;

    QPoint map_to_pixmap_point(QPoint const &pt) const;
    QRect map_to_pixmap_rect(QRect const &rect) const;

    void scale_rubber_band(const QPixmap &pix);
    rband_iter select_rubber_band(QPoint const &pos);

    bool within_pixmap(QPoint const &pt) const;
    bool within_pixmap(QRect const &pt) const;

    bool ctrl_key_press_; //resize rubber band
    rband_iter cur_rband;
    bool delete_key_press_; //delete rubber band
    bool enable_focus_;
    QGraphicsPixmapItem *graph_pixmap_;
    QGraphicsScene      *graph_scene_;
    bool move_rubber_band_;
    QPoint origin_;
    std::vector<rubber_band*> rubber_band_;
    QPoint rubber_band_offset_;
    //move rubber band or create new rubber band
    bool shift_key_press_;
};

} /*! @} End of Doxygen Groups*/

#endif // IMG_REGION_SELECTOR_HPP
