/* TagExpression.h */

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

#ifndef TAGEXPRESSION_H
#define TAGEXPRESSION_H

#include "Utils/Pimpl.h"

class MetaData;

namespace Tagging
{
	using TagString = QString;
	using ReplacedString = QString;

	enum TagName
	{
		TagNone = 0,
		TagTitle,
		TagAlbum,
		TagArtist,
		TagTrackNum,
		TagYear,
		TagDisc,
		TagIgnore
	};

	QString tagNameToString(TagName tagName);

	/**
	 * @brief The TagExpression class
	 * @ingroup Tagging
	 */
	class Expression
	{
		PIMPL(Expression)

		private:
			bool updateTag(const QString& lineEditString, const QString& filepath);

		public:
			Expression() = delete;
			Expression(const QString& tagString, const QString& filepath);
			~Expression();

			QMap<Tagging::TagName, QString> capturedTags() const;
			bool isValid() const;

			bool apply(MetaData& track) const;
	};
}

#endif // TAGEXPRESSION_H
