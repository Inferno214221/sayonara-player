/* PLSParser.cpp */

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

#include "PLSParser.h"

#include "Utils/FileSystem.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Tagging/TagReader.h"

#include <QMap>
#include <QRegExp>
#include <QStringList>

namespace
{
	int getIndex(const QString& key)
	{
		auto re = QRegExp {"\\S+([0-9]+)"};
		const auto index = (re.indexIn(key) >= 0)
		                   ? re.cap(1).toInt()
		                   : -1;

		return index;
	}

	QMap<QString, QString> parseFileIntoMap(const QStringList& lines)
	{
		auto result = QMap<QString, QString> {};
		for(auto line: lines)
		{
			line = line.trimmed();
			if(line.startsWith("#") || line.isEmpty())
			{
				continue;
			}

			auto splitted = line.split("=");
			const auto key = splitted.takeFirst().toLower().trimmed();

			if(splitted.count() >= 1)
			{
				result[key] = splitted.join("="); // maybe there's an '=' inside the value
			}
		}

		return result;
	}
}

PLSParser::PLSParser(const QString& filename,
                     const Util::FileSystemPtr& fileSystem,
                     const Tagging::TagReaderPtr& tagReader) :
	AbstractPlaylistParser(filename, fileSystem, tagReader) {}

PLSParser::~PLSParser() = default;

void PLSParser::parse()
{
	const auto lines = content().split("\n");
	const auto map = parseFileIntoMap(lines);

	const auto keys = map.keys();
	for(const auto& key: keys)
	{
		if(!key.startsWith("file"))
		{
			continue;
		}

		if(const auto index = getIndex(key); index >= 0)
		{
			auto track = MetaData {getAbsoluteFilename(map[key])};

			if(const auto titleKey = QString("title%1").arg(index); map.contains(titleKey))
			{
				track.setTitle(map[titleKey]);
			}

			if(const auto lengthKey = QString("length%1").arg(index); map.contains(lengthKey))
			{
				const auto len = std::max(0, map[lengthKey].toInt());
				track.setDurationMs(len * 1'000); // NOLINT(readability-magic-numbers)
			}

			addTrack(track);
		}
	}
}
