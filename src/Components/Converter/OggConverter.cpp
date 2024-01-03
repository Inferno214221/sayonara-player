/* OggConverter.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "OggConverter.h"
#include "Utils/MetaData/MetaData.h"

#include <QStringList>

OggConverter::OggConverter(int quality, QObject* parent) :
	Converter(quality, parent) {}

OggConverter::~OggConverter() = default;

QStringList OggConverter::supportedInputFormats() const
{
	return {"flac", "wav"};
}

QString OggConverter::binary() const
{
	return "oggenc";
}

QStringList OggConverter::processEntry(const MetaData& md) const
{
	QStringList ret
		{
			QString("-q"), QString("%1").arg(quality()),
			QString("-o"), QString("%1").arg(targetFile(md)),
			QString("%1").arg(md.filepath())
		};

	return ret;
}

QString OggConverter::extension() const
{
	return QString("ogg");
}
