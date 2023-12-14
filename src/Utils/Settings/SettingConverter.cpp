/* SettingConverter.cpp */

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

#include "SettingConverter.h"
#include "SettingConvertible.h"

#include "Utils/Algorithm.h"

#include <QStringList>
#include <QSize>
#include <QPoint>

#include <optional>

namespace
{
	auto stringToIntConverter = [](const auto& str) -> std::optional<int> {
		bool ok = false;
		auto i = str.toInt(&ok);

		return ok ? std::optional {i} : std::nullopt;
	};

	template<typename A, typename B>
	QString pairToString(const A& a, const B& b) { return QString("%1,%2").arg(a).arg(b); }

	template<typename A>
	std::optional<std::pair<A, A>>
	stringToPair(const QString& str, std::function<std::optional<A>(QString)>&& converter)
	{
		const auto splitted = str.split(",");
		if(splitted.count() < 2)
		{
			return std::nullopt;
		}

		const auto a = converter(splitted[0]);
		const auto b = converter(splitted[1]);

		return (a.has_value() && b.has_value())
		       ? std::optional {std::make_pair(a.value(), b.value())}
		       : std::nullopt;
	}
}

QString SettingConverter::toString(const bool& val) { return val ? "true" : "false"; }

bool SettingConverter::fromString(const QString& val, bool& b)
{
	b = (val.toLower() == "true") || (val.toInt() > 0);

	return true;
}

QString SettingConverter::toString(const int& val) { return QString::number(val); }

bool SettingConverter::fromString(const QString& val, int& i)
{
	bool ok;
	i = val.toInt(&ok);

	return ok;
}

QString SettingConverter::toString(const float& val) { return QString::number(val); }

bool SettingConverter::fromString(const QString& val, float& i)
{
	bool ok;
	i = val.toFloat(&ok);

	return ok;
}

QString SettingConverter::toString(const QStringList& val) { return val.join(","); }

bool SettingConverter::fromString(const QString& val, QStringList& lst)
{
	lst = val.split(",");
	return true;
}

QString SettingConverter::toString(const QString& val) { return val; }

bool SettingConverter::fromString(const QString& val, QString& b)
{
	b = val;
	return true;
}

QString SettingConverter::toString(const QSize& val) { return pairToString(val.width(), val.height()); }

bool SettingConverter::fromString(const QString& val, QSize& sz)
{
	const auto result = stringToPair<int>(val, stringToIntConverter);
	if(result.has_value())
	{
		sz = QSize {result->first, result->second};
	}

	return result.has_value();
}

QString SettingConverter::toString(const QPoint& val) { return pairToString(val.x(), val.y()); }

bool SettingConverter::fromString(const QString& val, QPoint& point)
{
	const auto result = stringToPair<int>(val, stringToIntConverter);
	if(result.has_value())
	{
		point = QPoint {result->first, result->second};
	}

	return result.has_value();
}

QString SettingConverter::toString(const QByteArray& arr)
{
	QStringList numbers;
	Util::Algorithm::transform(arr, numbers, [](const auto& byte) {
		return QString::number(byte);
	});

	return numbers.join(",");
}

bool SettingConverter::fromString(const QString& str, QByteArray& arr)
{
	arr.clear();

	if(str.isEmpty())
	{
		return true;
	}

	const auto numbers = str.split(",");
	for(const auto& numStr: numbers)
	{
		const auto num = numStr.toInt();
		arr.append(char(num));
	}

	return !numbers.empty();
}

QString SettingConverter::toString(const SettingConvertible& t) { return t.toString(); }

bool SettingConverter::fromString(const QString& val, SettingConvertible& t) { return t.loadFromString(val); }
