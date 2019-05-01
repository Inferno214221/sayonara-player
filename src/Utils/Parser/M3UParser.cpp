/* M3UParser.cpp */

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

#include "PlaylistParser.h"
#include "M3UParser.h"

#include "Tagging/Tagging.h"

#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include <QRegExp>
#include <QStringList>

M3UParser::M3UParser(const QString& filename) :
	AbstractPlaylistParser(filename) {}

M3UParser::~M3UParser() {}

void M3UParser::parse()
{
	QStringList list = content().split('\n');

	MetaData md;

	for(QString line : list) {
		line = line.trimmed();
		if(line.isEmpty()){
			continue;
		}

		if(line.startsWith("#EXTINF:", Qt::CaseInsensitive)) {
			md = MetaData();
			parse_first_line(line, md);
			continue;
		}

		if(line.trimmed().startsWith('#')) {
			continue;
		}

		if(Util::File::is_playlistfile(line)){
			MetaDataList v_md = PlaylistParser::parse_playlist(line);
			add_tracks(v_md);
			continue;
		}

		else if( !Util::File::is_www(line)) {
			parse_local_file(line, md);
		}

		else {
			parse_www_file(line, md);
		}

		if(!md.filepath().isEmpty()){
			add_track(md);
			md = MetaData();
		}
	}
}


bool M3UParser::parse_first_line(const QString& line, MetaData& md)
{
	QRegExp re("^#EXTINF:\\s*([0-9]+)\\s*,\\s*(\\S)+\\s*-\\s*(\\S)+");
	int idx = re.indexIn(line);
	if(idx < 0){
		return false;
	}

	md.length_ms = re.cap(1).toInt() * 1000;
	md.set_artist(re.cap(2));
	md.set_title(re.cap(3));

	return true;
}


void M3UParser::parse_local_file(const QString& line, MetaData& md)
{
	MetaData md_db;
	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(-1, 0);

	QString abs_filename = get_absolute_filename(line);

	if(abs_filename.isEmpty()){
		return;
	}

	md_db = lib_db->getTrackByPath(abs_filename);

	if( md_db.id >= 0 ) {
		md = md_db;
	}

	else {
		md.set_filepath(abs_filename);
		Tagging::Utils::getMetaDataOfFile(md);
	}
}


void M3UParser::parse_www_file(const QString& line, MetaData& md)
{
	if(md.artist().isEmpty()){
		md.set_artist(line);
	}

	md.set_filepath(line);
}

