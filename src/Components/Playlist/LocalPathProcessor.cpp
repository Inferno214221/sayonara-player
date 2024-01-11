/* ExternTracksPlaylistGenerator.cpp */

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

#include "LocalPathProcessor.h"

#include "Components/Directories/MetaDataScanner.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QStringList>
#include <QThread>
#include <utility>

namespace Playlist
{
	struct LocalPathProcessor::Private
	{
		PlaylistPtr playlist;
		int targetRowIndex {-1};

		explicit Private(PlaylistPtr playlist) :
			playlist {std::move(playlist)} {}
	};

	LocalPathProcessor::LocalPathProcessor(const PlaylistPtr& playlist)
	{
		m = Pimpl::make<Private>(playlist);
	}

	LocalPathProcessor::~LocalPathProcessor() = default;

	void LocalPathProcessor::insertPaths(const QStringList& paths, int targetRowIndex)
	{
		if(!paths.isEmpty())
		{
			m->targetRowIndex = targetRowIndex;
			scanFiles(paths);
		}
	}

	void LocalPathProcessor::addPaths(const QStringList& paths)
	{
		const auto mode = m->playlist->mode();
		if(!Mode::isActiveAndEnabled(mode.append()))
		{
			clear(*m->playlist, Reason::AsyncPlaylistCreator);
		}

		insertPaths(paths, count(*m->playlist));
	}

	void LocalPathProcessor::scanFiles(const QStringList& paths)
	{
		m->playlist->setBusy(true);

		using Directory::MetaDataScanner;

		auto* t = new QThread();
		auto* worker = new MetaDataScanner(paths, true, nullptr);

		connect(t, &QThread::started, worker, &MetaDataScanner::start);
		connect(t, &QThread::finished, t, &QObject::deleteLater);
		connect(worker, &MetaDataScanner::sigFinished, this, &LocalPathProcessor::filesScanned);
		connect(worker, &MetaDataScanner::sigFinished, t, &QThread::quit);

		worker->moveToThread(t);
		t->start();
	}

	void LocalPathProcessor::filesScanned()
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
			clear(*m->playlist, Reason::AsyncPlaylistCreator);
			insertTracks(*m->playlist, tracks, 0, Reason::AsyncPlaylistCreator);
		}

		else if(m->targetRowIndex >= count(*m->playlist))
		{
			appendTracks(*m->playlist, tracks, Reason::AsyncPlaylistCreator);
		}

		else
		{
			insertTracks(*m->playlist, tracks, m->targetRowIndex, Reason::AsyncPlaylistCreator);
		}

		emit sigFinished(); // NOLINT(readability-misleading-indentation)
	}
}