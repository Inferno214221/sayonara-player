/* SomaFMStation.cpp */

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

/* SomaFMStation.cpp */

#include "SomaFMStation.h"

#include "Helper/MetaData/MetaDataList.h"
#include "Helper/Helper.h"
#include "Helper/FileHelper.h"

#include "Components/Covers/CoverLocation.h"

#include <QMap>
#include <QStringList>
#include <QUrl>

struct SomaFMStation::Private
{
	QString			content;
	QString			station_name;
	QMap<QString, SomaFMStation::UrlType> urls;
	QString			description;
	CoverLocation	cover;
	MetaDataList	v_md;
	bool			loved;

	QString complete_url(const QString& url)
	{
		if(url.startsWith("/")){
			return QString("https://somafm.com") + url;
		}

		return url;
	}

	void parse_station_name()
	{
		QString pattern("<h3>(.*)</h3>");
		QRegExp re(pattern);
		re.setMinimal(true);

		int idx = re.indexIn(content);
		if(idx > 0){
			station_name = Helper::cvt_str_to_first_upper(re.cap(1));
		}
	}

	void parse_urls()
	{
		QString mp3_pattern("<nobr>\\s*MP3:\\s*<a\\s+href=\"(.*)\"");
		QString aac_pattern("<nobr>\\s*AAC:\\s*<a\\s+href=\"(.*)\"");
		QRegExp re_mp3(mp3_pattern);
		QRegExp re_aac(aac_pattern);

		re_mp3.setMinimal(true);
		re_aac.setMinimal(true);

		int idx=-1;
		do{
			idx = re_mp3.indexIn(content, idx+1);
			if(idx > 0){
				QString url = complete_url(re_mp3.cap(1));
				urls[url] = SomaFMStation::UrlType::MP3;
			}
		} while(idx > 0);

		idx=-1;
		do{
			idx = re_aac.indexIn(content, idx+1);

			if(idx > 0){
				QString url = complete_url(re_aac.cap(1));
				urls[url] = SomaFMStation::UrlType::AAC;
			}

		} while(idx > 0);
	}



	void parse_description()
	{
		QString pattern("<p\\s*class=\"descr\">(.*)</p>");
		QRegExp re(pattern);
		re.setMinimal(true);

		int idx = re.indexIn(content);
		if(idx > 0){
			description = re.cap(1);
		}
	}

	void parse_image()
	{
		QString pattern("<img\\s*src=\\s*\"(.*)\"");
		QRegExp re(pattern);

		re.setMinimal(true);

		int idx = re.indexIn(content);
		if(idx > 0){
			QString url = complete_url(re.cap(1));
			QString cover_path = Helper::get_sayonara_path() +
					"/covers/" +
					station_name + "." + Helper::File::get_file_extension(url);

			cover = CoverLocation::get_cover_location(QUrl(url), cover_path);
		}
	}


};


SomaFMStation::SomaFMStation()
{
	_m = Pimpl::make<SomaFMStation::Private>();
	_m->cover = CoverLocation::getInvalidLocation();
	_m->loved = false;
}

SomaFMStation::SomaFMStation(const QString& content) :
	SomaFMStation()
{
	_m->content = content;

	_m->parse_description();
	_m->parse_station_name();
	_m->parse_image();
	_m->parse_urls();
}

SomaFMStation::SomaFMStation(const SomaFMStation& other)
{
	_m = Pimpl::make<SomaFMStation::Private>();
	SomaFMStation::Private data = *(other._m.get());
	(*_m) = data;
}

SomaFMStation& SomaFMStation::operator=(const SomaFMStation& other)
{
	SomaFMStation::Private data = *(other._m.get());
	(*_m) = data;
	return *this;
}

SomaFMStation::~SomaFMStation() {}


QString SomaFMStation::get_name() const
{
	return _m->station_name;
}

QStringList SomaFMStation::get_urls() const
{
	return _m->urls.keys();
}

SomaFMStation::UrlType SomaFMStation::get_url_type(const QString& url) const
{
	return _m->urls[url];
}

QString SomaFMStation::get_description() const
{
	return _m->description;
}

CoverLocation SomaFMStation::get_cover_location() const
{
	return _m->cover;
}

bool SomaFMStation::is_valid() const
{
	return (!_m->station_name.isEmpty() &&
			!_m->urls.isEmpty() &&
			!_m->description.isEmpty() &&
			_m->cover.valid());
}

MetaDataList SomaFMStation::get_metadata() const
{
	return _m->v_md;
}

void SomaFMStation::set_metadata(const MetaDataList& v_md)
{
	_m->v_md = v_md;
}

void SomaFMStation::set_loved(bool loved){
	_m->loved = loved;
}

bool SomaFMStation::is_loved() const
{
	return _m->loved;
}
