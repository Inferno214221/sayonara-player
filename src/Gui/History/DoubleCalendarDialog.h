/* DoubleCalendarDialog.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
