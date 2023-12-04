/* ASXParser.cpp */

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

#include "ASXParser.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/FileSystem.h"
#include "Utils/Tagging/TagReader.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>

namespace
{
	QString parseRefNode(const QDomNode& node)
	{
		const auto element = node.toElement();
		const auto attributes = element.attributes();

		for(auto j = 0; j < attributes.size(); j++)
		{
			const auto itemNode = attributes.item(j);
			const auto nodeName = itemNode.nodeName();
			if(nodeName.toLower() == "href")
			{
				return element.attribute(nodeName);
			}
		}

		return {};
	}

	MetaData parseTrack(const QDomNode& entry, std::function<QString(QString)>&& fileNameConverter)
	{
		auto track = MetaData {};
		track.setDurationMs(0);
		track.setAlbum("");

		for(int i = 0; i < entry.childNodes().size(); i++)
		{
			const auto node = entry.childNodes().at(i);
			const auto nodeName = node.nodeName().toLower();

			const auto e = node.toElement();
			if(e.isNull())
			{
				continue;
			}

			if(nodeName == "ref")
			{
				const auto parsed = parseRefNode(node);
				const auto absoluteFilename = fileNameConverter(parseRefNode(node));
				track.setFilepath(absoluteFilename);
			}

			else if(nodeName == "title")
			{
				track.setTitle(e.text());
			}

			else if(nodeName == "album")
			{
				track.setAlbum(e.text());
			}

			else if(nodeName == "author")
			{
				track.setArtist(e.text());
			}
		}

		return track;
	}

	MetaData applyGlobalAttributes(MetaData metaData, const QString& author, const QString& album, const QString& title)
	{
		if(!author.isEmpty())
		{
			metaData.setAlbumArtist(author);
			if(metaData.artist().isEmpty())
			{
				metaData.setArtist(author);
			}
		}

		if(!album.isEmpty() && metaData.album().isEmpty())
		{
			metaData.setAlbum(album);
		}

		if(!title.isEmpty() && metaData.title().isEmpty())
		{
			metaData.setTitle(title);
		}

		return metaData;
	}
}

ASXParser::ASXParser(const QString& filename,
                     const Util::FileSystemPtr& fileSystem,
                     const Tagging::TagReaderPtr& tagReader) :
	AbstractPlaylistParser(filename, fileSystem, tagReader) {}

ASXParser::~ASXParser() = default;

void ASXParser::parse()
{
	auto doc = QDomDocument("AsxFile");
	doc.setContent(content());

	const auto docElement = doc.documentElement();
	auto entryElement = docElement.firstChildElement("entry");
	auto entry = static_cast<QDomNode>(entryElement);

	const auto title = docElement.firstChildElement("title").text();
	const auto author = docElement.firstChildElement("author").text();
	const auto album = docElement.firstChildElement("album").text();

	int index = 1;
	while(!entry.isNull() && entry.hasChildNodes())
	{
		auto metaData = parseTrack(entry, [this](const auto& relativePath) {
			return getAbsoluteFilename(relativePath);
		});

		metaData = applyGlobalAttributes(metaData, author, album, title);
		metaData.setTrackNumber(index++);

		addTrack(metaData);
		entry = entry.nextSibling();
	}
}
