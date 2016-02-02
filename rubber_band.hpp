#ifndef RUBBER_BAND_H
#define RUBBER_BAND_H

#include <QRubberBand>

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

protected:
    void mousePressEvent(QMouseEvent * event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;

private:
    QPoint origin_;
    bool tweaking_;
    QString tweaking_part_;
    QPoint tl_, tr_, bl_, br_;
};

} /*! @} End of Doxygen Groups*/

#endif // RUBBER_BAND_H
