/* LyricServer.cpp */

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
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

namespace Lyrics
{
	struct Server::Private
	{
		QString name;
		QString address;
		Server::Replacements replacements;
		QString directUrlTemplate;
		Server::StartEndTags startEndTag;
		bool isStartTagIncluded = false;
		bool isEndTagIncluded = false;
		bool isNumeric = false;
		bool isLowercase = false;
		QString errorString;

		QString searchResultRegex;
		QString searchResultUrlTemplate;
		QString searchUrlTemplate;
	};

	Server::Server()
	{
		m = Pimpl::make<Private>();
	}

	Server::~Server() = default;

	bool Server::canFetchDirectly() const
	{
		return !(m->address.isEmpty() || m->directUrlTemplate.isEmpty() || m->startEndTag.isEmpty());
	}

	bool Server::canSearch() const
	{
		return !(m->address.isEmpty() || m->searchResultRegex.isEmpty() || m->searchResultUrlTemplate.isEmpty() ||
		         m->searchUrlTemplate.isEmpty());
	}

	QString Server::name() const
	{
		return m->name;
	}

	void Server::setName(const QString& name)
	{
		m->name = name;
	}

	QString Server::address() const
	{
		return m->address;
	}

	void Server::setAddress(const QString& address)
	{
		m->address = address;
	}

	Server::Replacements Server::replacements() const
	{
		return m->replacements;
	}

	void Server::setReplacements(const Server::Replacements& replacements)
	{
		m->replacements = replacements;
	}

	QString Server::directUrlTemplate() const
	{
		return m->directUrlTemplate;
	}

	void Server::setDirectUrlTemplate(const QString& direct_url_template)
	{
		m->directUrlTemplate = direct_url_template;
	}

	Server::StartEndTags Server::startEndTag() const
	{
		return m->startEndTag;
	}

	void Server::setStartEndTag(const StartEndTags& start_end_tag)
	{
		m->startEndTag = start_end_tag;
	}

	bool Server::isStartTagIncluded() const
	{
		return m->isStartTagIncluded;
	}

	void Server::setIsStartTagIncluded(bool is_start_tag_included)
	{
		m->isStartTagIncluded = is_start_tag_included;
	}

	bool Server::isEndTagIncluded() const
	{
		return m->isEndTagIncluded;
	}

	void Server::setIsEndTagIncluded(bool isEndTagIncluded)
	{
		m->isEndTagIncluded = isEndTagIncluded;
	}

	bool Server::isNumeric() const
	{
		return m->isNumeric;
	}

	void Server::setIsNumeric(bool isNumeric)
	{
		m->isNumeric = isNumeric;
	}

	bool Server::isLowercase() const
	{
		return m->isLowercase;
	}

	void Server::setIsLowercase(bool isLowercase)
	{
		m->isLowercase = isLowercase;
	}

	QString Server::errorString() const
	{
		return m->errorString;
	}

	void Server::setErrorString(const QString& errorString)
	{
		m->errorString = errorString;
	}

	QString Server::searchResultRegex() const
	{
		return m->searchResultRegex;
	}

	void Server::setSearchResultRegex(const QString& searchResultRegex)
	{
		m->searchResultRegex = searchResultRegex;
	}

	QString Server::searchResultUrlTemplate() const
	{
		return m->searchResultUrlTemplate;
	}

	void Server::setSearchResultUrlTemplate(const QString& searchResultUrlTemplate)
	{
		m->searchResultUrlTemplate = searchResultUrlTemplate;
	}

	QString Server::searchUrlTemplate() const
	{
		return m->searchUrlTemplate;
	}

	void Server::setSearchUrlTemplate(const QString& searchUrlTemplate)
	{
		m->searchUrlTemplate = searchUrlTemplate;
	}

	QJsonObject Server::toJson()
	{
		return Lyrics::ServerJsonWriter::toJson(this);
	}

	Server* Server::fromJson(const QJsonObject& json)
	{
		return Lyrics::ServerJsonReader::fromJson(json);
	}

	QString Server::applyReplacements(const QString& str) const
	{
		return applyReplacements(str, this->replacements());
	}

	QString Server::applyReplacements(const QString& str, const Lyrics::Server::Replacements& replacements)
	{
		auto result = str;

		for(const auto& replacement : replacements)
		{
			while(result.contains(replacement.first))
			{
				result.replace(replacement.first, replacement.second);
			}
		}

		return result;
	}
}