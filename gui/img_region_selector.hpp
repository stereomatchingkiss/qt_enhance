#ifndef IMG_REGION_SELECTOR_HPP
#define IMG_REGION_SELECTOR_HPP

#include "rubber_band.hpp"

#include <QLabel>

#include <vector>

/*!
 *  \addtogroup qte
 *  @{
 */
namespace qte{

class img_region_selector : public QLabel
{
    Q_OBJECT
public:
    explicit img_region_selector(QWidget *parent = nullptr);

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

    rband_iter select_rubber_band(QPoint const &pos);

    bool ctrl_key_press_; //resize rubber band
    rband_iter cur_rband;
    bool delete_key_press_; //delete rubber band
    bool move_rubber_band_;
    QPoint origin_;
    std::vector<rubber_band*> rubber_band_;
    QPoint rubber_band_offset_;
    //move rubber band or create new rubber band
    bool shift_key_press_;  
};

} /*! @} End of Doxygen Groups*/

#endif // IMG_REGION_SELECTOR_HPP