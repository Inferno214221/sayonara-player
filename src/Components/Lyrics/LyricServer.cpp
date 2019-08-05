/* LyricServer.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

using namespace Lyrics;

struct Server::Private
{
	QString name;
	QString address;
	Server::Replacements replacements;
	QString direct_url_template;
	Server::StartEndTags start_end_tag;
	bool is_start_tag_included=false;
	bool is_end_tag_included=false;
	bool is_numeric=false;
	bool is_lowercase=false;
	QString error_string;

	QString search_result_regex;
	QString search_result_url_template;
	QString search_url_template;
};

Server::Server()
{
	m = Pimpl::make<Private>();
}

Server::~Server() = default;

bool Server::can_fetch_directly() const
{
	return !(m->address.isEmpty() || m->direct_url_template.isEmpty() || m->start_end_tag.isEmpty());
}

bool Server::can_search() const
{
	return !(m->address.isEmpty() || m->search_result_regex.isEmpty() || m->search_result_url_template.isEmpty() || m->search_url_template.isEmpty());
}

QString Server::name() const
{
	return m->name;
}

void Server::set_name(const QString& name)
{
	m->name = name;
}

QString Server::address() const
{
	return m->address;
}

void Server::set_address(const QString& address)
{
	m->address = address;
}

Server::Replacements Server::replacements() const
{
	return m->replacements;
}

void Server::set_replacements(const Server::Replacements& replacements)
{
	m->replacements = replacements;
}

QString Server::direct_url_template() const
{
	return m->direct_url_template;
}

void Server::set_direct_url_template(const QString& direct_url_template)
{
	m->direct_url_template = direct_url_template;
}

Server::StartEndTags Server::start_end_tag() const
{
	return m->start_end_tag;
}

void Server::set_start_end_tag(const StartEndTags& start_end_tag)
{
	m->start_end_tag = start_end_tag;
}

bool Server::is_start_tag_included() const
{
	return m->is_start_tag_included;
}

void Server::set_is_start_tag_included(bool is_start_tag_included)
{
	m->is_start_tag_included = is_start_tag_included;
}

bool Server::is_end_tag_included() const
{
	return m->is_end_tag_included;
}

void Server::set_is_end_tag_included(bool is_end_tag_included)
{
	m->is_end_tag_included = is_end_tag_included;
}

bool Server::is_numeric() const
{
	return m->is_numeric;
}

void Server::set_is_numeric(bool is_numeric)
{
	m->is_numeric = is_numeric;
}

bool Server::is_lowercase() const
{
	return m->is_lowercase;
}

void Server::set_is_lowercase(bool is_lowercase)
{
	m->is_lowercase = is_lowercase;
}

QString Server::error_string() const
{
	return m->error_string;
}

void Server::set_error_string(const QString& error_string)
{
	m->error_string = error_string;
}

QString Server::search_result_regex() const
{
	return m->search_result_regex;
}

void Server::set_search_result_regex(const QString& search_result_regex)
{
	m->search_result_regex = search_result_regex;
}

QString Server::search_result_url_template() const
{
	return m->search_result_url_template;
}

void Server::set_search_result_url_template(const QString& search_result_url_template)
{
	m->search_result_url_template = search_result_url_template;
}

QString Server::search_url_template() const
{
	return m->search_url_template;
}

QJsonObject Server::to_json()
{
	return Lyrics::ServerJsonWriter::to_json(this);
}

Server* Server::from_json(const QJsonObject& json)
{
	return Lyrics::ServerJsonReader::from_json(json);
}

void Server::set_search_url_template(const QString& search_url_template)
{
	m->search_url_template = search_url_template;
}

QString Server::apply_replacements(const QString& str, const Server::Replacements& replacements)
{
	QString ret(str);

	for(const Server::Replacement& r : replacements)
	{
		while(ret.contains(r.first))
		{
			ret.replace(r.first, r.second);
		}
	}

	return ret;
}

QString Server::apply_replacements(const QString& str) const
{
	return apply_replacements(str, this->replacements());
}
