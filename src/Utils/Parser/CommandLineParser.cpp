/* CommandLineParser.cpp */

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

#include "CommandLineParser.h"
#include "Utils/Logger/Logger.h"
#include "Utils/FileUtils.h"
#include <QDir>

CommandLineData::CommandLineData()
{
	multiple_instances = false;
	abort = false;
	force_show = false;
}


CommandLineData CommandLineParser::parse(int argc, char** argv)
{
	CommandLineData data;
	QStringList www_files;

	for(int i=1; i<argc; i++)
	{
		QString str(argv[i]);
		QRegExp re("--lang=([a-z]+).*");

		if(str.compare("--help") == 0)
		{
			help();
			data.abort = true;
			return data;
		}

		if(str.compare("--multi-instances") == 0)
		{
			data.multiple_instances = true;
			continue;
		}

		if(str.compare("--show") == 0)
		{
			data.force_show = true;
			continue;
		}

		if(re.indexIn(str) >= 0)
		{
			data.language = re.cap(1);
			sp_log(Log::Info, "CommandLineParser") << "Force language to " << data.language;
		}

		else
		{
			if(Util::File::is_www(str))
			{
				www_files << str;
			}

			else if(Util::File::is_soundfile(str) || Util::File::is_playlistfile(str))
			{
				data.files_to_play << Util::File::get_absolute_filename(str);
			}
		}
	}

	if(!www_files.isEmpty())
	{
		QString playlist_filename = QDir::temp().absoluteFilePath("playlist.m3u");
		QByteArray raw_data = www_files.join("\n").toLocal8Bit();
		Util::File::write_file(raw_data, playlist_filename);
		data.files_to_play << playlist_filename;
	}

	return data;
}


void CommandLineParser::help()
{
	sp_log(Log::Info, "") << "sayonara [options] <list>";
	sp_log(Log::Info, "") << "<list> can consist of either files or directories or both";
	sp_log(Log::Info, "") << "Options:";
	sp_log(Log::Info, "") << "\t--multi-instances      Run more than one instance";
	sp_log(Log::Info, "") << "\t--lang=<country code>  Force language";
	sp_log(Log::Info, "") << "\t--help                 Print this help dialog";
	sp_log(Log::Info, "") << "Bye.";
}
