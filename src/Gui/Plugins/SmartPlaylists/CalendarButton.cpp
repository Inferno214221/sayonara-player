/* CalendarButton.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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
#include "CalendarButton.h"

#include "Components/SmartPlaylists/SmartPlaylistByCreateDate.h"
#include "Components/SmartPlaylists/DateConverter.h"

#include "Gui/Utils/Widgets/CalendarWidget.h"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QDialog>

struct CalendarButton::Private
{
	QLineEdit* lineEdit;
	QDialog* dialog;
	Gui::CalendarWidget* calendarWidget;

	explicit Private(QLineEdit* lineEdit) :
		lineEdit {lineEdit},
		dialog(new QDialog(lineEdit)),
		calendarWidget(new Gui::CalendarWidget)
	{
		auto* layout = new QHBoxLayout();
		layout->addWidget(calendarWidget);
		dialog->setLayout(layout);
	}
};

CalendarButton::CalendarButton(QLineEdit* lineEdit, QWidget* parent) :
	QPushButton(parent),
	m {Pimpl::make<Private>(lineEdit)}
{
	setText(QString::fromUtf8("≡"));

	connect(this, &QPushButton::clicked, m->dialog, &QWidget::show);
	connect(m->calendarWidget, &QCalendarWidget::clicked, this, &CalendarButton::calendarActivated);
}

CalendarButton::~CalendarButton() noexcept = default;

void CalendarButton::calendarActivated(const QDate& date)
{
	const auto day = date.day();
	const auto month = date.month();
	const auto year = date.year();

	const auto value = year * 10000 + month * 100 + day;
	m->lineEdit->setText(SmartPlaylists::DateConverter().intToString(value));
	m->dialog->close();
}
