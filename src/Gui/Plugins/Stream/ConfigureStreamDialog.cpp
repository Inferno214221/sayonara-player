/* ConfigureStreamDialog.cpp
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

#include "ConfigureStreamDialog.h"
#include "Components/Streaming/Streams/StreamHandler.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Streams/Station.h"

#include <QLineEdit>
#include <QCheckBox>

struct ConfigureStreamDialog::Private
{
	QLineEdit* name {new QLineEdit()};
	QLineEdit* url {new QLineEdit()};
	QCheckBox* useSpecialUserAgent {new QCheckBox()};
	QLineEdit* userAgent {new QLineEdit()};
	QCheckBox* updateMetadata {new QCheckBox()};

	Private()
	{
		updateMetadata->setChecked(GetSetting(Set::Stream_UpdateMetadata));
	}
};

ConfigureStreamDialog::ConfigureStreamDialog(QWidget* parent) :
	GUI_ConfigureStation(parent),
	m {Pimpl::make<Private>()}
{
	connect(m->useSpecialUserAgent, &QCheckBox::toggled, m->userAgent, &QWidget::setVisible);
}

ConfigureStreamDialog::~ConfigureStreamDialog() = default;

StationPtr ConfigureStreamDialog::configuredStation()
{
	return std::make_shared<Stream>(
		m->name->text(),
		m->url->text(),
		m->updateMetadata->isChecked(),
		m->useSpecialUserAgent->isChecked() ? m->userAgent->text() : QString());
}

void ConfigureStreamDialog::configureWidgets(StationPtr station)
{
	auto stream = std::dynamic_pointer_cast<Stream>(station);
	if(stream)
	{
		m->name->setText(stream->name());
		m->url->setText(stream->url());
		m->userAgent->setText(stream->userAgent());
		m->updateMetadata->setChecked(stream->isUpdatable());
	}

	else
	{
		m->name->setText({});
		m->url->setText({});
		m->userAgent->setText({});
		m->updateMetadata->setChecked(GetSetting(Set::Stream_UpdateMetadata));
	}

	const auto hasCustomUserAgent = !m->userAgent->text().isEmpty();
	m->useSpecialUserAgent->setChecked(hasCustomUserAgent);
	m->userAgent->setVisible(hasCustomUserAgent);
}

QList<QWidget*> ConfigureStreamDialog::configurationWidgets()
{
	return {m->name, m->url, m->updateMetadata, m->useSpecialUserAgent, m->userAgent};
}

QString ConfigureStreamDialog::labelText(const int index) const
{
	switch(index)
	{
		case 0:
			return Lang::get(Lang::Name);
		case 1:
			return "Url";
		case 2:
			return tr("Update Metadata");
		case 3:
			return "User-Agent";
		case 4:
		default:
			return {};
	}
}
