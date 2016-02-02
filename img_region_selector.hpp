#ifndef IMG_REGION_SELECTOR_HPP
#define IMG_REGION_SELECTOR_HPP

#include <QLabel>

#include <vector>

class QRubberBand;

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

protected:    
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    using rband_iter = std::vector<QRubberBand*>::reverse_iterator;

    void change_cur_band_color(QColor const &color);
    void create_rubber_band(QPoint const &pos);

    bool is_valid_ker_press() const;

    rband_iter select_rubber_band(QPoint const &pos);
    void set_resize_info();

    bool ctrl_key_press_; //resize rubber band
    rband_iter cur_rband;
    bool delete_key_press_; //delete rubber band
    bool move_rubber_band_;
    QPoint origin_;
    std::vector<QRubberBand*> rubber_band_;
    QPoint rubber_band_offset_;
    //move rubber band or create new rubber band
    bool shift_key_press_;

    QPoint tl_, tr_, bl_, br_;
    QString tweaking_part_;
};

} /*! @} End of Doxygen Groups*/

#endif // IMG_REGION_SELECTOR_HPP
