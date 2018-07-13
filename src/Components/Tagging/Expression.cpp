/* TagExpression.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
	/**
	 * @brief m->cap_map contains the Tag and the text that fits the regular expression
	 */
	QMap<Tag, ReplacedString>	cap_map;

	/**
	 * @brief m->tag_regex_map keys = the tag (e.g. <t>) and the corresponding regular expression
	 */
	QMap<Tag, QString>		tag_regex_map;
	bool					valid;

	Private()
	{
		valid = false;
		tag_regex_map.insert(TAG_TITLE, QString("(.+)"));
		tag_regex_map.insert(TAG_ALBUM, QString ("(.+)"));
		tag_regex_map.insert(TAG_ARTIST, QString("(.+)"));
		tag_regex_map.insert(TAG_TRACK_NUM, QString("(\\d+)"));
		tag_regex_map.insert(TAG_YEAR, QString("(\\d{4})"));
		tag_regex_map.insert(TAG_DISC, QString("(\\d{1,3})"));
		tag_regex_map.insert(TAG_IGNORE, QString("(.+)"));
	}
};

Expression::Expression()
{
	m = Pimpl::make<Private>();
}

Expression::Expression(const QString& tag_str, const QString& filepath) :
	Expression()
{
	update_tag(tag_str, filepath);
}

Expression::~Expression() {}

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

QStringList Expression::split_tag_string( const QString& tag_str ) const
{
	QStringList splitted_tag_str;
	QString tag_str_tmp = escape_special_chars(tag_str);

	QMap<int, QString> tags;

	QStringList available_tags;
	available_tags  << TAG_TITLE
					<< TAG_ALBUM
					<< TAG_ARTIST
					<< TAG_TRACK_NUM
					<< TAG_YEAR
					<< TAG_DISC;


	// search for the tags in tag_str
	for(const QString& tag : ::Util::AsConst(available_tags))
	{
		int idx = tag_str_tmp.indexOf(tag);
		if(idx >= 0){
			tags.insert(idx, tag);
		}
	}

	int cur_idx = 0;

	for(auto it=tags.cbegin(); it != tags.cend(); it++)
	{
		int idx = it.key();
		QString value = it.value();

		int len = idx - cur_idx;

		splitted_tag_str << tag_str_tmp.mid(cur_idx, idx - cur_idx);
		splitted_tag_str << value;

		cur_idx += (value.size() + len);
	}

	splitted_tag_str << tag_str_tmp.right(tag_str_tmp.length() - cur_idx);
	splitted_tag_str.removeAll("");

	return splitted_tag_str;
}


QString Expression::calc_regex_string(const QStringList& splitted_str) const
{
	QString re_str;

	for(const QString& s : splitted_str){
		if(s.isEmpty()) continue;


		if( m->tag_regex_map.contains(s) ){
			re_str += m->tag_regex_map[s];
		}

		else{
			re_str += "(" + s + ")";
		}
	}

	return re_str;
}


bool Expression::update_tag(const QString& tag_str, const QString& filepath)
{
	m->cap_map.clear();

	QStringList splitted_tag_str = split_tag_string(tag_str);
	QString regex = calc_regex_string(splitted_tag_str);

	QRegExp re( regex );

	re.indexIn( filepath );

	QStringList captured_texts = re.capturedTexts();
	captured_texts.removeAt(0);
	captured_texts.removeAll("");

	int n_caps = captured_texts.size();
	int n_tags = splitted_tag_str.size();

	bool valid = (n_caps == n_tags);

	if( !valid ){
		sp_log(Log::Warning) << regex;
		sp_log(Log::Warning) <<  n_caps << " tags found, but requested " << n_tags;
		sp_log(Log::Warning) << "Caps: ";
		sp_log(Log::Warning) << "";

		for(const QString& s : ::Util::AsConst(captured_texts))
		{
			sp_log(Log::Warning) << "  " << s;
		}

		sp_log(Log::Warning) << "";
	}

	else
	{
		for(int i=0; i<n_caps; i++)
		{
			Tag tag = splitted_tag_str[i];
			QString cap = captured_texts[i];

			if(i==0){
				QString dir, filename;
				Util::File::split_filename(cap, dir, filename);
				cap = filename;
			}


			m->cap_map[tag] = cap;
		}
	}

	return valid;
}

bool Expression::check_tag(const Tag& tag, const QString& str)
{
	if(!m->tag_regex_map.contains(tag)) {
		return false;
	}

	QString escape_str = escape_special_chars(str);
	QRegExp re(m->tag_regex_map[tag]);

	if(re.indexIn(escape_str) != 0 ) {
		return false;
	}

	return true;
}

QMap<Tag, ReplacedString> Expression::get_tag_val_map() const
{
	return m->cap_map;
}
