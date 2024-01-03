/* PlaylistParser.cpp */

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

#include "M3UParser.h"
#include "PLSParser.h"
#include "ASXParser.h"

#include "Utils/FileSystem.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Tagging/TagReader.h"

#include <QUrl>

namespace
{
	template<typename ParserType>
	std::shared_ptr<ParserType> tryOutParser(const QString& filename,
	                                         const Util::FileSystemPtr& fileSystem,
	                                         const Tagging::TagReaderPtr& tagReader)
	{
		const auto parser = std::make_shared<ParserType>(filename, fileSystem, tagReader);
		return !parser->tracks().isEmpty()
		       ? parser
		       : nullptr;
	}

	std::shared_ptr<AbstractPlaylistParser> determinePlaylistParser(const QString& filename,
	                                                                const Util::FileSystemPtr& fileSystem,
	                                                                const Tagging::TagReaderPtr& tagReader)
	{
		if(auto parser = tryOutParser<M3UParser>(filename, fileSystem, tagReader); parser)
		{
			return parser;
		}

		if(auto parser = tryOutParser<PLSParser>(filename, fileSystem, tagReader); parser)
		{
			return parser;
		}

		if(auto parser = tryOutParser<ASXParser>(filename, fileSystem, tagReader); parser)
		{
			return parser;
		}

		return nullptr;
	}

	std::shared_ptr<AbstractPlaylistParser> getPlaylistParser(const QString& filename,
	                                                          const Util::FileSystemPtr& fileSystem,
	                                                          const Tagging::TagReaderPtr& tagReader)
	{
		if(Util::File::getFileExtension(filename).toLower() == "m3u")
		{
			return std::make_shared<M3UParser>(filename, fileSystem, tagReader);
		}

		if(Util::File::getFileExtension(filename).toLower() == "pls")
		{
			return std::make_shared<PLSParser>(filename, fileSystem, tagReader);
		}

		if(Util::File::getFileExtension(filename).toLower() == "ram")
		{
			return std::make_shared<M3UParser>(filename, fileSystem, tagReader);
		}

		if(Util::File::getFileExtension(filename).toLower() == "asx")
		{
			return std::make_shared<ASXParser>(filename, fileSystem, tagReader);
		}

		return determinePlaylistParser(filename, fileSystem, tagReader);
	}
}

MetaDataList PlaylistParser::parsePlaylist(const QString& filename,
                                           const Util::FileSystemPtr& fileSystem,
                                           const Tagging::TagReaderPtr& tagReader)
{
	if(Util::File::isWWW(filename))
	{
		return {};
	}

	const auto playlistParser = getPlaylistParser(filename, fileSystem, tagReader);
	if(!playlistParser)
	{
		return {};
	}

	auto result = MetaDataList {};
	auto tracks = playlistParser->tracks();
	for(auto& track: tracks)
	{
		if(track.title().isEmpty())
		{
			const auto title = Util::File::getFilenameOfPath(track.filepath());
			track.setTitle(title);
		}

		if(Util::File::checkFile(track.filepath()))
		{
			result << std::move(track);
		}
	}

	return result;
}

MetaDataList
PlaylistParser::parsePlaylistWithoutTags(const QString& filename, const std::shared_ptr<Util::FileSystem>& fileSystem)
{
	return parsePlaylist(filename, fileSystem, nullptr);
}
