/* SettingConverter.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef SETTINGCONVERTER_H
#define SETTINGCONVERTER_H

#include "Utils/typedefs.h"

#include <QPair>
#include <QStringList>

#include <exception>
#include <iostream>

class QSize;
class QPoint;
class SettingConvertible;

/**
* @brief The SettingConverter<bool> class
* @ingroup Settings
*/
namespace SettingConverter
{
	QString to_string(const SettingConvertible& t);
	bool from_string(const QString& val, SettingConvertible& t);

	QString to_string(const bool& val);
	bool from_string(const QString& val, bool& b);

	QString to_string(const int& val);
	bool from_string(const QString& val, int& i);

	QString to_string(const float& val);
	bool from_string(const QString& val, float& i);

	QString to_string(const QStringList& val);
	bool from_string(const QString& val, QStringList& lst);

	QString to_string(const QString& val);
	bool from_string(const QString& val, QString& b);

	QString to_string(const QSize& val);
	bool from_string(const QString& val, QSize& sz);

	QString to_string(const QPoint& val);
	bool from_string(const QString& val, QPoint& sz);

	QString to_string(const QByteArray& arr);
	bool from_string(const QString& str, QByteArray& arr);

	template<typename A, typename B>
	QString to_string(const QPair<A, B>& arr)
	{
		return to_string(arr.first) + "," + to_string(arr.second);
	}

	template<typename A, typename B>
	bool from_string(const QString& str, QPair<A, B>& pair)
	{
		QStringList lst = str.split(",");

		if(lst.size() >= 2)
		{
			from_string(lst[0], pair.first);
			from_string(lst[1], pair.second);
		}

		return (lst.size() >= 2);
	}

	template<typename T>
	QString to_string(const QList<T>& val)
	{
		QStringList lst;

		for(const T& v : val) {
			lst << to_string(v);
		}

		return lst.join(",");
	}

	template<typename T>
	bool from_string(const QString& val, QList<T>& ret)
	{
		ret.clear();
		QStringList lst = val.split(",");

		for(const QString& l : lst)
		{
			try
			{
				T v;
				if(from_string(l, v)){
					ret << v;
				}
			} catch (std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
		}

		return true;
	}
}

#endif // SETTINGCONVERTER_H
