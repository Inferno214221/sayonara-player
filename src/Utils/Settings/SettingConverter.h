/* SettingConverter.h */

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

#ifndef SETTINGCONVERTER_H
#define SETTINGCONVERTER_H

#include <QPair>
#include <QStringList>
#include "Utils/typedefs.h"

#include <exception>
#include <iostream>

class QSize;
class QString;
class QPoint;

// generic
template<typename T>
/**
 * @brief The SettingConverter class
 * @ingroup Settings
 */
class SettingConverter
{
public:
	static QString cvt_to_string(const T& val){
		return val.toString();
	}

	static bool cvt_from_string(const QString& val, T& ret){
		try {
			ret = T::fromString(val);
			return true;
		}
		catch(std::exception& e)
		{
			std::cerr << e.what() << std::endl;
			return false;
		}
	}
};


// from bool
template<>
/**
 * @brief The SettingConverter<bool> class
 * @ingroup Settings
 */
class SettingConverter<bool>{
public:
	static QString cvt_to_string(const bool& val);
	static bool cvt_from_string(const QString& val, bool& b);
};


// for int

/**
 * @brief The SettingConverter<int> class
 * @ingroup Settings
 */
template<>
class SettingConverter<int>{
public:
	static QString cvt_to_string(const int& val);
	static bool cvt_from_string(const QString& val, int& i);
};

template<>
class SettingConverter<float>{
public:
	static QString cvt_to_string(const float& val);
	static bool cvt_from_string(const QString& val, float& i);
};


// for QStringList
template<>
/**
 * @brief The SettingConverter<QStringList> class
 * @ingroup Settings
 */
class SettingConverter<QStringList>{
public:
	static QString cvt_to_string(const QStringList& val);
	static bool cvt_from_string(const QString& val, QStringList& lst);
};


// for QString
template<>
/**
 * @brief The SettingConverter<QString> class
 * @ingroup Settings
 */
class SettingConverter<QString>{
public:
	static QString cvt_to_string(const QString& val);
	static bool cvt_from_string(const QString& val, QString& b);
};


// for QSize
template<>
/**
 * @brief The SettingConverter<QSize> class
 * @ingroup Settings
 */
class SettingConverter<QSize>{
public:
	static QString cvt_to_string(const QSize& val);
	static bool cvt_from_string(const QString& val, QSize& sz);
};


// for QPoint
template<>
/**
 * @brief The SettingConverter<QPoint> class
 * @ingroup Settings
 */
class SettingConverter<QPoint>{
public:
	static QString cvt_to_string(const QPoint& val);
	static bool cvt_from_string(const QString& val, QPoint& sz);
};


// for QByteArray
template<>
/**
 * @brief The SettingConverter<QByteArray> class
 * @ingroup Settings
 */
class SettingConverter<QByteArray>{
public:
	static QString cvt_to_string(const QByteArray& arr);
	static bool cvt_from_string(const QString& str, QByteArray& arr);
};


// generic for lists
template<typename T>
/**
 * @brief The SettingConverter<QList<T> > class
 * @ingroup Settings
 */
class SettingConverter< QList<T> >{
public:
	static QString cvt_to_string(const QList<T>& val)
	{
		SettingConverter<T> sc;
		QStringList lst;

		for(const T& v : val){
			lst << sc.cvt_to_string(v);
		}

		return lst.join(",");
	}


	static bool cvt_from_string(const QString& val, QList<T>& ret)
	{
		SettingConverter<T> sc;
		ret.clear();
		QStringList lst = val.split(",");

		for(const QString& l : lst)
		{
			T v;
			try {
				if(sc.cvt_from_string(l, v)){
					ret << v;
				}
			} catch (std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
		}

		return true;
	}
};

// generic for lists
template<>
/**
 * @brief The SettingConverter<QList<T> > class
 * @ingroup Settings
 */
class SettingConverter< BoolList >{
public:
	static QString cvt_to_string(const BoolList& val)
	{
		SettingConverter<bool> sc;
		QStringList lst;

		for(const bool& v : val){
			lst << sc.cvt_to_string(v);
		}

		return lst.join(",");
	}


	static bool cvt_from_string(const QString& val, BoolList& ret)
	{
		SettingConverter<bool> sc;
		ret.clear();
		QStringList lst = val.split(",");

		for(const QString& l : lst){
			bool v;
			sc.cvt_from_string(l, v);
			ret.push_back(v);
		}

		return true;
	}
};

template<typename A, typename B>
/**
 * @brief The SettingConverter<QPair<A, B> > class
 * @ingroup Settings
 */
class SettingConverter< QPair<A,B> >{
public:
	static QString cvt_to_string(const QPair<A,B>& val){
		A a = val.first;
		B b = val.second;
		SettingConverter<A> sc_a;
		SettingConverter<B> sc_b;

		return sc_a.cvt_to_string(val.first) + "," + sc_b.cvt_to_string(b);
	}

	static bool cvt_from_string(const QString& val, QPair<A,B>& ret){
		SettingConverter<A> sc_a;
		SettingConverter<B> sc_b;

		QStringList lst = val.split(",");
		QString a, b;
		bool success = true;
		if(lst.size() > 0){
			a = lst[0];
		}

		if(lst.size() > 1){
			b = lst[1];
		}
		else
		{
			success = false;
		}

		sc_a.cvt_from_string (a, ret.first);
		sc_b.cvt_from_string (b, ret.second);

		return success;
	}
};

#endif // SETTINGCONVERTER_H
