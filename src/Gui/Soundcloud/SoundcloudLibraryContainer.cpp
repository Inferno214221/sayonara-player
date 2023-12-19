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

#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Components/Playlist/LibraryPlaylistInteractor.h"
#include "Components/Streaming/Soundcloud/SoundcloudLibrary.h"
#include "Components/Streaming/Soundcloud/SoundcloudTokenObserver.h"

#include <QIcon>
#include <QThread>
#include <QtGlobal>

static void initSoundcloudIcons()
{
	Q_INIT_RESOURCE(SoundcloudIcons);
}

struct SC::LibraryContainer::Private
{
	LibraryPlaylistInteractor* playlistInteractor;
	QThread* tokenThread;

	explicit Private(LibraryPlaylistInteractor* playlistInteractor) :
		playlistInteractor(playlistInteractor),
		tokenThread {new QThread {}} {}

	~Private()
	{
		while(tokenThread->isRunning())
		{
			tokenThread->quit();
			QThread::currentThread()->wait(10);
		}
	}
};

SC::LibraryContainer::LibraryContainer(LibraryPlaylistInteractor* playlistInteractor,
                                       ::Library::PluginHandler* pluginHandler) :
	Gui::Library::Container(pluginHandler)
{
	initSoundcloudIcons();

	m = Pimpl::make<Private>(playlistInteractor);

	auto* tokenObserver = new SC::TokenObserver(nullptr);
	auto* thread = new QThread(nullptr);
	tokenObserver->moveToThread(thread);

	connect(thread, &QThread::started, tokenObserver, &SC::TokenObserver::start);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	thread->start();
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
	if(ui)
	{
		return ui->getMenu();
	}

	return nullptr;
}

void SC::LibraryContainer::initUi()
{
	auto* library = new SC::Library(m->playlistInteractor, this);
	ui = new SC::GUI_Library(library);
}

QFrame* SC::LibraryContainer::header() const
{
	return ui->headerFrame();
}

QIcon SC::LibraryContainer::icon() const
{
	initSoundcloudIcons();
	return QIcon(":/sc_icons/icon.png");
}
