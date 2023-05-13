/* Utils.cpp */
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

#include "Database/Query.h"
#include "Database/Utils.h"
#include "Utils/Ranges.h"
#include "Utils/Library/Filter.h"

#include <QStringList>
#include <QVariant>

namespace
{
	DB::RangeMapping
	convertRangeToMapping(const Util::Range& range, const QString& attribute, const QString& sqlPlaceholderStart,
	                      const QString& sqlPlaceholderEnd)
	{
		DB::RangeMapping result;
		if(range.first == -1)
		{
			return result;
		}

		const auto& [from, to] = range;
		if(from == to)
		{
			result.sqlString = QString("(%1 = %2)")
				.arg(attribute)
				.arg(sqlPlaceholderStart);
			result.mapping[sqlPlaceholderStart] = from;
		}

		else
		{
			result.sqlString = QString("(%1 >= %2 AND %1 <= %3)")
				.arg(attribute)
				.arg(sqlPlaceholderStart)
				.arg(sqlPlaceholderEnd);
			result.mapping[sqlPlaceholderStart] = from;
			result.mapping[sqlPlaceholderEnd] = to;
		}

		return result;
	}

	std::pair<QString, QString> createPlaceholders(const QString& sqlPlaceholderTemplate, Id startId, Id endId)
	{
		const auto sqlPlaceholderStart = QString(":%1from%2")
			.arg(sqlPlaceholderTemplate)
			.arg(startId);

		const auto sqlPlaceholderEnd = QString(":%1to%2")
			.arg(sqlPlaceholderTemplate)
			.arg(endId);

		return std::make_pair(sqlPlaceholderStart, sqlPlaceholderEnd);
	}

	DB::RangeMapping mergeRangeMappings(const QList<DB::RangeMapping>& rangeMappings)
	{
		DB::RangeMapping result;
		QStringList sqlStrings;
		for(const auto& rangeMapping: rangeMappings)
		{
			sqlStrings << rangeMapping.sqlString;
			for(auto it = rangeMapping.mapping.begin(); it != rangeMapping.mapping.end(); it++)
			{
				result.mapping[it.key()] = it.value();
			}
		}

		result.sqlString = sqlStrings.join(" OR ");

		return result;
	}
}

DB::RangeMapping DB::convertRangesToMapping(const Util::RangeList& ranges, const QString& attribute,
                                            const QString& sqlPlaceholderTemplate)
{
	QList<DB::RangeMapping> rangeMappings;
	Util::Algorithm::transform(ranges, rangeMappings, [&](const auto& range) {
		const auto [from, to] = createPlaceholders(sqlPlaceholderTemplate, range.first, range.second);
		return convertRangeToMapping(range, attribute, from, to);
	});

	return mergeRangeMappings(rangeMappings);
}

QString DB::getFilterWhereStatement(const Library::Filter& filter, QString searchPlaceholder)
{
	searchPlaceholder.remove(":");

	switch(filter.mode())
	{
		case Library::Filter::Genre:
			return QString("genreCissearch LIKE :%1").arg(searchPlaceholder);

		case Library::Filter::InvalidGenre:
			return QStringLiteral("genre = ''");

		case Library::Filter::Filename:
			return QString("fileCissearch LIKE :%1").arg(searchPlaceholder);

		case Library::Filter::Fulltext:
		case Library::Filter::Invalid:
		default:
			return QString("allCissearch LIKE :%1").arg(searchPlaceholder);
	}
}

void DB::bindMappingToQuery(DB::Query& query, const DB::RangeMapping& rangeMapping, const QList<Id>& elements)
{
	for(auto it = rangeMapping.mapping.begin(); it != rangeMapping.mapping.end(); it++)
	{
		query.bindValue(it.key(), elements[it.value()]);
	}
}

