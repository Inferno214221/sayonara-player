/* SettingConverter.h */

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

#ifndef SAYONARA_PLAYER_SETTINGCONVERTER_H
#define SAYONARA_PLAYER_SETTINGCONVERTER_H

#include "Utils/typedefs.h"

#include <QPair>
#include <QStringList>

#include <exception>
#include <iostream>

class QSize;
class QPoint;
class SettingConvertible;

namespace SettingConverter
{
	QString toString(const SettingConvertible& t);
	bool fromString(const QString& val, SettingConvertible& t);

	QString toString(const bool& val);
	bool fromString(const QString& val, bool& b);

	QString toString(const int& val);
	bool fromString(const QString& val, int& i);

	template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
	QString toString(const T& val)
	{
		int i = static_cast<int>(val);
		return toString(i);
	}

	template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
	bool fromString(const QString& val, T& e)
	{
		int i = 0;
		const auto b = fromString(val, i);

		e = static_cast<T>(i);
		return b;
	}

	QString toString(const float& val);
	bool fromString(const QString& val, float& i);

	QString toString(const QStringList& val);
	bool fromString(const QString& val, QStringList& lst);

	QString toString(const QString& val);
	bool fromString(const QString& val, QString& b);

	QString toString(const QSize& val);
	bool fromString(const QString& val, QSize& sz);

	QString toString(const QPoint& val);
	bool fromString(const QString& val, QPoint& sz);

	QString toString(const QByteArray& arr);
	bool fromString(const QString& str, QByteArray& arr);

	template<typename A, typename B>
	QString toString(const QPair<A, B>& arr)
	{
		return toString(arr.first) + "," + toString(arr.second);
	}

	template<typename A, typename B>
	bool fromString(const QString& str, QPair<A, B>& pair)
	{
		QStringList lst = str.split(",");

		if(lst.size() >= 2)
		{
			fromString(lst[0], pair.first);
			fromString(lst[1], pair.second);
		}

		return (lst.size() >= 2);
	}

	template<typename T>
	QString toString(const QList<T>& val)
	{
		QStringList lst;

		for(const T& v: val)
		{
			lst << toString(v);
		}

		return lst.join(",");
	}

	template<typename T>
	bool fromString(const QString& val, QList<T>& ret)
	{
		ret.clear();
		QStringList lst = val.split(",");

		for(const auto& item: lst)
		{
			try
			{
				T v;
				if(fromString(item, v))
				{
					ret << v;
				}
			} catch(std::exception& e)
			{
				std::cerr << e.what() << std::endl;
			}
		}

		return true;
	}
}

#endif // SAYONARA_PLAYER_SETTINGCONVERTER_H
