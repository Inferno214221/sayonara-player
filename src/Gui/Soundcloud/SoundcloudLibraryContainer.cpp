/* SoundcloudLibraryContainer.cpp */

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

/* SoundcloudLibraryContainer.cpp */

#include "SoundcloudLibraryContainer.h"
#include "Gui/Soundcloud/GUI_SoundcloudLibrary.h"
#include "Components/Streaming/Soundcloud/SoundcloudLibrary.h"
#include "Components/Playlist/PlaylistHandler.h"

#include <qglobal.h>
#include <QPixmap>

static void sc_init_icons()
{
	Q_INIT_RESOURCE(SoundcloudIcons);
}

struct SC::LibraryContainer::Private
{
	Playlist::Handler* playlistHandler;

	Private(Playlist::Handler* playlistHandler) :
		playlistHandler(playlistHandler)
	{}
};

SC::LibraryContainer::LibraryContainer(Playlist::Handler* playlistHandler, QObject* parent) :
	::Library::Container(parent)
{
	sc_init_icons();

	m = Pimpl::make<Private>(playlistHandler);
}

SC::LibraryContainer::~LibraryContainer() = default;

QString SC::LibraryContainer::name() const
{
	return "soundcloud";
}

QString SC::LibraryContainer::displayName() const
{
	return "Soundcloud";
}

QWidget* SC::LibraryContainer::widget() const
{
	return static_cast<QWidget*>(ui);
}

QMenu* SC::LibraryContainer::menu()
{
	if(ui){
		return ui->getMenu();
	}

	return nullptr;
}

void SC::LibraryContainer::initUi()
{
	SC::Library* library = new SC::Library(m->playlistHandler, this);
	ui = new SC::GUI_Library(library);
}

QFrame* SC::LibraryContainer::header() const
{
	return ui->headerFrame();
}

QPixmap SC::LibraryContainer::icon() const
{
	sc_init_icons();
	return QPixmap(":/sc_icons/icon.png");
}
