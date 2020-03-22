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
#include <QDir>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QPalette>
#include <QPixmap>
#include <QRegExp>
#include <QString>

#include <thread>
#include <chrono>

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


namespace Algorithm=Util::Algorithm;

template<typename T>
QString cvtNum2String(T num, int digits) {
	QString str = QString::number(num);
	while(str.size() < digits) {
		str.prepend("0");
	}

	return str;
}

uint64_t Util::dateToInt(const QDateTime& date_time)
{
	QString str = date_time.toUTC().toString("yyyyMMddHHmmss");
	return str.toULongLong();
}

QDateTime Util::intToDate(uint64_t date)
{
	QString str = QString::number(qulonglong(date));
	QDateTime dt;

	if(str.size() == 12)
	{
		dt = QDateTime::fromString(str, "yyMMddHHmmss");
		QDate date = dt.date();
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
	QString str = QDateTime::currentDateTimeUtc().toString("yyyyMMddHHmmss");
	return str.toULongLong();
}

QString Util::stringToFirstUpper(const QString& str)
{
	if(str.isEmpty()){
		return QString();
	}

	QString ret = str.toLower();
	QString chars=" \n\t. (+?!<\"";
	for(QChar c : chars)
	{
		QStringList lst = ret.split(c);

		for(QString& s : lst)
		{
			if(s.trimmed().isEmpty())
			{
				continue;
			}

			if(s[0].isLetter())
			{
				s.replace(0, 1, s[0].toUpper());
			}
		}

		ret = lst.join(c);
	}

	return ret;
}

QString Util::stringToVeryFirstUpper(const QString& str)
{
	if(str.isEmpty()){
		return str;
	}

	QString ret_str = str.toLower();
	ret_str.replace(0, 1, ret_str[0].toUpper());

	return ret_str;
}


QString Util::sayonaraPath() { return sayonaraPath(QString()); }
QString Util::sayonaraPath(const QString& append_path)
{
	QString basepath = Util::File::cleanFilename(getEnvironment("SAYONARA_HOME_DIR"));
	if(basepath.isEmpty())
	{
		basepath = QDir::homePath() + "/.Sayonara/";
	}

	basepath.replace("~", QDir::homePath());

	static bool checked = false;
	if(!checked && !Util::File::exists(basepath))
	{
		bool b = Util::File::createDir(basepath);
		if(!b) {
			return QString();
		}
	}

	checked = true;

	return Util::File::cleanFilename(basepath + "/" + append_path);
}

QString Util::sharePath() { return sharePath(QString()); }
QString Util::sharePath(const QString& append_path)
{
#ifdef Q_OS_WIN
	return QCoreApplication::applicationDirPath() + "/share/";
#else
	QString base_path(Util::File::cleanFilename(getEnvironment("SAYONARA_SHARE_DIR")));
	if(!Util::File::exists(base_path)){
		base_path = SAYONARA_INSTALL_SHARE_PATH;
	}

	return Util::File::cleanFilename(base_path + "/" + append_path);
#endif
}

QString Util::libPath() { return libPath(QString()); }
QString Util::libPath(const QString& append_path)
{
#ifdef Q_OS_WIN
	return QCoreApplication::applicationDirPath() + "/lib/";
#else

	QString base_path(Util::File::cleanFilename(getEnvironment("SAYONARA_LIB_DIR")));
	if(!Util::File::exists(base_path)){
		base_path = SAYONARA_INSTALL_LIB_PATH;
	}

	return Util::File::cleanFilename(base_path + "/" + append_path);
#endif
}

QString Util::tempPath()
{
	return tempPath(QString());
}

QString Util::tempPath(const QString& append_path)
{
	QString simple_temp_path = QDir::temp().absoluteFilePath("sayonara");
	QString path = simple_temp_path + "/" + append_path;

	if(!QFile::exists(simple_temp_path)) {
		Util::File::createDir(simple_temp_path);
	}

	return path;
}

QString Util::createLink(const QString& name, bool dark, bool underline)
{
	return createLink(name, dark, underline, QString());
}

QString Util::createLink(const QString& name, bool dark, bool underline, const QString& target)
{
	QString new_target;
	QString content;
	QString style;
	QString ret;

	if(target.size() == 0){
		new_target = name;
	}

	else {
		new_target = target;
	}

	if(!underline) {
		style = " style: \"text-decoration=none;\" ";
	};

	if(dark) {
		content = LIGHT_BLUE(name);
	}
	else {
		QColor color = QPalette().color(QPalette::Link);
		QString color_name = color.name(QColor::HexRgb);
		content = QString("<font color=%1>%2</font>").arg(color_name).arg(name);
	}

	if(new_target.contains("://") || new_target.contains("mailto:")){
		ret = QString("<a href=\"") + new_target + "\"" + style + ">" + content + "</a>";
	}

	else {
		ret = QString("<a href=\"file://") + new_target + "\"" + style + ">" + content + "</a>";
	}

	return ret;
}

QString Util::getFileFilter(Util::Extensions extensions, const QString& name)
{
	QStringList ret;
	if(extensions | Extension::Soundfile){
		ret << soundfileExtensions();
	}

	if(extensions | Extension::Playlist){
		ret << playlistExtensions();
	}

	if(extensions | Extension::Podcast){
		ret << podcastExtensions();
	}

	if(extensions | Extension::Haltdeimaul){
		ret << imageExtensions();
	}

	return QString("%1 (%2)").arg(name).arg(ret.join(" "));
}


QStringList Util::soundfileExtensions(bool with_asterisk)
{
	QStringList filters;
	filters << "mp3"
			<< "ogg"
			<< "opus"
			<< "oga"
			<< "m4a"
			<< "wav"
			<< "flac"
			<< "aac"
			<< "wma"
			<< "mpc"
			<< "aiff"
			<< "ape"
			<< "webm";

	QStringList upper_filters;
	for(QString& filter : filters) {
		if(with_asterisk) {
			filter.prepend("*.");
		}

		upper_filters << filter.toUpper();
	}

	filters.append(upper_filters);


	return filters;
}


QString Util::soundfileFilter()
{
	QStringList extensions = soundfileExtensions();
	return QString("Soundfiles (") + extensions.join(" ") + ")";
}

QStringList Util::playlistExtensions(bool with_asterisk)
{
	QStringList filters;

	filters << "pls"
			<< "m3u"
			<< "ram"
			<< "asx";

	QStringList upper_filters;
	for(QString& filter : filters) {
		if(with_asterisk) {
			filter.prepend("*.");
		}
		upper_filters << filter.toUpper();
	}

	filters.append(upper_filters);

	return filters;
}


QStringList Util::podcastExtensions(bool with_asterisk)
{
	QStringList filters;

	filters << "xml"
			<< "rss";

	QStringList upper_filters;
	for(QString& filter : filters) {
		if(with_asterisk) {
			filter.prepend("*.");
		}
		upper_filters << filter.toUpper();
	}

	filters.append(upper_filters);

	return filters;
}

QStringList Util::imageExtensions(bool with_asterisk)
{
	QStringList filters;

	filters << "jpg"
			<< "jpeg"
			<< "png"
			<< "bmp"
			<< "tiff"
			<< "tif"
			<< "svg";

	QStringList upper_filters;
	for(QString& filter : filters) {
		if(with_asterisk) {
			filter.prepend("*.");
		}
		upper_filters << filter.toUpper();
	}

	filters.append(upper_filters);

	return filters;
}


QString Util::easyTagFinder(const QString& tag, const QString& xml_doc)
{
	int p = tag.indexOf('.');
	QString ret = tag;
	QString new_tag = tag;
	QString t_rev;
	QString new_xml_doc = xml_doc;

	while(p > 0) {
		ret = new_tag.left(p);
		t_rev = tag.right(new_tag.length() - p -1);

		new_xml_doc = easyTagFinder(ret, new_xml_doc);
		p = t_rev.indexOf('.');
		new_tag = t_rev;
	}

	ret = new_tag;

	QString str2search_start = QString("<%1.*>").arg(ret);
	QString str2search_end = QString("</%1>").arg(ret);
	QString str2search = str2search_start + "(.+)" + str2search_end;
	QRegExp rx(str2search);
	rx.setMinimal(true);


	int pos = 0;
	if(rx.indexIn(new_xml_doc, pos) != -1) {
		return rx.cap(1);
	}

	return "";
}


QByteArray Util::calcHash(const QByteArray& data)
{
	if(data.isEmpty()){
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

#include <cstdlib>


QString Util::getEnvironment(const char* key)
{
#ifdef Q_OS_WIN
	_getenv(key.toLocal8Bit().constData());
	sp_log(Log::Info) << "Windows: Get environment variable " << key;
#else
	const char* c = getenv(key);
	if(c == nullptr){
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
	for(int i=0; i<max_chars; i++)
	{
		char c = static_cast<char>(randomNumber(97, 122));
		ret.append(QChar(c));
	}

	return ret;
}


QString Util::convertNotNull(const QString& str)
{
	if(str.isNull()){
		return QString("");
	}

	return str;
}

QByteArray Util::convertPixmapToByteArray(const QPixmap& pm)
{
	if(pm.hasAlpha()) {
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
	if(pm.isNull()) {
		pm.loadFromData(arr, "PNG");
	}
	return pm;
}


QString Util::msToString(MilliSeconds msec, const QString& format)
{
	uint64_t secs = uint64_t(msec / 1000);

	uint64_t sec = secs % 60;
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
		sec = sec % 60;
	}

	if(days == 0)
	{
		ret.replace("$De", QString());
		ret.replace("$D", QString());
	}

	if(hours == 0)
	{
		ret.replace("$He", QString());
		ret.replace("$H", QString());
	}

	ret.replace("$Se", QString("%1%2").arg(sec).arg(Lang::get(Lang::SecondsShort)));
	ret.replace("$Me", QString("%1%2").arg(min).arg(Lang::get(Lang::MinutesShort)));
	ret.replace("$He", QString("%1%2").arg(hours).arg(Lang::get(Lang::HoursShort)));
	ret.replace("$De", QString("%1%2").arg(days).arg(Lang::get(Lang::DaysShort)));

	ret.replace("$S", cvtNum2String(sec, 2));
	ret.replace("$M", cvtNum2String(min, 2));
	ret.replace("$H", QString::number(hours));
	ret.replace("$D", QString::number(days));

	return ret.trimmed();
}
