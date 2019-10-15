/* LameConverter.cpp */

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

#include "LameConverter.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/Genre.h"

struct LameConverter::Private
{
	bool cbr;

	Private(bool cbr) :
		cbr(cbr)
	{}
};

LameConverter::LameConverter(bool cbr, int quality, QObject* parent) :
	Converter(quality, parent)
{
	m = Pimpl::make<Private>(cbr);
}

LameConverter::~LameConverter() = default;

QStringList LameConverter::supported_input_formats() const
{
	return {"flac", "wav"};
}

QString LameConverter::binary() const
{
	return "lame";
}

QStringList LameConverter::process_entry(const MetaData& md) const
{
	QStringList ret
	{
		"--id3v2-only",
		"--verbose",
		QString("--tt"), QString("%1").arg(md.title()).toUtf8().data(),
		QString("--ta"), QString("%1").arg(md.artist().toUtf8().data()),
		QString("--tl"), QString("%1").arg(md.album()).toUtf8().data(),
		QString("--ty"), QString("%1").arg(md.year).toUtf8().data(),
		QString("--tc"), QString("%1").arg(md.comment()).toUtf8().data(),
		QString("--tn"), QString("%1").arg(md.track_num).toUtf8().data(),
		QString("--tg"), QString("%1").arg(md.genres_to_list().join(",")).toUtf8().data(),
	};

	if(m->cbr)
	{
		ret << QStringList
		{
			QString("--cbr"),
			QString("-b"), QString("%1").arg(quality())
		};
	}

	else
	{
		ret << QStringList
		{
			QString("--vbr"),
			QString("-V"), QString("%1").arg(quality())
		};
	}

	ret << QStringList
	{
		QString("%1").arg(md.filepath()),
		QString("%1").arg(target_file(md))
	};

	return ret;
}


QString LameConverter::extension() const
{
	return "mp3";
}
