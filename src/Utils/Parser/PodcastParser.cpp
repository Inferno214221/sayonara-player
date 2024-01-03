/* PodcastParser.cpp */

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

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/PodcastParser.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"

#include <algorithm>

#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QStringList>

namespace
{
	Year findYear(const QString& str)
	{
		const auto index = str.indexOf(QRegExp("[0-9]{4,4}"));
		return (index >= 0)
		       ? Year(str.midRef(index, 4).toInt())
		       : 0;
	}

	int parseLengthInSeconds(const QString& str)
	{
		QStringList lst = str.split(":");
		auto hour = 0;
		auto minute = 0;
		auto seconds = 0;

		if(lst.size() == 3)
		{
			hour = lst[0].toInt();
			minute = lst[1].toInt();
			seconds = lst[2].split(".").first().toInt();
		}

		if(lst.size() == 2)
		{
			minute = lst[0].toInt();
			seconds = lst[1].split(".").first().toInt();
		}

		if(lst.size() == 1)
		{
			seconds = lst[0].split(".").first().toInt();
		}

		return (hour * 3600 + minute * 60 + seconds); // NOLINT(readability-magic-numbers)
	}

	MetaData setChapterInfo(QXmlStreamReader& reader, MetaData track)
	{
		auto chapterCount = 0;
		while(reader.readNextStartElement())
		{
			QString title, lengthStr;

			const auto attributes = reader.attributes();
			for(const auto& attr: attributes)
			{
				if(attr.name() == "start")
				{
					const auto length = parseLengthInSeconds(attr.value().toString());
					lengthStr = QString::number(length);
				}

				else if(attr.name() == "title")
				{
					title = attr.value().toString();
				}
			}

			if(title.isEmpty() || lengthStr.isEmpty())
			{
				reader.skipCurrentElement();
			}

			chapterCount++;

			const auto chapterInfo = lengthStr + ":" + title;
			const auto chapterKey = QString("Chapter %1").arg(chapterCount);

			track.addCustomField(chapterKey, chapterKey, chapterInfo);
		}

		return track;
	}

	QString decide(const QString& choice1, const QString& choice2)
	{
		return (choice1.trimmed().isEmpty())
		       ? choice2.trimmed()
		       : choice1.trimmed();
	}

	QStringList toList(const QString& choice1, const QString& choice2)
	{
		auto result = QStringList {};

		if(!choice1.trimmed().isEmpty())
		{
			result.push_back(choice1.trimmed());
		}
		if(!choice2.trimmed().isEmpty())
		{
			result.push_back(choice2.trimmed());
		}

		result.removeDuplicates();

		return result;
	}

	MetaData parseItem(QXmlStreamReader& reader)
	{
		struct Info
		{
			QString author;
			QString title;
			QString summary;
		} standardInfo, iTunesInfo;

		MetaData track;

		while(reader.readNextStartElement())
		{
			if(reader.prefix() == "itunes")
			{
				// covers for single tracks are ignored since the playlist and the DB is not yet capable
				// of storing covers on a per-track base (hashes are calculated for artist/album combinations in
				// Cover::Location and PlaylistModel)
				if(reader.name() == "author")
				{
					iTunesInfo.author = reader.readElementText().trimmed();
				}

				else if(reader.name() == "duration")
				{
					const auto len = parseLengthInSeconds(reader.readElementText().trimmed());
					track.setDurationMs(len * 1000); // NOLINT(readability-magic-numbers)
				}

				else if(reader.name() == "title")
				{
					iTunesInfo.title = reader.readElementText().trimmed();
				}

				else if(reader.name() == "summary")
				{
					iTunesInfo.summary = reader.readElementText().trimmed();
				}

				else
				{
					reader.skipCurrentElement();
				}
			}

			else if(reader.name() == "title")
			{
				standardInfo.title = reader.readElementText().trimmed();
			}

			else if(reader.name() == "description")
			{
				standardInfo.summary = reader.readElementText().trimmed();
			}

			else if(reader.name() == "enclosure")
			{
				const auto attributes = reader.attributes();
				track.setFilepath(attributes.value("url").toString());
				reader.skipCurrentElement();
			}

			else if(reader.name() == "link")
			{
				const auto link = reader.readElementText().trimmed();
				track.addCustomField("link", "Link", QString("<a href=\"%1\">%1</a>").arg(link));
			}

			else if(reader.name() == "author")
			{
				standardInfo.author = decide(reader.readElementText(), standardInfo.author);
			}

			else if(reader.name() == "pubDate")
			{
				const auto text = reader.readElementText().trimmed();
				const auto year = findYear(reader.readElementText().trimmed());
				track.setYear(year);
				track.addCustomField("pubDate", "Publication Date", text);
			}

			else if((reader.prefix() == "dc") && (reader.name() == "date"))
			{
				const auto year = findYear(reader.readElementText().trimmed());
				if(track.year() < 1000) // NOLINT(readability-magic-numbers)
				{
					track.setYear(year);
				}
			}

			else if(reader.prefix() == "psc" && reader.name() == "chapters")
			{
				setChapterInfo(reader, track);
			}

			else
			{
				reader.skipCurrentElement();
			}
		}

		track.setArtist(decide(iTunesInfo.author, standardInfo.author));
		track.setTitle(decide(iTunesInfo.title, standardInfo.title));
		track.addCustomField("title", "Title", track.title());
		track.addCustomField("description", "Description", decide(iTunesInfo.summary, standardInfo.summary));
		track.changeRadioMode(RadioMode::Podcast);

		return track;
	}

	QStringList parseCategory(QXmlStreamReader& reader) // NOLINT(misc-no-recursion)
	{
		QStringList ret;
		const auto attributes = reader.attributes();
		for(const QXmlStreamAttribute& attr: attributes)
		{
			if(attr.name() == "text")
			{
				ret << attr.value().toString();
			}
		}

		while(reader.readNextStartElement())
		{
			if(reader.name() == "category")
			{
				ret << parseCategory(reader);
			}

			else
			{
				reader.skipCurrentElement();
			}
		}

		return ret;
	}

	MetaDataList parseChannel(QXmlStreamReader& reader) // NOLINT(readability-function-cognitive-complexity)
	{
		struct Info
		{
			QString title;
			QString summary;
			QString album;
			QString author;
			QString coverUrl;
			QString comment;
			QStringList categories;
		} standardInfo, iTunesInfo;

		MetaDataList result;

		const auto name = reader.name().toString();
		while(reader.readNextStartElement())
		{
			auto n = reader.name().toString();

			if(reader.prefix() == "itunes")
			{
				if(reader.name() == "author")
				{
					iTunesInfo.author = reader.readElementText().trimmed();
				}

				else if(reader.name() == "category")
				{
					iTunesInfo.categories = parseCategory(reader);
				}

				else if(reader.name() == "image")
				{
					const auto attributes = reader.attributes();
					iTunesInfo.coverUrl = attributes.value("href").toString();

					reader.skipCurrentElement();
				}

				else if(reader.name() == "summary")
				{
					iTunesInfo.summary = reader.readElementText().trimmed();
				}

				else if(reader.name() == "title")
				{
					iTunesInfo.title = reader.readElementText().trimmed();
				}

				else
				{
					reader.skipCurrentElement();
				}
			}

			else if(reader.name() == "title")
			{
				standardInfo.title = reader.readElementText().trimmed();
			}

			else if(reader.name() == "image" && standardInfo.coverUrl.isEmpty())
			{
				while(reader.readNextStartElement())
				{
					if(reader.name() == "url")
					{
						standardInfo.coverUrl = reader.readElementText().trimmed();
					}

					else
					{
						reader.skipCurrentElement();
					}
				}
			}

			else if(reader.name() == "item")
			{
				auto track = parseItem(reader);
				if(!track.filepath().isEmpty())
				{
					result << std::move(track);
				}
			}

			else
			{
				reader.skipCurrentElement();
			}
		}

		for(auto& track: result)
		{
			if(track.artist().isEmpty())
			{
				track.setArtist(decide(iTunesInfo.author, standardInfo.author));
			}

			if(track.album().isEmpty())
			{
				track.setAlbum(decide(iTunesInfo.album, standardInfo.album));
			}

			if(track.title().isEmpty())
			{
				track.setTitle(decide(iTunesInfo.title, standardInfo.title));
			}

			if(track.comment().isEmpty())
			{
				track.setComment(decide(iTunesInfo.summary, standardInfo.summary));
			}

			track.setGenres(toList(iTunesInfo.categories.join(","), standardInfo.categories.join(",")));
			track.setCoverDownloadUrls(toList(iTunesInfo.coverUrl, standardInfo.coverUrl));
		}

		return result;
	}

}

MetaDataList PodcastParser::parsePodcastXmlFile(const QString& content)
{
	MetaDataList result;

	QXmlStreamReader reader(content);
	while(reader.readNextStartElement())
	{
		if(reader.name() == "rss")
		{
			while(reader.readNextStartElement())
			{
				if(reader.name() == "channel")
				{
					result = parseChannel(reader);
				}

				else
				{
					reader.skipCurrentElement();
				}
			}
		}

		else
		{
			reader.skipCurrentElement();
		}
	}

	return result;
}


