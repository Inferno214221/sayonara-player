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

#include "LibraryContainerImpl.h"
#include "LibraryPluginCombobox.h"

#include <QWidget>
#include <QAction>
#include <QLayout>

using Library::ContainerImpl;

struct ContainerImpl::Private
{
	bool		initialized;

	Private() :
		initialized(false)
	{}
};

ContainerImpl::ContainerImpl(QObject* parent) :
	QObject(parent),
	Library::Container()
{
	m = Pimpl::make<Private>();
}

void ContainerImpl::rename(const QString& new_name)
{
	Q_UNUSED(new_name);
}

ContainerImpl::~ContainerImpl() = default;

QString ContainerImpl::displayName() const
{
	return name();
}

QMenu* ContainerImpl::menu()
{
	return nullptr;
}

bool ContainerImpl::isLocal() const
{
	return false;
}

void ContainerImpl::init()
{
	if(m->initialized){
		return;
	}

	this->initUi();

	QWidget* ui = this->widget();
	QLayout* layout = ui->layout();
	if(layout) {
		layout->setContentsMargins(5, 0, 8, 0);
	}

	QFrame* header_frame = this->header();
	if(header_frame)
	{
		auto* layout = new QVBoxLayout(header_frame);
		layout->setContentsMargins(0, 0, 0, 0);

		auto* combo_box = new Library::PluginCombobox(this->displayName(), header_frame);
		layout->addWidget(combo_box);

		header_frame->setFrameShape(QFrame::NoFrame);
		header_frame->setLayout(layout);
	}

	m->initialized = true;
}



