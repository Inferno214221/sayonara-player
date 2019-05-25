/* LyricServer.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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
#include "Utils/Logger/Logger.h"

#define STR_TRUE	QStringLiteral("true")
#define STR_FALSE	QStringLiteral("false")

void ServerTemplate::addReplacement(const QString& rep, const QString& rep_with)
{
	replacements[rep] = rep_with;
}

void ServerTemplate::print_xml() const
{
	sp_log(Log::Info, this) << "<ServerTemplate>";
	sp_log(Log::Info, this) << "  <name>\"" << display_str << "\"</name>";
	sp_log(Log::Info, this) << "  <server_address>\"" << server_address << "\"</server_address>";
	sp_log(Log::Info, this) << "  <call_policy>\"" << call_policy << "\"</call_policy>";
	/*sp_log(Log::Info) << "  <start_tag>\"" << start_tag << "\"</start_tag>";
	sp_log(Log::Info) << "  <end_tag>\"" << end_tag << "\"</end_tag>";*/
	sp_log(Log::Info, this) << "  <include_start_tag>" << (include_start_tag ? STR_TRUE : STR_FALSE) << "</include_start_tag>";
	sp_log(Log::Info, this) << "  <include_end_tag>" << (include_end_tag ? STR_TRUE : STR_FALSE) << "</include_end_tag>";
	sp_log(Log::Info, this) << "  <is_numeric>" << (is_numeric ? STR_TRUE : STR_FALSE) << "</is_numeric>";
	sp_log(Log::Info, this) << "  <to_lower>" << (to_lower ? STR_TRUE : STR_FALSE) << "</to_lower>";
	sp_log(Log::Info, this) << "  <error>\"" << error << "\"</error>";

	for(auto it=replacements.cbegin(); it!=replacements.cend(); it++)
	{
		sp_log(Log::Info, this) << "  <replacement>";
		sp_log(Log::Info, this) << "    <from>\"" << it.key() << "\"</from>";
		sp_log(Log::Info, this) << "    <to>\"" << it.value() << "\"</to>";
		sp_log(Log::Info, this) << "  </replacement>";
	}

	sp_log(Log::Info, this) << "</ServerTemplate>";
}

void ServerTemplate::print_json() const
{
	sp_log(Log::Info, this) << "  {";
	sp_log(Log::Info, this) << "    \"ServerName\": \"" + display_str + "\",";
	sp_log(Log::Info, this) << "    \"ServerAddress\": \"" + server_address + "\",";
	sp_log(Log::Info, this) << "    \"CallPolicy\": \"" + call_policy + "\",";
	sp_log(Log::Info, this) << "    \"IncludeStartTag\": " + QString::number(include_start_tag) + ",";
	sp_log(Log::Info, this) << "    \"IncludeEndTag\": " + QString::number(include_end_tag) + ",";
	sp_log(Log::Info, this) << "    \"IsNumeric\": " + QString::number(is_numeric) + ",";
	sp_log(Log::Info, this) << "    \"ToLower\": " + QString::number(to_lower) + ",";
	sp_log(Log::Info, this) << "    \"Error\": \"" + error + "\",";

	sp_log(Log::Info, this) << "    \"Replacements\": [";

	for(auto it=replacements.cbegin(); it!=replacements.cend(); it++)
	{
		sp_log(Log::Info, this) << "      {";
		sp_log(Log::Info, this) << "        \"OrgString\": \"" + it.key() + "\",";
		sp_log(Log::Info, this) << "        \"RepString\": \"" + it.value() + "\"";
		sp_log(Log::Info, this) << "      },";
	}

	sp_log(Log::Info, this) << "    ]";

	sp_log(Log::Info, this) << "    \"BorderTags\": [";
	for(auto it=start_end_tag.cbegin(); it != start_end_tag.cend(); it++)
	{
		QString key = it.key();
		key.replace("\"", "\\\"");
		QString value = it.value();
		value.replace("\"", "\\\"");

		sp_log(Log::Info, this) << "      {";
		sp_log(Log::Info, this) << "        \"StartTag\": \"" + key + "\",";
		sp_log(Log::Info, this) << "        \"EndTag\": \"" + value + "\"";
		sp_log(Log::Info, this) << "      },";
	}

	sp_log(Log::Info, this) << "    ]";
	sp_log(Log::Info, this) << "  }";
}
