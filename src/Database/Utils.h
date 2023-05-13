/* Utils.h */
/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#ifndef SAYONARA_PLAYER_UTILS_H
#define SAYONARA_PLAYER_UTILS_H

#include "Utils/typedefs.h"

#include <QList>
#include <QMap>
#include <QString>
#include <QSqlQuery>

#include <utility>
#include <vector>

namespace Library
{
	class Filter;
}

namespace Util
{
	using Range = std::pair<int, int>;
	using RangeList = std::vector<Range>;
}

namespace DB
{
	struct RangeMapping
	{
		QString sqlString;
		QMap<QString, int> mapping;
	};

	RangeMapping convertRangesToMapping(const Util::RangeList& ranges, const QString& attribute,
	                                    const QString& sqlPlaceholderTemplate);

	void bindMappingToQuery(QSqlQuery& query, const RangeMapping& rangeMapping, const QList<Id>& elements);

	QString getFilterWhereStatement(const ::Library::Filter& filter, QString searchPlaceholder);
}

#endif //SAYONARA_PLAYER_UTILS_H
