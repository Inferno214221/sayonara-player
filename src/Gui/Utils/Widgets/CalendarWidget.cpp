#include "CalendarWidget.h"
#include "Gui/Utils/Style.h"

#include <QRect>
#include <QPainter>
#include <QDate>
#include <QPalette>

using Gui::CalendarWidget;

CalendarWidget::CalendarWidget(QWidget* parent) :
	QCalendarWidget(parent)
{}

CalendarWidget::~CalendarWidget() = default;

void CalendarWidget::paintCell(QPainter* painter, const QRect& rect, const QDate& date) const
{
	if(Style::isDark())
	{
		bool isInvalidDate = (date < this->minimumDate() || date > this->maximumDate());
		const QPalette p = this->palette();

		if(isInvalidDate)
		{
			QColor c = p.color(QPalette::ColorGroup::Disabled, QPalette::ColorRole::WindowText);

			painter->setPen(c);
			painter->drawText(rect, int(Qt::AlignHCenter | Qt::AlignVCenter), QString::number(date.day()));

			return;
		}
	}

	QCalendarWidget::paintCell(painter, rect, date);
}
