/* LFMSimArtistsParser.cpp */

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

#include "LfmSimiliarArtistsParser.h"
#include "DynamicPlayback/ArtistMatch.h"
#include "Utils/Logger/Logger.h"

#include <QFile>
#include <QMap>
#include <QString>
#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>

using DynamicPlayback::ArtistMatch;

namespace
{
	ArtistMatch::Entry parseArtist(const QDomNode& node)
	{
		if(node.nodeName().compare("artist", Qt::CaseInsensitive) != 0)
		{
			return ArtistMatch::Entry {};
		}

		ArtistMatch::Entry result;
		const auto childNodes = node.childNodes();
		for(int contentIndex = 0; contentIndex < childNodes.size(); contentIndex++)
		{
			const auto childNode = childNodes.at(contentIndex);
			const auto nodeName = childNode.nodeName().toLower();
			const auto element = childNode.toElement();

			if(nodeName.compare("name") == 0)
			{
				if(!element.isNull())
				{
					result.artist = element.text();
				}
			}

			else if(nodeName.compare("match") == 0)
			{
				if(!element.isNull())
				{
					result.similarity = element.text().toDouble();
				}
			}

			else if(nodeName.compare("mbid") == 0)
			{
				if(!element.isNull())
				{
					result.mbid = element.text();
				}
			}

			if(result.isValid())
			{
				break;
			}
		}

		return result;
	}
}

ArtistMatch DynamicPlayback::parseLastFMAnswer(const QString& artistName, const QByteArray& data)
{
	QDomDocument doc("similarArtists");

	if(!doc.setContent(data))
	{
		spLog(Log::Warning, "LFMSimArtistParser") << "Cannot parse similar artists document";
		return ArtistMatch {};
	}

	ArtistMatch artistMatch(artistName);

	const auto docElement = doc.documentElement();
	const auto similarArtists = docElement.firstChild();
	const auto childNodes = similarArtists.childNodes();

	for(int artistIndex = 0; artistIndex < childNodes.size(); artistIndex++)
	{
		const auto artistNode = childNodes.item(artistIndex);
		const auto artistEntry = parseArtist(artistNode);
		if(artistEntry.isValid())
		{
			artistMatch.add(artistEntry);
		}
	}

	return artistMatch;
}
