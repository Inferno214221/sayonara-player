/* LFMSimArtistsParser.cpp */

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

#include "LfmSimiliarArtistsParser.h"
#include "DynamicPlayback/ArtistMatch.h"
#include "Utils/Logger/Logger.h"

#include <QFile>
#include <QMap>
#include <QString>
#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>

namespace
{
	DynamicPlayback::ArtistMatch::Entry parseArtist(const QDomNode& node)
	{
		if(node.nodeName().toLower() != "artist")
		{
			return {};
		}

		auto result = DynamicPlayback::ArtistMatch::Entry {};
		const auto childNodes = node.childNodes();
		for(int contentIndex = 0; contentIndex < childNodes.size(); contentIndex++)
		{
			const auto childNode = childNodes.at(contentIndex);
			const auto nodeName = childNode.nodeName().toLower();
			const auto element = childNode.toElement();
			if(element.isNull())
			{
				continue;
			}

			if(nodeName == "name")
			{
				result.artist = element.text();
			}

			else if(nodeName == "match")
			{
				result.similarity = element.text().toDouble();
			}

			else if(nodeName == "mbid")
			{
				result.mbid = element.text();
			}

			if(result.isValid())
			{
				break;
			}
		}

		return result;
	}

	QPair<bool, QString> parseError(const QDomElement& rootElement)
	{
		if(rootElement.hasAttribute("status"))
		{
			if(rootElement.attribute("status", "failed") != "ok")
			{
				return {true, rootElement.text()};
			}
		}

		return {false, {}};
	}
}

namespace DynamicPlayback
{
	ParsingResult parseLastFMAnswer(const QString& artistName, const QByteArray& data)
	{
		QDomDocument doc("similarArtists");

		if(!doc.setContent(data))
		{
			return {{}, "Cannot parse document", true};
		}

		ArtistMatch artistMatch(artistName);

		const auto docElement = doc.documentElement();
		const auto [hasError, error] = parseError(docElement);
		if(hasError)
		{
			return {{}, QString("Cannot parse document: %1").arg(error), hasError};
		}

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

		return {artistMatch, {}, hasError};
	}
}