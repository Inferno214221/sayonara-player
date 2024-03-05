/* ConfigurePodcastDialog.cpp
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

#include "ConfigurePodcastDialog.h"
#include "Utils/Language/Language.h"
#include "Components/Streaming/Streams/PodcastHandler.h"

#include <QLineEdit>
#include <QCheckBox>

struct ConfigurePodcastDialog::Private
{
	QLineEdit* name {new QLineEdit()};
	QLineEdit* url {new QLineEdit()};
	QCheckBox* useSpecialUserAgent {new QCheckBox()};
	QLineEdit* userAgent {new QLineEdit()};
	QCheckBox* reverse {new QCheckBox()};
};

ConfigurePodcastDialog::ConfigurePodcastDialog(QWidget* parent) :
	GUI_ConfigureStation(parent),
	m {Pimpl::make<Private>()}
{
	connect(m->useSpecialUserAgent, &QCheckBox::toggled, m->userAgent, &QWidget::setVisible);
}

ConfigurePodcastDialog::~ConfigurePodcastDialog() = default;

StationPtr ConfigurePodcastDialog::configuredStation()
{
	return std::make_shared<Podcast>(
		m->name->text(),
		m->url->text(),
		m->reverse->isChecked(),
		m->useSpecialUserAgent->isChecked() ? m->userAgent->text() : QString());
}

QList<QWidget*> ConfigurePodcastDialog::configurationWidgets()
{
	return {m->name, m->url, m->reverse, m->useSpecialUserAgent, m->userAgent};
}

void ConfigurePodcastDialog::configureWidgets(StationPtr station)
{
	const auto podcast = std::dynamic_pointer_cast<Podcast>(station);
	if(podcast)
	{
		m->name->setText(podcast->name());
		m->url->setText(podcast->url());
		m->userAgent->setText(podcast->userAgent());
		m->reverse->setChecked(podcast->reversed());
	}

	else
	{
		m->name->setText(QString());
		m->url->setText(QString());
		m->userAgent->setText({});
		m->reverse->setChecked(false);
	}

	const auto hasCustomUserAgent = !m->userAgent->text().isEmpty();
	m->useSpecialUserAgent->setChecked(hasCustomUserAgent);
	m->userAgent->setVisible(hasCustomUserAgent);
}

QString ConfigurePodcastDialog::labelText(const int i) const
{
	switch(i)
	{
		case 0:
			return Lang::get(Lang::Name);
		case 1:
			return "Url";
		case 2:
			return Lang::get(Lang::ReverseOrder);
		case 3:
			return "User-Agent";
		case 4:
		default:
			return {};
	}
}
