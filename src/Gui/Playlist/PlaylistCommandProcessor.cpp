/* PlaylistCommandProcessor.cpp, (Created on 20.02.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of Sayonara Player
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

#include "PlaylistCommandProcessor.h"

#include "PlaylistView.h"
#include "PlaylistModel.h"
#include "Utils/Language/Language.h"

#include <QString>

namespace
{
	constexpr const auto* CmdSortTitle = "sort-title";
	constexpr const auto* CmdSortRating = "sort-rating";
	constexpr const auto* CmdSortTrackNr = "sort-nr";
	constexpr const auto* CmdSortFilename = "sort-file";
	constexpr const auto* CmdLock = "lck";
	constexpr const auto* CmdShuffle = "shuffle";
	constexpr const auto* CmdReverse = "rev";
}

namespace Playlist
{
	struct CommandProcessor::Private
	{
		View* view;
		Model* model;

		Private(View* view, Model* model) :
			view {view},
			model {model} {}
	};

	CommandProcessor::CommandProcessor(View* view, Model* model) :
		m {Pimpl::make<Private>(view, model)} {}

	CommandProcessor::~CommandProcessor() = default;

	QMap<QString, QString> CommandProcessor::commands() const
	{
		auto result = QMap<QString, QString> {
			{CmdLock, Lang::get(Lang::LockPlaylist) + "/" + Lang::get(Lang::UnlockPlaylist)},
			{"j {n}", QObject::tr("Jump to row n")}
		};

		if(!m->view->isLocked())
		{
			result[CmdSortTitle] = QObject::tr("Sort by title");
			result[CmdSortTrackNr] = QObject::tr("Sort by track numb");
			result[CmdSortRating] = QObject::tr("Sort by rating");
			result[CmdSortFilename] = QObject::tr("Sort by filename");
			result[CmdReverse] = Lang::get(Lang::ReverseOrder);
			result[CmdShuffle] = Lang::get(Lang::ShufflePlaylist);
		}

		return result;
	}

	void CommandProcessor::runCommand(const QString& command)
	{
		if(command == CmdLock)
		{
			m->model->setLocked(!m->model->isLocked());
		}

		if(command == CmdShuffle)
		{
			m->model->randomizeTracks();
		}

		else if(command == CmdReverse)
		{
			m->model->reverseTracks();
		}

		else if(command == CmdSortTitle)
		{
			m->model->sortTracks(Library::TrackSortorder::TitleAsc);
		}

		else if(command == CmdSortTrackNr)
		{
			m->model->sortTracks(Library::TrackSortorder::TrackNumberAsc);
		}

		else if(command == CmdSortRating)
		{
			m->model->sortTracks(Library::TrackSortorder::RatingDesc);
		}

		else if(command == CmdSortFilename)
		{
			m->model->sortTracks(Library::TrackSortorder::FilenameAsc);
		}

		else if(command.indexOf(QRegExp("j\\s?[0-9]+")) == 0)
		{
			const auto re = QRegExp("([0-9]+)");
			if(const auto index = re.indexIn(command); index > 0)
			{
				bool ok = false;
				if(const auto line = re.cap(1).toInt(&ok); ok && (line > 0) && (line <= m->model->rowCount()))
				{
					m->view->gotoRow(line - 1);
					m->view->selectRow(line - 1);
				}
			}
		}
	}
} // Playlist