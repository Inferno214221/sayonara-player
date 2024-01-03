/* SoundcloudLibraryContainer.cpp */

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
	SC::Library* library;
	SC::GUI_Library* ui {nullptr};

	Private(LibraryPlaylistInteractor* playlistInteractor, QObject* parent) :
		playlistInteractor(playlistInteractor),
		tokenThread {new QThread {parent}},
		library {new SC::Library(playlistInteractor, parent)} {}

	~Private()
	{
		while(tokenThread->isRunning())
		{
			tokenThread->quit();
			QThread::currentThread()->wait(10); // NOLINT(readability-magic-numbers)
		}
	}
};

SC::LibraryContainer::LibraryContainer(LibraryPlaylistInteractor* playlistInteractor,
                                       ::Library::PluginHandler* pluginHandler) :
	Gui::Library::Container(pluginHandler),
	m {Pimpl::make<Private>(playlistInteractor, this)}
{
	initSoundcloudIcons();

	auto* tokenObserver = new SC::TokenObserver(nullptr);
	auto* thread = new QThread(nullptr);
	tokenObserver->moveToThread(thread);

	connect(thread, &QThread::started, tokenObserver, &SC::TokenObserver::start);
	connect(thread, &QThread::finished, thread, &QThread::deleteLater);

	thread->start();
}

SC::LibraryContainer::~LibraryContainer() = default;

QString SC::LibraryContainer::name() const { return "soundcloud"; }

QString SC::LibraryContainer::displayName() const { return "Soundcloud"; }

QWidget* SC::LibraryContainer::widget() const { return m->ui; }

QMenu* SC::LibraryContainer::menu() { return m->ui ? m->ui->getMenu() : nullptr; }

void SC::LibraryContainer::initUi() { m->ui = new SC::GUI_Library(m->library); }

QFrame* SC::LibraryContainer::header() const { return m->ui->headerFrame(); }

QIcon SC::LibraryContainer::icon() const { return QIcon(":/sc_icons/icon.png"); }

void SC::LibraryContainer::rename(const QString& /*newName*/) {}

bool SC::LibraryContainer::isLocal() const { return false; }
