/* CommandLineParser.cpp */

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

#include "CommandLineParser.h"
#include "Utils/Logger/Logger.h"
#include "Utils/FileUtils.h"

#include <QDir>

CommandLineData::CommandLineData() :
	multipleInstances {false},
	abort {false},
	forceShow {false} {}

CommandLineData CommandLineParser::parse(int argc, char** argv)
{
	CommandLineData data;
	QStringList onlineFiles;

	for(auto i = 1; i < argc; i++)
	{
		const auto str = QString(argv[i]);
		const auto regExp = QRegExp("--lang=([a-z]+).*");

		if(str == "--help")
		{
			help();
			data.abort = true;
			return data;
		}

		if(str == "--multi-instances")
		{
			data.multipleInstances = true;
			continue;
		}

		if(str == "--show")
		{
			data.forceShow = true;
			continue;
		}

		if(regExp.indexIn(str) >= 0)
		{
			data.language = regExp.cap(1);
			spLog(Log::Info, "CommandLineParser") << "Force language to " << data.language;
		}

		else
		{
			if(Util::File::isWWW(str))
			{
				onlineFiles << str;
			}

			else if(Util::File::isSoundFile(str) || Util::File::isPlaylistFile(str) || Util::File::isDir(str))
			{
				data.filesToPlay << Util::File::getAbsoluteFilename(str);
			}
		}
	}

	if(!onlineFiles.isEmpty())
	{
		const auto playlistFilename = QDir::temp().absoluteFilePath("playlist.m3u");
		const auto rawData = onlineFiles.join("\n").toLocal8Bit();
		Util::File::writeFile(rawData, playlistFilename);

		data.filesToPlay << playlistFilename;
	}

	return data;
}

void CommandLineParser::help()
{
	spLog(Log::Info, "") << "sayonara [options] <list>";
	spLog(Log::Info, "") << "<list> can consist of either files or directories or both";
	spLog(Log::Info, "") << "Options:";
	spLog(Log::Info, "") << "\t--multi-instances      Run more than one instance";
	spLog(Log::Info, "") << "\t--lang=<country code>  Force language";
	spLog(Log::Info, "") << "\t--help                 Print this help dialog";
	spLog(Log::Info, "") << "Bye.";
}
