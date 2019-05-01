/* SettingConverter.cpp */

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

#include "SettingConverter.h"

#include <QStringList>
#include <QSize>
#include <QPoint>

/** Bool **/
QString SettingConverter<bool>::cvt_to_string(const bool& val)
{
	if(val) {
		return QString("true");
	}

	else {
		return QString("false");
	}
}

bool SettingConverter<bool>::cvt_from_string(const QString& val, bool& b)
{
	if( val.compare("true", Qt::CaseInsensitive) == 0 ||
		val.toInt() > 0)
	{
		b = true;
	}

	else
	{
		b = false;
	}

	return true;
}


/** Integer **/
QString SettingConverter<int>::cvt_to_string(const int& val)
{
	return QString::number(val);
}

bool SettingConverter<int>::cvt_from_string(const QString& val, int& i)
{
	bool ok;
	i = val.toInt(&ok);

	return ok;
}


/** Floating Point **/
QString SettingConverter<float>::cvt_to_string(const float& val)
{
	return QString::number(val);
}

bool SettingConverter<float>::cvt_from_string(const QString& val, float& i)
{
	bool ok;
	i = val.toFloat(&ok);

	return ok;
}


/** QStringList **/
QString SettingConverter<QStringList>::cvt_to_string(const QStringList& val)
{
	return val.join(",");
}

bool SettingConverter<QStringList>::cvt_from_string(const QString& val, QStringList& lst)
{
	lst = val.split(",");
	return true;
}

/** QString **/
QString SettingConverter<QString>::cvt_to_string(const QString& val)
{
	return val;
}

bool SettingConverter<QString>::cvt_from_string(const QString& val, QString& b)
{
	b = val;
	return true;
}

/** QSize **/
QString SettingConverter<QSize>::cvt_to_string(const QSize& val)
{
	return QString::number(val.width()) + "," + QString::number(val.height());
}

bool SettingConverter<QSize>::cvt_from_string(const QString& val, QSize& sz)
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
QString SettingConverter<QPoint>::cvt_to_string(const QPoint& val)
{
	return QString::number(val.x()) + "," + QString::number(val.y());
}

bool SettingConverter<QPoint>::cvt_from_string(const QString& val, QPoint& sz)
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
QString SettingConverter<QByteArray>::cvt_to_string(const QByteArray& arr)
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

bool SettingConverter<QByteArray>::cvt_from_string(const QString& str, QByteArray& arr)
{
	if(str.isEmpty()){
		arr = QByteArray();
		return true;
	}

	QStringList numbers = str.split(",");

	for(const QString& num_str : numbers)
	{
		uint8_t num = num_str.toInt();
		arr.append((char) num);
	}

	return (numbers.size() > 0);
}
