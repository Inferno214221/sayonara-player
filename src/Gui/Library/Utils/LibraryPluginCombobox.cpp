/* LibraryPluginCombobox.cpp */

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

#include "LibraryPluginCombobox.h"
#include "LibraryPluginComboBoxDelegate.h"

#include "Components/LibraryManagement/LibraryContainer.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"

#include <QList>
#include <QAction>
#include <QFontMetrics>

namespace
{
	QString getElidedText(const Library::LibraryContainer* container, const QFontMetrics& fontMetrics)
	{
		return fontMetrics.elidedText(container->displayName(), Qt::TextElideMode::ElideRight, 200);
	}
}

namespace Library
{
	struct PluginCombobox::Private
	{
		QList<QAction*> actions;
		PluginHandler* pluginHandler {PluginHandler::instance()};
	};

	PluginCombobox::PluginCombobox(const QString& text, QWidget* parent) :
		ComboBox(parent)
	{
		m = Pimpl::make<Private>();

		this->setSizeAdjustPolicy(QComboBox::AdjustToContents);
		this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		this->setFocusPolicy(Qt::ClickFocus);
		this->setItemDelegate(new PluginComboBoxDelegate(this));

		connect(m->pluginHandler, &PluginHandler::sigLibrariesChanged, this, &PluginCombobox::setupActions);
		connect(m->pluginHandler,
		        &PluginHandler::sigCurrentLibraryChanged,
		        this,
		        &PluginCombobox::currentLibraryChanged);

		connect(this, combo_activated_int, this, &PluginCombobox::currentIndexChanged);

		setupActions();
		setCurrentText(text);
	}

	PluginCombobox::~PluginCombobox() = default;

	void PluginCombobox::setupActions()
	{
		this->clear();

		const auto containers = m->pluginHandler->libraries(true);
		for(const auto* container: containers)
		{
			const auto icon = container->icon();
			const auto displayName = getElidedText(container, this->fontMetrics());

			this->addItem(icon, displayName, container->name());
		}

		this->insertSeparator(1);
		this->setItemIcon(1, QIcon());

		currentLibraryChanged();
	}

	void PluginCombobox::currentLibraryChanged()
	{
		auto* currentLibrary = m->pluginHandler->currentLibrary();
		if(!currentLibrary)
		{
			return;
		}

		const auto name = currentLibrary->name();

		for(auto i = 0; i < this->count(); i++)
		{
			if(this->itemData(i).toString() == name)
			{
				if(i != this->currentIndex())
				{
					this->setCurrentIndex(i);
				}

				break;
			}
		}
	}

	void PluginCombobox::currentIndexChanged(int index)
	{
		m->pluginHandler->setCurrentLibrary(index - 2);
	}

	void PluginCombobox::languageChanged()
	{
		if(m)
		{
			setupActions();
		}
	}

	void PluginCombobox::skinChanged()
	{
		if(!m)
		{
			return;
		}

		auto i = 0;

		const auto containers = m->pluginHandler->libraries(true);
		for(const auto* container: containers)
		{
			if(this->itemData(i, Qt::DisplayRole).toString().isEmpty())
			{
				i++;
			}

			const auto icon = container->icon();
			this->setItemIcon(i, icon);
			i++;
		}
	}
}