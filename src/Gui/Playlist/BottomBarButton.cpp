#include "BottomBarButton.h"
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>

void BottomBarButton::set_pixmap(const QPixmap& pm)
{
	m_pixmap = pm;
}

BottomBarButton::BottomBarButton(const QPixmap& pm, QWidget* parent) :
	QPushButton(parent),
	m_pixmap(pm)
{}

BottomBarButton::~BottomBarButton() = default;


void BottomBarButton::paintEvent(QPaintEvent* e)
{
	if(!this->isChecked())
	{
		QPushButton::paintEvent(e);
	}

    if (!m_pixmap.isNull())
    {
		const int w = this->width();
		const int h = this->height();

		int pm_w = (w * 800) / 1000;
		int pm_h = (h * 800) / 1000;

		const int x = (w - pm_w) / 2;
		const int y = (h - pm_h) / 2;

		if((w - pm_w) % 2 == 1){
			pm_w++;
		}

		if((h - pm_h) % 2 == 1){
			pm_h++;
		}

		QPixmap pm = m_pixmap.scaled(pm_w, pm_h, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QPainter painter(this);
        if(this->isChecked())
        {
			QRect r = e->rect();
			//r.setY(y + pm_h);
			painter.setPen(palette().background().color());
			painter.drawRect(r);

			QColor c = palette().highlight().color();
			painter.setOpacity(0.3);
			painter.setPen(c);
			painter.setBrush(c);
			painter.drawRect(r);
			painter.fillRect(r, c);
        }

		painter.setOpacity(1.0);
        painter.drawPixmap
        (
			QRect(x, y, pm_w, pm_h),
			pm
        );
    }
}
