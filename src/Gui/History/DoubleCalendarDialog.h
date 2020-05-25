#ifndef DOUBLECALENDARDIALOG_H
#define DOUBLECALENDARDIALOG_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

class QDate;

namespace Gui
{
	class CalendarWidget;

	class DoubleCalendarDialog :
		public Gui::Dialog
	{
		Q_OBJECT
		PIMPL(DoubleCalendarDialog)

	signals:
		void sigAccepted();
		void sigRejected();

	public:
		explicit DoubleCalendarDialog(QWidget* parent=nullptr);
		~DoubleCalendarDialog() override;

        QDate startDate() const;
		QDate endDate() const;

	private slots:
		void startDateSelected(const QDate& date);
		void endDateSelected(const QDate& date);
	};
}

#endif // DOUBLECALENDARDIALOG_H
