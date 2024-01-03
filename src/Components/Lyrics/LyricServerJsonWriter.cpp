/* LyricServerJsonWriter.cpp */

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

#include "LyricServer.h"
#include "LyricServerJsonWriter.h"

#include "Utils/FileUtils.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>

using Lyrics::Server;

namespace
{
	Server::Replacement getReplacement(const QJsonObject& object)
	{
		Server::Replacement replacement;

		for(auto it = object.begin(); it != object.end(); it++)
		{
			if(it.key() == "replacement_from")
			{
				replacement.first = it->toString();
			}

			else if(it.key() == "replacement_to")
			{
				replacement.second = it->toString();
			}
		}

		return replacement;
	}

	Server::Replacements getReplacements(const QJsonArray& array)
	{
		Server::Replacements replacements;

		for(auto it = array.begin(); it != array.end(); it++)
		{
			const auto replacement = getReplacement(it->toObject());
			if(!replacement.first.isEmpty())
			{
				replacements << replacement;
			}
		}

		return replacements;
	}

	Server::StartEndTag getStartEndTag(const QJsonObject& object)
	{
		Server::StartEndTag startEndTag;

		for(auto it = object.begin(); it != object.end(); it++)
		{
			if(it.key() == "start_tag")
			{
				startEndTag.first = it->toString();
			}

			else if(it.key() == "end_tag")
			{
				startEndTag.second = it->toString();
			}
		}

		return startEndTag;
	}

	Server::StartEndTags getStartEndTags(const QJsonArray& array)
	{
		Server::StartEndTags startEndTags;

		for(auto it = array.begin(); it != array.end(); it++)
		{
			const auto startEndTag = getStartEndTag(it->toObject());
			if(!startEndTag.first.isEmpty() && !startEndTag.second.isEmpty())
			{
				startEndTags << startEndTag;
			}
		}

		return startEndTags;
	}

	QJsonArray replacementsToArray(const Server::Replacements& replacements)
	{
		QJsonArray result;

		for(const auto& replacement: replacements)
		{
			QJsonObject replacementObject;
			replacementObject.insert("replacement_from", QJsonValue(replacement.first));
			replacementObject.insert("replacement_to", QJsonValue(replacement.second));

			result.append(QJsonValue(replacementObject));
		}

		return result;
	}

	QJsonArray startEndTagsToArray(const Server::StartEndTags& startEndTags)
	{
		QJsonArray result;

		for(const auto& startEndTag: startEndTags)
		{
			QJsonObject startEndTagObject;
			startEndTagObject.insert("start_tag", QJsonValue(startEndTag.first));
			startEndTagObject.insert("end_tag", QJsonValue(startEndTag.second));

			result.append(QJsonValue(startEndTagObject));
		}

		return result;
	}
}

QJsonObject Lyrics::ServerJsonWriter::toJson(Lyrics::Server* server)
{
	QJsonObject object;

	object.insert("name", QJsonValue(server->name()));
	object.insert("address", QJsonValue(server->address()));
	object.insert("direct_url_template", QJsonValue(server->directUrlTemplate()));
	object.insert("is_start_tag_included", QJsonValue(server->isStartTagIncluded()));
	object.insert("is_end_tag_included", QJsonValue(server->isEndTagIncluded()));
	object.insert("is_numeric", QJsonValue(server->isNumeric()));
	object.insert("is_lowercase", QJsonValue(server->isLowercase()));
	object.insert("error_string", QJsonValue(server->errorString()));

	const auto arrayReplacements = replacementsToArray(server->replacements());
	object.insert("replacements", QJsonValue(arrayReplacements));

	const auto arrStartEndTags = startEndTagsToArray(server->startEndTag());
	object.insert("start_end_tags", QJsonValue(arrStartEndTags));

	object.insert("search_result_regex", QJsonValue(server->searchResultRegex()));
	object.insert("search_result_url_template", QJsonValue(server->searchResultUrlTemplate()));
	object.insert("search_url_template", QJsonValue(server->searchUrlTemplate()));

	return object;
}

Lyrics::Server* Lyrics::ServerJsonReader::fromJson(const QJsonObject& json)
{
	auto* server = new Server();

	for(auto it = json.begin(); it != json.end(); it++)
	{
		const auto key = it.key();
		if(key == "name")
		{
			server->setName(it->toString());
		}

		else if(key == "address")
		{
			server->setAddress(it->toString());
		}

		else if(key == "replacements")
		{
			server->setReplacements(getReplacements(it->toArray()));
		}

		else if(key == "direct_url_template")
		{
			server->setDirectUrlTemplate(it->toString());
		}

		else if(key == "start_end_tags")
		{
			server->setStartEndTag(getStartEndTags(it->toArray()));
		}

		else if(key == "is_start_tag_included")
		{
			server->setIsStartTagIncluded(it->toBool());
		}

		else if(key == "is_end_tag_included")
		{
			server->setIsEndTagIncluded(it->toBool());
		}

		else if(key == "is_numeric")
		{
			server->setIsNumeric(it->toBool());
		}

		else if(key == "is_lowercase")
		{
			server->setIsLowercase(it->toBool());
		}

		else if(key == "error_string")
		{
			server->setErrorString(it->toString());
		}

		else if(key == "search_result_regex")
		{
			server->setSearchResultRegex(it->toString());
		}

		else if(key == "search_result_url_template")
		{
			server->setSearchResultUrlTemplate(it->toString());
		}

		else if(key == "search_url_template")
		{
			server->setSearchUrlTemplate(it->toString());
		}
	}

	if(server->name().isEmpty() || server->address().isEmpty())
	{
		delete server;
		return nullptr;
	}

	return server;
}

QList<Lyrics::Server*> Lyrics::ServerJsonReader::parseJsonFile(const QString& filename)
{
	QList<Server*> ret;
	QByteArray data;
	Util::File::readFileIntoByteArray(filename, data);

	const auto doc = QJsonDocument::fromJson(data);
	if(doc.isArray())
	{
		const auto arr = doc.array();
		for(auto it = arr.begin(); it != arr.end(); it++)
		{
			if(auto* server = fromJson(it->toObject()); server != nullptr)
			{
				ret << server;
			}
		}
	}

	else if(doc.isObject())
	{
		if(auto* server = fromJson(doc.object()); server != nullptr)
		{
			ret << server;
		}
	}

	return ret;
}
