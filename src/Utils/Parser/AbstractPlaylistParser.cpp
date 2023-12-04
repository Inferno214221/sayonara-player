/* AbstractPlaylistParser.cpp */

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

#include "AbstractPlaylistParser.h"

#include "Utils/FileSystem.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Parser/PlaylistParser.h"

#include <QDir>

struct AbstractPlaylistParser::Private
{
	Util::FileSystemPtr fileSystem;
	Tagging::TagReaderPtr tagReader;
	QString fileContent;
	QString directory;
	MetaDataList tracks;

	Private(const QString& filename, Util::FileSystemPtr fileSystem_, Tagging::TagReaderPtr tagReader) :
		fileSystem {std::move(fileSystem_)},
		tagReader {std::move(tagReader)},
		fileContent {fileSystem->readFileIntoString(filename)},
		directory {Util::File::getParentDirectory(filename)} {}
};

AbstractPlaylistParser::AbstractPlaylistParser(const QString& filename,
                                               const Util::FileSystemPtr& fileSystem,
                                               const Tagging::TagReaderPtr& tagReader) :
	m {Pimpl::make<Private>(filename, fileSystem, tagReader)} {}

AbstractPlaylistParser::~AbstractPlaylistParser() = default;

MetaDataList AbstractPlaylistParser::tracks()
{
	m->tracks.clear();

	parse();

	if(m->tagReader)
	{
		for(auto& track: m->tracks)
		{
			auto maybeTrack = m->tagReader->readMetadata(track.filepath());
			if(maybeTrack)
			{
				std::swap(track, maybeTrack.value());
			}
		}
	}

	return m->tracks;
}

QString AbstractPlaylistParser::getAbsoluteFilename(const QString& filename) const
{
	if(filename.isEmpty())
	{
		return {};
	}

	if(Util::File::isWWW(filename))
	{
		return filename;
	}

	const auto fullPath = Util::File::isAbsolute(filename)
	                      ? filename
	                      : QDir(m->directory).absoluteFilePath(filename);

	return m->fileSystem->exists(fullPath)
	       ? Util::File::cleanFilename(fullPath)
	       : QString {};
}

void AbstractPlaylistParser::addTrack(const MetaData& track)
{
	if(!track.filepath().isEmpty())
	{
		m->tracks << track;
	}
}

void AbstractPlaylistParser::addTracks(const MetaDataList& tracks)
{
	for(const auto& track: tracks)
	{
		addTrack(track);
	}
}

const QString& AbstractPlaylistParser::content() const { return m->fileContent; }

void AbstractPlaylistParser::parseSubPlaylist(const QString& playlistPath)
{
	addTracks(PlaylistParser::parsePlaylist(playlistPath, m->fileSystem, m->tagReader));
}
