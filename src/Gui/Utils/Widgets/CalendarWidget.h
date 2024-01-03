/* CalendarWidget.h
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
