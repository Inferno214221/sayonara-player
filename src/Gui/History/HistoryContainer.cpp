/* HistoryContainer.cpp
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

#include "HistoryContainer.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Components/Session/Session.h"
#include "Gui/History/GUI_History.h"
#include "Gui/Utils/Icons.h"

#include <QIcon>

struct HistoryContainer::Private
{
	std::shared_ptr<GUI_History> widget = nullptr;
	LibraryPlaylistInteractor* libraryPlaylistInteractor;
	Session::Manager* sessionManager;

	Private(LibraryPlaylistInteractor* libraryPlaylistInteractor, Session::Manager* sessionManager) :
		libraryPlaylistInteractor {libraryPlaylistInteractor},
		sessionManager(sessionManager) {}
};

HistoryContainer::HistoryContainer(LibraryPlaylistInteractor* libraryPlaylistInteractor,
                                   Session::Manager* sessionManager, Library::PluginHandler* pluginHandler) :
	Gui::Library::Container(pluginHandler),
	m {Pimpl::make<Private>(libraryPlaylistInteractor, sessionManager)} {}

HistoryContainer::~HistoryContainer() = default;

QString HistoryContainer::name() const { return "history"; }

QString HistoryContainer::displayName() const { return QObject::tr("History"); }

QWidget* HistoryContainer::widget() const { return m->widget.get(); }

QFrame* HistoryContainer::header() const { return m->widget->header(); }

QIcon HistoryContainer::icon() const { return Gui::Icons::icon(Gui::Icons::Edit); }

void HistoryContainer::initUi()
{
	m->widget = std::make_shared<GUI_History>(m->libraryPlaylistInteractor,
	                                          m->sessionManager);
}

void HistoryContainer::rename(const QString& /*newName*/) {}

QMenu* HistoryContainer::menu() { return nullptr; }

bool HistoryContainer::isLocal() const { return false; }
