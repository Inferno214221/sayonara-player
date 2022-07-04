/* InputField.cpp */
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

#include "InputField.h"
#include "TimeSpanChooser.h"

#include "Components/SmartPlaylists/SmartPlaylist.h"
#include "Components/SmartPlaylists/TimeSpan.h"
#include "Components/SmartPlaylists/DateConverter.h"
#include "Components/SmartPlaylists/TimeSpanConverter.h"
#include "Gui/Utils/EventFilter.h"
#include "Gui/Utils/Widgets/CalendarWidget.h"

#include <QMouseEvent>

struct InputField::Private
{
	SmartPlaylists::InputFormat inputFormat {SmartPlaylists::InputFormat::Text};
	std::shared_ptr<SmartPlaylists::StringConverter> converter;
};

InputField::InputField(QWidget* parent) :
	QLineEdit(parent),
	m {Pimpl::make<Private>()}
{
	auto* eventFilter = new Gui::MousePressedFilter(this);
	connect(eventFilter, &Gui::MousePressedFilter::sigMousePressed, this, &InputField::mousePressed);
	installEventFilter(eventFilter);
}

InputField::~InputField() = default;

void InputField::mousePressed(QMouseEvent* /*e*/)
{
	const auto data = property("internal").toInt();

	if(m->inputFormat == SmartPlaylists::InputFormat::TimeSpan)
	{
		const auto timeSpan = SmartPlaylists::timeSpanFromDays(data);

		auto* timeSpanChooser = new TimeSpanChooser(timeSpan, this);
		connect(timeSpanChooser, &QDialog::accepted, this, &InputField::timeSpanAccepted);
		connect(timeSpanChooser, &QDialog::rejected, timeSpanChooser, &QObject::deleteLater);
		timeSpanChooser->show();
	}

	else if(m->inputFormat == SmartPlaylists::InputFormat::Calendar)
	{
		const auto date = SmartPlaylists::intToDate(data);
		auto* calendarWidget = new Gui::CalendarWidget(this);

		calendarWidget->setWindowFlag(Qt::WindowType::Dialog, true);
		calendarWidget->setMaximumDate(QDate::currentDate());
		calendarWidget->setCurrentPage(date.year(), date.month());

		connect(calendarWidget, &QCalendarWidget::clicked, this, &InputField::calendarAccepted);
		calendarWidget->show();
	}
}

void InputField::timeSpanAccepted()
{
	auto* timeSpanChooser = dynamic_cast<TimeSpanChooser*>(sender());

	setData(SmartPlaylists::InputFormat::TimeSpan, m->converter, timeSpanChooser->days());

	timeSpanChooser->deleteLater();
}

void InputField::calendarAccepted(const QDate& date)
{
	if(date.isValid())
	{
		setData(SmartPlaylists::InputFormat::Calendar, m->converter, SmartPlaylists::dateToInt(date));
	}

	sender()->deleteLater();
}

void InputField::setData(SmartPlaylists::InputFormat inputFormat,
                         const std::shared_ptr<SmartPlaylists::StringConverter>& converter, const int value)
{
	m->inputFormat = inputFormat;
	m->converter = converter;

	setReadOnly((m->inputFormat == SmartPlaylists::InputFormat::TimeSpan) ||
	            (m->inputFormat == SmartPlaylists::InputFormat::Calendar));

	setProperty("internal", value);
	setText(converter->intToUserString(value));
}

std::optional<int> InputField::data() const
{
	return (isReadOnly())
	       ? property("internal").toInt()
	       : m->converter->stringToInt(text());
}