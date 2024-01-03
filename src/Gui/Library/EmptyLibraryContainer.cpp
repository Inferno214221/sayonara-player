/* EmptyLibraryContainer.cpp */

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

#include "EmptyLibraryContainer.h"
#include "GUI_EmptyLibrary.h"

#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Gui/Utils/Icons.h"
#include "Utils/Language/Language.h"

#include <QIcon>
#include <QComboBox>
#include <QPixmap>

using namespace Library;

struct EmptyLibraryContainer::Private
{
	Library::Manager* libraryManager;
	std::shared_ptr<GUI_EmptyLibrary> ui = nullptr;

	explicit Private(Library::Manager* libraryManager) :
		libraryManager {libraryManager} {}
};

EmptyLibraryContainer::EmptyLibraryContainer(Library::Manager* libraryManager,
                                             Library::PluginHandler* pluginHandler) :
	Gui::Library::Container(pluginHandler),
	m {Pimpl::make<Private>(libraryManager)} {}

EmptyLibraryContainer::~EmptyLibraryContainer() = default;

QString EmptyLibraryContainer::name() const { return "empty-library"; }

QString EmptyLibraryContainer::displayName() const { return Lang::get(Lang::New); }

QWidget* EmptyLibraryContainer::widget() const { return static_cast<QWidget*>(m->ui.get()); }

QMenu* EmptyLibraryContainer::menu() { return nullptr; }

void EmptyLibraryContainer::initUi() { m->ui = std::make_shared<GUI_EmptyLibrary>(m->libraryManager); }

QFrame* EmptyLibraryContainer::header() const { return m->ui->headerFrame(); }

QIcon EmptyLibraryContainer::icon() const { return Gui::Icons::icon(Gui::Icons::Star); }

void EmptyLibraryContainer::rename(const QString& /*newName*/) {}

bool EmptyLibraryContainer::isLocal() const { return false; }
