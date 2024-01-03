/* OpusConverter.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "OpusConverter.h"
#include "Utils/MetaData/MetaData.h"

struct OpusConverter::Private
{
	bool cbr;

	Private(bool cbr) :
		cbr(cbr)
	{}
};

OpusConverter::OpusConverter(bool cbr, int quality, QObject* parent) :
	Converter(quality, parent)
{
	m = Pimpl::make<Private>(cbr);
}

OpusConverter::~OpusConverter() = default;

QStringList OpusConverter::supportedInputFormats() const
{
	return {"flac", "wav"};
}

QString OpusConverter::binary() const
{
	return "opusenc";
}

QStringList OpusConverter::processEntry(const MetaData& md) const
{
	QStringList ret
		{
			QString("--title"), QString("%1").arg(md.title()).toUtf8().data(),
			QString("--artist"), QString("%1").arg(md.artist().toUtf8().data()),
			QString("--album"), QString("%1").arg(md.album()).toUtf8().data(),
			//QString("--comment"), QString("%1").arg(md.comment()).toUtf8().data(),
			QString("--bitrate"), QString("%1.000").arg(quality())
		};

	if(m->cbr)
	{
		ret << "--hard-cbr";
	}

	else
	{
		ret << "--vbr";
	}

	ret << QStringList
		{
			QString("%1").arg(md.filepath()),
			QString("%1").arg(targetFile(md))
		};

	return ret;
}

QString OpusConverter::extension() const
{
	return "opus";
}
