/* LibraryContainer.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "LibraryContainer.h"
#include "Gui/Library/Utils/LibraryPluginCombobox.h"

#include <QWidget>
#include <QAction>
#include <QLayout>

using Library::Container;

struct Container::Private
{
	bool initialized;

	Private() :
		initialized(false) {}
};

Container::Container(QObject* parent) :
	QObject(parent),
	Library::AbstractContainer()
{
	m = Pimpl::make<Private>();
}

void Container::rename(const QString& newName)
{
	Q_UNUSED(newName)
}

Container::~Container() = default;

QString Container::displayName() const
{
	return name();
}

QMenu* Container::menu()
{
	return nullptr;
}

bool Container::isLocal() const
{
	return false;
}

void Container::init()
{
	if(m->initialized)
	{
		return;
	}

	this->initUi();

	QWidget* ui = this->widget();
	QLayout* layout = ui->layout();
	if(layout)
	{
		layout->setContentsMargins(5, 0, 8, 0);
	}

	QFrame* headerFrame = this->header();
	if(headerFrame)
	{
		auto* vBoxLayout = new QVBoxLayout(headerFrame);
		vBoxLayout->setContentsMargins(0, 0, 0, 0);

		auto* comboBox = new Library::PluginCombobox(this->displayName(), headerFrame);
		vBoxLayout->addWidget(comboBox);

		headerFrame->setFrameShape(QFrame::NoFrame);
		headerFrame->setLayout(vBoxLayout);
	}

	m->initialized = true;
}
