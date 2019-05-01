/* TagExpression.h */

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

#ifndef TAGEXPRESSION_H
#define TAGEXPRESSION_H

#include "Utils/Pimpl.h"

namespace Tagging
{
	using TagString=QString;
	using ReplacedString=QString;

	enum TagName
	{
		TagNone=0,
		TagTitle,
		TagAlbum,
		TagArtist,
		TagTrackNum,
		TagYear,
		TagDisc,
		TagIgnore
	};

	QMap<Tagging::TagName, TagString>	tag_name_map();
	TagString							tag_name_to_string(Tagging::TagName name);
	Tagging::TagName					tag_string_to_name(const TagString& tag_string);

	/**
	 * @brief The TagExpression class
	 * @ingroup Tagging
	 */
	class Expression
	{
		PIMPL(Expression)

	private:
		/**
		 * @brief prepends a "\\" before special characters
		 * @param str string to be modified
		 * @return new string with escaped characters
		 */
		QString escape_special_chars(const QString& str) const;


		/**
		 * @brief calcs the regular expression string
		 * @param splitted_tag_str a stringlist gennerated by split_tag_string
		 * @return the regular expression string
		 */
		QString calc_regex_string(const QStringList& splitted_tag_str) const;


		/**
		 * @brief splits the tag string into normal string and tags e.g. foo<t>bar -> (foo, <t>, bar)
		 * @param tag_str
		 * @return a stringlist containing normal strings and tags
		 */
		QStringList split_tag_string(const QString& tag_str) const;


		/**
		 * @brief fills the captured_texts
		 * @param tag_str the tag string entered in UI
		 * @param filepath the filepath
		 * @return true if regular expressions can be applied to filepath, false else
		 */
		bool update_tag(const QString& tag_str, const QString& filepath);


	public:
		Expression()=delete;
		Expression(const QString& tag_str, const QString& filename);
		virtual ~Expression();

		QMap<Tagging::TagName, QString> captured_tags() const;
		bool is_valid() const;
	};
}

#endif // TAGEXPRESSION_H
