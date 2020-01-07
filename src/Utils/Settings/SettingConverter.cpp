/* SettingConverter.cpp */

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

#include "SettingConverter.h"
#include "SettingConvertible.h"

#include <QStringList>
#include <QSize>
#include <QPoint>

/** Bool **/
QString SettingConverter::to_string(const bool& val)
{
	if(val) {
		return QString("true");
	}

	else {
		return QString("false");
	}
}

bool SettingConverter::from_string(const QString& val, bool& b)
{
    b = ((val.compare("true", Qt::CaseInsensitive) == 0) || (val.toInt() > 0));

	return true;
}


/** Integer **/
QString SettingConverter::to_string(const int& val)
{
	return QString::number(val);
}

bool SettingConverter::from_string(const QString& val, int& i)
{
	bool ok;
	i = val.toInt(&ok);

	return ok;
}


/** Floating Point **/
QString SettingConverter::to_string(const float& val)
{
	return QString::number(val);
}

bool SettingConverter::from_string(const QString& val, float& i)
{
	bool ok;
	i = val.toFloat(&ok);

	return ok;
}


/** QStringList **/
QString SettingConverter::to_string(const QStringList& val)
{
	return val.join(",");
}

bool SettingConverter::from_string(const QString& val, QStringList& lst)
{
	lst = val.split(",");
	return true;
}

/** QString **/
QString SettingConverter::to_string(const QString& val)
{
	return val;
}

bool SettingConverter::from_string(const QString& val, QString& b)
{
	b = val;
	return true;
}

/** QSize **/
QString SettingConverter::to_string(const QSize& val)
{
	return QString::number(val.width()) + "," + QString::number(val.height());
}

bool SettingConverter::from_string(const QString& val, QSize& sz)
{
	bool ok;
	int width, height;

	QStringList lst = val.split(",");

	if(lst.size() < 2) return false;

	width = lst[0].toInt(&ok);

	if(!ok) return false;
	height = lst[1].toInt(&ok);
	if(!ok) return false;

	sz.setWidth(width);
	sz.setHeight(height);

	return true;
}

/** QPoint **/
QString SettingConverter::to_string(const QPoint& val)
{
	return QString::number(val.x()) + "," + QString::number(val.y());
}

bool SettingConverter::from_string(const QString& val, QPoint& sz)
{
	bool ok;
	int x, y;

	QStringList lst = val.split(",");

	if(lst.size() < 2) return false;

	x = lst[0].toInt(&ok);

	if(!ok) return false;
	y = lst[1].toInt(&ok);
	if(!ok) return false;

	sz.setX(x);
	sz.setY(y);

	return true;
}

/** QByteArray **/
QString SettingConverter::to_string(const QByteArray& arr)
{
	if(arr.isEmpty()){
		return QString();
	}

	QStringList numbers;
	for(Byte item : arr)
	{
		numbers << QString::number(item);
	}

	return numbers.join(",");
}

bool SettingConverter::from_string(const QString& str, QByteArray& arr)
{
	arr.clear();
	if(str.isEmpty()){
		return true;
	}

	QStringList numbers = str.split(",");

	for(const QString& num_str : numbers)
	{
		int num = num_str.toInt();
		arr.append(char(num));
	}

	return (!numbers.empty());
}

QString SettingConverter::to_string(const SettingConvertible& t)
{
	return t.toString();
}

bool SettingConverter::from_string(const QString& val, SettingConvertible& t)
{
	return t.loadFromString(val);
}
