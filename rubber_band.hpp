#ifndef RUBBER_BAND_H
#define RUBBER_BAND_H

#include <QRubberBand>

#include <map>

/*!
 *  \addtogroup qte
 *  @{
 */
namespace qte{

class rubber_band : public QRubberBand
{
    Q_OBJECT
public:
    explicit rubber_band(QWidget *parent = nullptr);

public:
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;

private:
    enum class tweak_part
    {
        top_left,
        top_right,
        bottom_left,
        bottom_right,
        top,
        bottom,
        left,
        right,
        none
    };

    std::vector<Qt::CursorShape> cursor_map_;
    QPoint origin_;
    tweak_part tweaking_part_;
    QPoint tl_, tr_, bl_, br_;
};

} /*! @} End of Doxygen Groups*/

#endif // RUBBER_BAND_H
