/* PlaylistParser.cpp */

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

#include "M3UParser.h"
#include "PLSParser.h"
#include "ASXParser.h"

#include "Utils/Parser/PlaylistParser.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QUrl>

namespace
{
	template<typename ParserType>
	std::shared_ptr<ParserType> tryOutParser(const QString& filename)
	{
		auto parser = std::make_shared<ParserType>(filename);
		const auto tracks = parser->tracks();

		return (!parser->tracks().isEmpty())
		       ? parser
		       : nullptr;
	}

	std::shared_ptr<AbstractPlaylistParser> determinePlaylistParser(const QString& filename)
	{
		if(auto parser = tryOutParser<M3UParser>(filename); parser)
		{
			return parser;
		}

		if(auto parser = tryOutParser<PLSParser>(filename); parser)
		{
			return parser;
		}

		if(auto parser = tryOutParser<ASXParser>(filename); parser)
		{
			return parser;
		}

		return nullptr;
	}

	std::shared_ptr<AbstractPlaylistParser> getPlaylistParser(const QString& filename)
	{
		if(filename.endsWith(QStringLiteral("m3u"), Qt::CaseInsensitive))
		{
			return std::make_shared<M3UParser>(filename);
		}

		if(filename.endsWith(QStringLiteral("pls"), Qt::CaseInsensitive))
		{
			return std::make_shared<PLSParser>(filename);
		}

		if(filename.endsWith(QStringLiteral("ram"), Qt::CaseInsensitive))
		{
			return std::make_shared<M3UParser>(filename);
		}

		if(filename.endsWith(QStringLiteral("asx"), Qt::CaseInsensitive))
		{
			return std::make_shared<ASXParser>(filename);
		}

		return determinePlaylistParser(filename);
	}
}

MetaDataList PlaylistParser::parsePlaylist(const QString& filename, bool parseTags)
{
	MetaDataList result;
	if(Util::File::isWWW(filename))
	{
		return result;
	}

	const auto playlistParser = getPlaylistParser(filename);
	if(!playlistParser)
	{
		return result;
	}

	auto tracks = playlistParser->tracks(parseTags);
	for(auto& track : tracks)
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

