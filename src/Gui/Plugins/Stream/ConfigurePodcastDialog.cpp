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
	QCheckBox* reverse {new QCheckBox()};
};

ConfigurePodcastDialog::ConfigurePodcastDialog(QWidget* parent) :
	GUI_ConfigureStation(parent),
	m {Pimpl::make<Private>()} {}

ConfigurePodcastDialog::~ConfigurePodcastDialog() = default;

StationPtr ConfigurePodcastDialog::configuredStation()
{
	return std::make_shared<Podcast>(m->name->text(), m->url->text(), m->reverse->isChecked());
}

QList<QWidget*> ConfigurePodcastDialog::configurationWidgets()
{
	return {m->name, m->url, m->reverse};
}

void ConfigurePodcastDialog::configureWidgets(StationPtr station)
{
	const auto podcast = std::dynamic_pointer_cast<Podcast>(station);
	if(podcast)
	{
		m->name->setText(podcast->name());
		m->url->setText(podcast->url());
		m->reverse->setChecked(podcast->reversed());
	}

	else
	{
		m->name->setText(QString());
		m->url->setText(QString());
		m->reverse->setChecked(false);
	}
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
		default:
			return QString {};
	}
}
