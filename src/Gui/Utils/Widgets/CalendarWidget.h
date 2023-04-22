#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include "Utils/Pimpl.h"

#include <QCalendarWidget>
#include <QDialog>

class QDate;

namespace Gui
{
	class CalendarWidget :
		public QCalendarWidget
	{
		Q_OBJECT

		public:
			explicit CalendarWidget(QWidget* parent = nullptr);
			~CalendarWidget() override;

		protected:
			void paintCell(QPainter* painter, const QRect& rect, const QDate& date) const override;
	};

	class CalendarDialog :
		public QDialog
	{
		Q_OBJECT
		PIMPL(CalendarDialog)

		public:
			CalendarDialog(const QString& title, const QString& text, QWidget* parent = nullptr);
			~CalendarDialog() noexcept override;

			[[nodiscard]] QDate selectedDate() const;
	};
}

#endif // CALENDARWIDGET_H
