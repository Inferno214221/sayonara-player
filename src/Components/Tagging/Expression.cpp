/* TagExpression.cpp */

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

#include "Expression.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"

#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QMap>

using namespace Tagging;

struct Expression::Private
{
	QMap<TagName, ReplacedString>	captured_tags;
	QMap<TagName, QString>			tag_regex_map;

	bool							valid;

	Private()
	{
		valid = false;
		tag_regex_map.insert(TagTitle, QString("(.+)"));
		tag_regex_map.insert(TagAlbum, QString ("(.+)"));
		tag_regex_map.insert(TagArtist, QString("(.+)"));
		tag_regex_map.insert(TagTrackNum, QString("(\\d+)"));
		tag_regex_map.insert(TagYear, QString("(\\d{4})"));
		tag_regex_map.insert(TagDisc, QString("(\\d{1,3})"));
		tag_regex_map.insert(TagIgnore, QString("(.+)"));
	}
};

Expression::Expression(const QString& tag_str, const QString& filepath)
{
	m = Pimpl::make<Private>();
	m->valid = update_tag(tag_str, filepath);
}

Expression::~Expression() {}


QMap<TagName, QString> Expression::captured_tags() const
{
	return m->captured_tags;
}

bool Expression::is_valid() const
{
	return m->valid;
}


QString Expression::escape_special_chars(const QString& str) const
{
	QString s = str;
	const QStringList str2escape
	{
		QStringLiteral("\\"),
		QStringLiteral("?"),
		QStringLiteral("+"),
		QStringLiteral("*"),
		QStringLiteral("["),
		QStringLiteral("]"),
		QStringLiteral("("),
		QStringLiteral(")"),
		QStringLiteral("{"),
		QStringLiteral("}"),
		QStringLiteral(".")
	};

	for(const QString& c : str2escape)
	{
		s.replace(c, QString("\\") + c);
	}

	return s;
}

QStringList Expression::split_tag_string( const QString& line_edit_str ) const
{
	// split the line edit: Write strings not covered by tags and tags
	// into the return value

	QString line_edit_escaped = escape_special_chars(line_edit_str);

	using IndexStringMap=QMap<int, TagString>;
	IndexStringMap index_string_map;

	const QStringList available_tags
	{
		tag_name_to_string(TagTitle),
		tag_name_to_string(TagAlbum),
		tag_name_to_string(TagArtist),
		tag_name_to_string(TagTrackNum),
		tag_name_to_string(TagYear),
		tag_name_to_string(TagDisc)
	};

	// search for the tags in tag_str and save the combination
	// Index and TagString into index_string_map
	for(const TagString& tag : available_tags)
	{
		int idx = line_edit_escaped.indexOf(tag);
		if(idx >= 0)
		{
			index_string_map.insert(idx, tag);
		}
	}


	// split the string and fill splitted_tag_str with
	// non-tags and tags
	int cur_idx = 0;
	QStringList splitted_tag_str;
	for(auto it=index_string_map.cbegin(); it != index_string_map.cend(); it++)
	{
		int idx = it.key();
		TagString tag_string = it.value();

		int len = idx - cur_idx;

		QString str_until_tag = line_edit_escaped.mid(cur_idx, len);
		if(!str_until_tag.isEmpty())
		{
			splitted_tag_str << str_until_tag;
		}

		splitted_tag_str << tag_string;

		cur_idx += (tag_string.size() + len);
	}

	// rest of the line
	QString rest_of_line = line_edit_escaped.right(line_edit_escaped.length() - cur_idx);
	if(!rest_of_line.isEmpty())
	{
		splitted_tag_str << rest_of_line;
	}

	return splitted_tag_str;
}


QString Expression::calc_regex_string(const QStringList& splitted_str) const
{
	QString regex;

	for(const QString& s : splitted_str)
	{
		if(s.isEmpty()) {
			continue;
		}

		TagName tag_name = tag_string_to_name(s);
		if( m->tag_regex_map.contains(tag_name) )
		{
			regex += m->tag_regex_map[tag_name];
		}

		else
		{
			// write a non-tag string into parenthesis to
			// trigger the capturing of the regex
			regex += "(" + s + ")";
		}
	}

	return regex;
}


bool Expression::update_tag(const QString& line_edit_str, const QString& filepath)
{
	m->captured_tags.clear();

	// create regular expression out of tag_str
	QStringList splitted_tag_str = split_tag_string(line_edit_str);
	QString regex =	calc_regex_string(splitted_tag_str);
	QRegExp re(regex);

	// save content of all entered tags and the rest into captured texts
	re.indexIn( filepath );

	QStringList captured_texts = re.capturedTexts();
	captured_texts.removeAt(0);
	captured_texts.removeAll("");

	int n_caps = captured_texts.size();
	int n_tags = splitted_tag_str.size();

	bool valid = (n_caps == n_tags);

	if( !valid )
	{
		sp_log(Log::Warning, this) << regex;
		sp_log(Log::Warning, this) <<  n_caps << " tags found, but requested " << n_tags;
		sp_log(Log::Warning, this) << "Caps: ";
		sp_log(Log::Warning, this) << "";

		for(const QString& s : ::Util::AsConst(captured_texts))
		{
			sp_log(Log::Warning, this) << "  " << s;
		}

		sp_log(Log::Warning, this) << "";

		return false;
	}

	for(int i=0; i<n_caps; i++)
	{
		QString splitted = splitted_tag_str[i]; // the original out of line edit
		QString captured = captured_texts[i]; // maybe replaced by the content of a tag

		if(i==0)
		{
			QString dir, filename;
			Util::File::split_filename(captured, dir, filename);
			captured = filename;
		}

		TagName tag_name = Tagging::tag_string_to_name(splitted);
		if(tag_name != TagName::TagNone)
		{
			m->captured_tags[tag_name] = captured;
		}
	}

	return true;
}


QMap<TagName, TagString> Tagging::tag_name_map()
{
	QMap<TagName, TagString> map =
	{
		{TagNone, QString()},
		{TagTitle, "<title>"},
		{TagAlbum, "<album>"},
		{TagArtist, "<artist>"},
		{TagTrackNum, "<tracknum>"},
		{TagYear, "<year>"},
		{TagDisc, "<disc>"},
		{TagIgnore, "<ignore>"},
	};

	return map;
}


TagString Tagging::tag_name_to_string(TagName name)
{
	QMap<TagName, TagString> map = tag_name_map();
	return map[name];
}

TagName Tagging::tag_string_to_name(const TagString& tag_string)
{
	QMap<TagName, TagString> map = tag_name_map();
	return map.key(tag_string, TagNone);
}

