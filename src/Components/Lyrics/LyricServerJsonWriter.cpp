/* LyricServerJsonWriter.cpp */

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



#include "LyricServer.h"
#include "LyricServerJsonWriter.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>

using Lyrics::Server;

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

	QJsonArray arr_replacements;
	auto replacements = server->replacements();
	for(const Server::Replacement& replacement : replacements)
	{
		QJsonObject replacement_object;
		replacement_object.insert("replacement_from", QJsonValue(replacement.first));
		replacement_object.insert("replacement_to", QJsonValue(replacement.second));

		arr_replacements.append(QJsonValue(replacement_object));
	}

	object.insert("replacements", QJsonValue(arr_replacements));

	QJsonArray arr_start_end_tags;
	auto start_end_tags = server->startEndTag();
	for(const Server::StartEndTag& start_end_tag : start_end_tags)
	{
		QJsonObject start_end_tag_object;
		start_end_tag_object.insert("start_tag", QJsonValue(start_end_tag.first));
		start_end_tag_object.insert("end_tag", QJsonValue(start_end_tag.second));

		arr_start_end_tags.append(QJsonValue(start_end_tag_object));
	}

	object.insert("start_end_tags", QJsonValue(arr_start_end_tags));

	object.insert("search_result_regex", QJsonValue(server->searchResultRegex()));
	object.insert("search_result_url_template", QJsonValue(server->searchResultUrlTemplate()));
	object.insert("search_url_template", QJsonValue(server->searchUrlTemplate()));

	return object;
}


Lyrics::Server* Lyrics::ServerJsonReader::fromJson(const QJsonObject& json)
{
	Server* server = new Server();

	for(auto it=json.begin(); it != json.end(); it++)
	{
		QString key = it.key();
		QJsonValue val = *it;

		if(key == "name")
		{
			server->setName(val.toString());
		}

		else if(key == "address")
		{
			server->setAddress(val.toString());
		}

		else if(key == "replacements")
		{
			Server::Replacements replacements;
			QJsonArray arr = val.toArray();
			for(auto arr_it=arr.begin(); arr_it != arr.end(); arr_it++)
			{

				Server::Replacement replacement;

				QJsonObject obj = (*arr_it).toObject();
				for(auto obj_it=obj.begin(); obj_it != obj.end(); obj_it++)
				{
					if(obj_it.key() == "replacement_from")
					{
						replacement.first = (*obj_it).toString();
					}

					else if(obj_it.key() == "replacement_to")
					{
						replacement.second = (*obj_it).toString();
					}
				}

				if(!replacement.first.isEmpty()){
					replacements << replacement;
				}
			}

			server->setReplacements(replacements);
		}

		else if(key == "direct_url_template")
		{
			server->setDirectUrlTemplate(val.toString());
		}

		else if(key == "start_end_tags")
		{
			Server::StartEndTags start_end_tags;
			QJsonArray arr = val.toArray();
			for(auto arr_it=arr.begin(); arr_it != arr.end(); arr_it++)
			{
				Server::StartEndTag start_end_tag;

				QJsonObject obj = (*arr_it).toObject();
				for(auto obj_it=obj.begin(); obj_it != obj.end(); obj_it++)
				{
					if(obj_it.key() == "start_tag")
					{
						start_end_tag.first = (*obj_it).toString();
					}

					else if(obj_it.key() == "end_tag")
					{
						start_end_tag.second = (*obj_it).toString();
					}
				}

				if(!start_end_tag.first.isEmpty() && !start_end_tag.second.isEmpty()){
					start_end_tags << start_end_tag;
				}
			}

			server->setStartEndTag(start_end_tags);
		}

		else if(key == "is_start_tag_included")
		{
			server->setIsStartTagIncluded(val.toBool());
		}

		else if(key == "is_end_tag_included")
		{
			server->setIsEndTagIncluded(val.toBool());
		}

		else if(key == "is_numeric")
		{
			server->setIsNumeric(val.toBool());
		}

		else if(key == "is_lowercase")
		{
			server->setIsLowercase(val.toBool());
		}

		else if(key == "error_string")
		{
			server->setErrorString(val.toString());
		}

		else if(key == "search_result_regex")
		{
			server->setSearchResultRegex(val.toString());
		}

		else if(key == "search_result_url_template")
		{
			server->setSearchResultUrlTemplate(val.toString());
		}

		else if(key == "search_url_template")
		{
			server->setSearchUrlTemplate(val.toString());
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

	QFile f(filename);
	f.open(QFile::ReadOnly);
	QByteArray data = f.readAll();
	f.close();

	QJsonDocument doc = QJsonDocument::fromJson(data);
	if(doc.isArray())
	{
		QJsonArray arr = doc.array();
		for(auto it=arr.begin(); it != arr.end(); it++)
		{
			Server* server = fromJson( (*it).toObject() );
			if(server){
				ret << server;
			}
		}
	}

	else if(doc.isObject())
	{
		Server* server = fromJson( doc.object() );
		if(server){
			ret << server;
		}
	}

	return ret;
}
