/* PodcastParser.cpp */

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

static Year find_year(QString str)
{
	int idx = str.indexOf(QRegExp("[0-9]{4,4}"));

	if(idx >= 0) {
		return Year(str.midRef(idx, 4).toInt());
	}

	return 0;
}

static int parse_length_s(const QString& str)
{
	QStringList lst = str.split(":");
	int h=0;
	int m=0;
	int s=0;

	if(lst.size() == 3)
	{
		h = lst[0].toInt();
		m = lst[1].toInt();
		s = lst[2].split(".").first().toInt();
	}

	if(lst.size() == 2)
	{
		m = lst[0].toInt();
		s = lst[1].split(".").first().toInt();
	}

	if(lst.size() == 1)
	{
		s = lst[0].split(".").first().toInt();
	}

	return (h * 3600 + m * 60 + s);
}

static MetaData parse_item(QXmlStreamReader& reader)
{
	int n_chapters=0;

	MetaData md;

	while(reader.readNextStartElement())
	{
		if(reader.prefix() == "itunes")
		{
			if(reader.name() == "author")
			{
				md.setArtist(reader.readElementText().trimmed());
			}

			else if(reader.name() == "duration")
			{
				int len = parse_length_s(reader.readElementText().trimmed());
				md.setDurationMs(len * 1000);
			}

			else
			{
				reader.skipCurrentElement();
			}

		}

		else if(reader.name() == "title")
		{
			md.setTitle(reader.readElementText().trimmed());
			md.addCustomField("1title", "Title", md.title());
		}

		else if(reader.name() == "description")
		{
			md.addCustomField("2desciption", "Description", reader.readElementText().trimmed());
		}

		else if(reader.name() == "enclosure")
		{
			const QXmlStreamAttributes attributes = reader.attributes();
			for(const QXmlStreamAttribute& attr : attributes)
			{
				if(attr.name() == "url")
				{
					md.setFilepath(attr.value().toString());
				}
			}

			reader.skipCurrentElement();
		}

		else if(reader.name() == "link" && md.filepath().isEmpty())
		{
			md.setFilepath(reader.readElementText().trimmed());
		}

		else if( (reader.name() == "author") && md.artist().isEmpty() )
		{
			md.setArtist(reader.readElementText().trimmed());
		}

		else if((reader.name() == "pubDate"))
		{
			md.setYear(find_year(reader.readElementText().trimmed()));
		}

		else if((reader.prefix() == "dc") && (reader.name() == "date"))
		{
			md.setYear(find_year(reader.readElementText().trimmed()));
		}

		else if(reader.prefix() == "psc" && reader.name() == "chapters")
		{
			while(reader.readNextStartElement())
			{
				QString title, length_str;

				const QXmlStreamAttributes attributes = reader.attributes();
				for(const QXmlStreamAttribute& attr : attributes)
				{
					if(attr.name() == "start")
					{
						int length = parse_length_s(attr.value().toString());
						length_str = QString::number(length);
					}

					else if(attr.name() == "title")
					{
						title = attr.value().toString();
					}
				}

				if(title.isEmpty() || length_str.isEmpty())
				{
					reader.skipCurrentElement();
				}

				n_chapters++;

				QString chapter_info = length_str + ":" + title;
				QString chapter_key = QString("Chapter %1").arg(n_chapters);

				md.addCustomField(chapter_key, chapter_key, chapter_info);
			} // chapter
		} // while chapters.hasElement

		else
		{
			reader.skipCurrentElement();
		}
	}

	md.changeRadioMode(RadioMode::Podcast);
	return md;
}

static QStringList parse_category(QXmlStreamReader& reader)
{
	QStringList ret;
	const QXmlStreamAttributes attributes = reader.attributes();
	for(const QXmlStreamAttribute& attr : attributes)
	{
		if(attr.name() == "text") {
			ret << attr.value().toString();
		}
	}

	while(reader.readNextStartElement())
	{
		if(reader.name() == "category")
		{
			ret << parse_category(reader);
		}

		else
		{
			reader.skipCurrentElement();
		}
	}

	return ret;
}

static MetaDataList parse_channel(QXmlStreamReader& reader)
{
	MetaDataList result;

	QString n = reader.name().toString();
	QString album, author, cover_url;
	QStringList categories;

	while(reader.readNextStartElement())
	{
		auto n = reader.name().toString();

		if(reader.prefix() == "itunes")
		{
			if(reader.name() == "author")
			{
				author = reader.readElementText().trimmed();
			}

			else if(reader.name() == "category")
			{
				categories = parse_category(reader);
			}

			else if(reader.name() == "image")
			{
				const QXmlStreamAttributes attributes = reader.attributes();
				for(const QXmlStreamAttribute& attr : attributes)
				{
					if(attr.name() == "href"){
						cover_url = attr.value().toString();
					}
				}

				reader.skipCurrentElement();
			}

			else
			{
				reader.skipCurrentElement();
			}
		}

		else if(reader.name() == "title")
		{
			album = reader.readElementText().trimmed();
		}

		else if(reader.name() == "image" && cover_url.isEmpty())
		{
			while(reader.readNextStartElement())
			{
				if(reader.name() == "url")
				{
					cover_url = reader.readElementText().trimmed();
				}

				else
				{
					reader.skipCurrentElement();
				}
			}
		}

		// item
		else if(reader.name() == "item")
		{
			MetaData md = parse_item(reader);
			if( !md.filepath().isEmpty() && Util::File::isSoundFile(md.filepath()) )
			{
				result << std::move(md);
			}
		} // if(reader.name() == item)

		else
		{
			reader.skipCurrentElement();
		}
	}

	spLog(Log::Info, "Podcast parser") << "Set cover url " << cover_url;
	for(auto it=result.begin(); it != result.end(); it++)
	{
		it->setCoverDownloadUrls({cover_url});
		if(it->artist().isEmpty())
		{
			it->setArtist(author);
		}

		if(it->album().isEmpty())
		{
			it->setAlbum(album);
		}

		it->setGenres(categories);
	}

	return result;
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
					result = parse_channel(reader);
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


