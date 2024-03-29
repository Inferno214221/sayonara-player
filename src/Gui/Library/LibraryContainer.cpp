/* LibraryContainer.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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
#include "Components/LibraryManagement/LibraryPluginHandler.h"

#include <QWidget>
#include <QAction>
#include <QLayout>

namespace Gui::Library
{
	struct Container::Private
	{
		bool initialized {false};
		::Library::PluginHandler* pluginHandler;

		explicit Private(::Library::PluginHandler* pluginHandler) :
			pluginHandler {pluginHandler} {}
	};

	Container::Container(::Library::PluginHandler* pluginHandler) :
		m {Pimpl::make<Private>(pluginHandler)} {}

	Container::~Container() = default;

	void Container::init()
	{
		if(m->initialized)
		{
			return;
		}

		initUi();

		auto* ui = widget();
		auto* layout = ui->layout();
		if(layout)
		{
			layout->setContentsMargins(5, 0, 8, 0); // NOLINT(readability-magic-numbers)
		}

		auto* headerFrame = header();
		if(headerFrame)
		{
			auto* vBoxLayout = new QVBoxLayout(headerFrame);
			vBoxLayout->setContentsMargins(0, 0, 0, 0);

			auto* comboBox = new ::Library::PluginCombobox(m->pluginHandler, displayName(), headerFrame);
			vBoxLayout->addWidget(comboBox);

			headerFrame->setFrameShape(QFrame::NoFrame);
			headerFrame->setLayout(vBoxLayout);
		}

		m->initialized = true;
	}
}