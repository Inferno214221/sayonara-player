/* ExternTracksPlaylistGenerator.cpp */

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

#include "ExternTracksPlaylistGenerator.h"

#include "Components/Directories/MetaDataScanner.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QStringList>
#include <QThread>

namespace
{
	PlaylistPtr addNewPlaylist(PlaylistCreator* playlistCreator)
	{
		const auto name = playlistCreator->requestNewPlaylistName();
		auto index = playlistCreator->createPlaylist(MetaDataList(), name, true);

		return playlistCreator->playlist(index);
	}
}

struct ExternTracksPlaylistGenerator::Private
{
	PlaylistCreator* playlistCreator;
	PlaylistPtr playlist;
	int targetRowIndex;

	Private(PlaylistCreator* playlistCreator, const PlaylistPtr& playlist) :
		playlistCreator {playlistCreator},
		playlist(playlist ? playlist : addNewPlaylist(playlistCreator)),
		targetRowIndex {-1} {}
};

ExternTracksPlaylistGenerator::ExternTracksPlaylistGenerator(PlaylistCreator* playlistCreator,
                                                             const PlaylistPtr& playlist)
{
	m = Pimpl::make<Private>(playlistCreator, playlist);
}

ExternTracksPlaylistGenerator::~ExternTracksPlaylistGenerator() = default;

void ExternTracksPlaylistGenerator::insertPaths(const QStringList& paths, int targetRowIndex)
{
	if(!paths.isEmpty())
	{
		m->targetRowIndex = targetRowIndex;
		scanFiles(paths);
	}
}

void ExternTracksPlaylistGenerator::addPaths(const QStringList& paths)
{
	const auto mode = m->playlist->mode();
	if(!Playlist::Mode::isActiveAndEnabled(mode.append()))
	{
		Playlist::clear(*m->playlist);
	}

	insertPaths(paths, Playlist::count(*m->playlist));
}

void ExternTracksPlaylistGenerator::scanFiles(const QStringList& paths)
{
	m->playlist->setBusy(true);

	using Directory::MetaDataScanner;

	auto* t = new QThread();
	auto* worker = new MetaDataScanner(paths, true, nullptr);

	connect(t, &QThread::started, worker, &MetaDataScanner::start);
	connect(t, &QThread::finished, t, &QObject::deleteLater);
	connect(worker, &MetaDataScanner::sigFinished, this, &ExternTracksPlaylistGenerator::filesScanned);
	connect(worker, &MetaDataScanner::sigFinished, t, &QThread::quit);

	worker->moveToThread(t);
	t->start();
}

void ExternTracksPlaylistGenerator::filesScanned()
{
	auto* worker = dynamic_cast<Directory::MetaDataScanner*>(sender());

	m->playlist->setBusy(false);

	const auto tracks = worker->metadata();
	worker->deleteLater();

	if(tracks.isEmpty())
	{
		emit sigFinished();
		return;
	}

	if(m->targetRowIndex < 0)
	{
		Playlist::clear(*m->playlist);
		Playlist::insertTracks(*m->playlist, tracks, 0);
	}

	else if(m->targetRowIndex >= Playlist::count(*m->playlist))
	{
		Playlist::appendTracks(*m->playlist, tracks);
	}

	else
	{
		Playlist::insertTracks(*m->playlist, tracks, m->targetRowIndex);
	}

	emit sigFinished(); // NOLINT(readability-misleading-indentation)
}
