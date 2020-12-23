/* Helper.cpp */

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


/*
 * Helper.cpp
 *
 *  Created on: Apr 4, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#include <QtGlobal>

#include <QBuffer>
#include <QByteArray>
#include <QColor>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QPalette>
#include <QPixmap>
#include <QString>
#include <QXmlStreamReader>

#include <thread>
#include <chrono>
#include <cstdlib>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/RandomGenerator.h"
#include "Utils/Macros.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"

#ifdef Q_OS_UNIX
#ifndef SAYONARA_INSTALL_LIB_PATH
#define SAYONARA_INSTALL_LIB_PATH "/usr/lib/sayonara"
#endif
#endif

namespace Algorithm = Util::Algorithm;

namespace
{
	QStringList createFileExtensionList(const QStringList& extensions, bool withAsterisk)
	{
		QStringList result;
		for(const auto& extension : extensions)
		{
			if(withAsterisk)
			{
				result.push_back("*." + extension);
				result.push_back("*." + extension.toUpper());
			}
			else
			{
				result.push_back(extension);
				result.push_back(extension.toUpper());
			}
		}

		return result;
	}
}

template<typename T>
QString cvtNum2String(T num, int digits)
{
	auto str = QString::number(num);
	while(str.size() < digits)
	{
		str.prepend("0");
	}

	return str;
}

uint64_t Util::dateToInt(const QDateTime& dateTime)
{
	const auto str = dateTime.toUTC().toString("yyyyMMddHHmmss");
	return str.toULongLong();
}

QDateTime Util::intToDate(uint64_t date)
{
	const auto str = QString::number(qulonglong(date));
	QDateTime dt;

	if(str.size() == 12)
	{
		dt = QDateTime::fromString(str, "yyMMddHHmmss");
		auto date = dt.date();
		date.setDate(date.year() + 100, date.month(), date.day());
		dt.setDate(date);
	}

	else
	{
		dt = QDateTime::fromString(str, "yyyyMMddHHmmss");
	}

	dt.setOffsetFromUtc(0);

	return dt;
}

uint64_t Util::currentDateToInt()
{
	const auto str = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmss");
	return str.toULongLong();
}

QString Util::stringToFirstUpper(const QString& str)
{
	if(str.isEmpty())
	{
		return QString();
	}

	auto result = str.toLower();

	result[0] = result[0].toUpper();

	for(int i = 1; i < result.size() - 1; i++)
	{
		const auto currentChar = result[i];
		const auto nextChar = result[i + 1];
		const auto isSeparator = (!currentChar.isLetter() && !currentChar.isNumber() &&
		                          currentChar != "'");
		if(isSeparator && nextChar.isLetter())
		{
			result[i + 1] = nextChar.toUpper();
			i++;
		}
	}

	return result;
}

QString Util::stringToVeryFirstUpper(const QString& str)
{
	if(str.isEmpty())
	{
		return str;
	}

	auto result = str.toLower();
	result.replace(0, 1, result[0].toUpper());

	return result;
}

QString Util::createLink(const QString& name, bool dark, bool underline)
{
	return createLink(name, dark, underline, QString());
}

QString Util::createLink(const QString& name, bool dark, bool underline, const QString& target)
{
	const auto color = (dark == true) ?
	                   QColor("#8888FF") :
	                   QPalette().color(QPalette::Link);

	return createLink(name, color, underline, target);
}

QString
Util::createLink(const QString& name, const QColor& color, bool underline, const QString& target)
{
	const auto underlineText = (underline) ? QString("underline") : QString("none");
	const auto colorName = (color.isValid()) ? color.name(QColor::HexRgb) : QPalette().color(
		QPalette::Text).name(QColor::HexRgb);

	QMap<QString, QString> stylesheetMap
		{
			{"text-decoration", underlineText},
			{"color",           colorName}
		};

	auto newTarget = (target.isEmpty()) ? name : target;
	if(!newTarget.contains("://") && !newTarget.contains("mailto:"))
	{
		newTarget.prepend("file://");
	}

	QStringList styleStringlist;
	for(auto it = stylesheetMap.begin(); it != stylesheetMap.end(); it++)
	{
		styleStringlist << QString("%1: %2;")
			.arg(it.key())
			.arg(it.value());
	}

	const char* html = R"(<a href="%1"><span style="%2">%3</span></a>)";
	const auto ret =
		QString(html)
			.arg(newTarget)
			.arg(styleStringlist.join(" "))
			.arg(name);

	return ret;
}

QString Util::getFileFilter(Util::Extensions extensions, const QString& name)
{
	QStringList ret;
	if(extensions & +Extension::Soundfile)
	{
		ret << soundfileExtensions();
	}

	if(extensions & +Extension::Playlist)
	{
		ret << playlistExtensions();
	}

	if(extensions & +Extension::Podcast)
	{
		ret << podcastExtensions();
	}

	if(extensions & +Extension::Images)
	{
		ret << imageExtensions();
	}

	return QString("%1 (%2)")
		.arg(name)
		.arg(ret.join(" "));
}

QStringList Util::soundfileExtensions(bool withAsterisk)
{
	const auto filters = QStringList {
		"mp3", "ogg", "opus", "oga", "m4a", "wav", "flac", "aac", "wma", "mpc", "aiff", "ape",
		"webm"
	};

	return createFileExtensionList(filters, withAsterisk);
}

QString Util::soundfileFilter()
{
	const auto extensions = soundfileExtensions();
	return QString("Soundfiles (") + extensions.join(" ") + ")";
}

QStringList Util::playlistExtensions(bool withAsterisk)
{
	const auto filters = QStringList {"pls", "m3u", "ram", "asx"};
	return createFileExtensionList(filters, withAsterisk);
}

QStringList Util::podcastExtensions(bool withAsterisk)
{
	const auto filters = QStringList {"xml", "rss"};
	return createFileExtensionList(filters, withAsterisk);
}

QStringList Util::imageExtensions(bool withAsterisk)
{
	const auto filters = QStringList {"jpg", "jpeg", "png", "bmp", "tiff", "tif", "svg"};
	return createFileExtensionList(filters, withAsterisk);
}

QString Util::easyTagFinder(const QString& tag, const QString& xmlDocument)
{
	if(tag.isEmpty()){
		return QString();
	}

	if(xmlDocument.isEmpty()){
		return QString();
	}

	const auto tags = tag.split('.');
	auto reader = QXmlStreamReader(xmlDocument);

	auto it = tags.begin();

	while(reader.readNextStartElement())
	{
		if(reader.name() == *it)
		{
			it++;
			if(it == tags.end())
			{
				return reader.readElementText();
			}
		}

		else
		{
			reader.skipCurrentElement();
		}
	}

	return QString();
}

QByteArray Util::calcHash(const QByteArray& data)
{
	if(data.isEmpty())
	{
		return QByteArray();
	}

	return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

void Util::sleepMs(uint64_t ms)
{
#ifdef Q_OS_WIN
	Sleep(ms);
#else
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

int Util::randomNumber(int min, int max)
{
	return RandomGenerator().getNumber(min, max);
}

QStringList Util::ipAddresses()
{
	QStringList ret;
	QList<QHostAddress> host_list;
	host_list = QNetworkInterface::allAddresses();

	for(const QHostAddress& host : Algorithm::AsConst(host_list))
	{
		QString address = host.toString();
		if(!address.startsWith("127") &&
		   host.protocol() == QAbstractSocket::IPv4Protocol)
		{
			ret << host.toString();
		}
	}

	return ret;
}

QString Util::getEnvironment(const char* key)
{
#ifdef Q_OS_WIN
	_getenv(key.toLocal8Bit().constData());
	sp_log(Log::Info) << "Windows: Get environment variable " << key;
#else
	const char* c = getenv(key);
	if(c == nullptr)
	{
		return QString();
	}

	return QString(c);
#endif
}

void Util::setEnvironment(const QString& key, const QString& value)
{
#ifdef Q_OS_WIN
	QString str = key + "=" + value;
	_putenv(str.toLocal8Bit().constData());
	sp_log(Log::Info) << "Windows: Set environment variable " << str;
#else
	setenv(key.toLocal8Bit().constData(), value.toLocal8Bit().constData(), 1);
#endif
}

void Util::unsetEnvironment(const QString& key)
{
#ifdef Q_OS_WIN
	QString str = key + "=";
	_putenv(str.toLocal8Bit().constData());
	sp_log(Log::Info) << "Windows: Set environment variable " << str;
#else
	unsetenv(key.toLocal8Bit().constData());
#endif
}

QString Util::randomString(int max_chars)
{
	QString ret;
	for(int i = 0; i < max_chars; i++)
	{
		char c = static_cast<char>(randomNumber(97, 122));
		ret.append(QChar(c));
	}

	return ret;
}

QString Util::convertNotNull(const QString& str)
{
	if(str.isNull())
	{
		return QString("");
	}

	return str;
}

QByteArray Util::convertPixmapToByteArray(const QPixmap& pm)
{
	if(pm.hasAlpha())
	{
		return convertPixmapToByteArray(pm, "PNG");
	}

	return convertPixmapToByteArray(pm, "JPG");
}

QByteArray Util::convertPixmapToByteArray(const QPixmap& pm, const char* format)
{
	QByteArray arr;
	QBuffer buffer(&arr);
	buffer.open(QIODevice::WriteOnly);
	pm.save(&buffer, format);

	return arr;
}

QPixmap Util::convertByteArrayToPixmap(const QByteArray& arr)
{
	QPixmap pm;
	pm.loadFromData(arr, "JPG");
	if(pm.isNull())
	{
		pm.loadFromData(arr, "PNG");
	}
	return pm;
}

QString Util::msToString(MilliSeconds msec, const QString& format)
{
	uint64_t secs = uint64_t(msec / 1000);
	uint64_t min = secs / 60;
	uint64_t hours = min / 60;
	uint64_t days = hours / 24;

	QString ret(format);
	if(format.contains("$D"))
	{
		hours = hours % 24;
	}

	if(format.contains("$H"))
	{
		min = min % 60;
	}

	if(format.contains("$M"))
	{
		secs = secs % 60;
	}

	if(days == 0)
	{
		ret.replace("$De", QString());
		ret.replace("$D", QString());
	}

	if(days == 0 && hours == 0)
	{
		ret.replace("$He", QString());
		ret.replace("$H", QString());
	}

	ret.replace("$Se", QString("%1%2").arg(secs).arg(Lang::get(Lang::SecondsShort)));
	ret.replace("$Me", QString("%1%2").arg(min).arg(Lang::get(Lang::MinutesShort)));
	ret.replace("$He", QString("%1%2").arg(hours).arg(Lang::get(Lang::HoursShort)));
	ret.replace("$De", QString("%1%2").arg(days).arg(Lang::get(Lang::DaysShort)));

	ret.replace("$S", cvtNum2String(secs, 2));
	ret.replace("$M", cvtNum2String(min, 2));
	ret.replace("$H", QString::number(hours));
	ret.replace("$D", QString::number(days));

	return ret.trimmed();
}
