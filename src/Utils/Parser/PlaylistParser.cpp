/* PlaylistParser.cpp */

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

#include "M3UParser.h"
#include "PLSParser.h"
#include "ASXParser.h"

#include "Utils/Parser/PlaylistParser.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include <QUrl>
#include <QDir>
#include <QFile>

MetaDataList PlaylistParser::parse_playlist(const QString& local_filename)
{
	if(Util::File::is_www(local_filename)){
		return MetaDataList();
	}

	MetaDataList result;
	MetaDataList v_md_tmp;
	MetaDataList v_md_to_delete;
	AbstractPlaylistParser* playlist_parser;

	if(local_filename.endsWith(QStringLiteral("m3u"), Qt::CaseInsensitive)) {
		playlist_parser = new M3UParser(local_filename);
	}

	else if(local_filename.endsWith(QStringLiteral("pls"), Qt::CaseInsensitive)) {
		playlist_parser = new PLSParser(local_filename);
	}

	else if(local_filename.endsWith(QStringLiteral("ram"), Qt::CaseInsensitive)) {
		playlist_parser = new M3UParser(local_filename);
	}

	else if(local_filename.endsWith(QStringLiteral("asx"), Qt::CaseInsensitive)) {
		playlist_parser = new ASXParser(local_filename);
	}

	else{
		playlist_parser = new M3UParser(local_filename);
		v_md_tmp = playlist_parser->tracks();

		if(v_md_tmp.isEmpty()){
			delete playlist_parser;
			playlist_parser = new PLSParser(local_filename);
			v_md_tmp = playlist_parser->tracks();
		}

		if(v_md_tmp.isEmpty()){
			delete playlist_parser; playlist_parser = nullptr;
			playlist_parser = new ASXParser(local_filename);
		}
	}

	v_md_tmp = playlist_parser->tracks();

	for(const MetaData& md : v_md_tmp)
	{
		if( Util::File::check_file(md.filepath()) ){
			result << md;
		}

		else {
			v_md_to_delete << md;
		}
	}

	if(!v_md_to_delete.isEmpty())
	{
		DB::Connector* db = DB::Connector::instance();
		DB::LibraryDatabase* lib_db = db->library_db(-1, 0);
		lib_db->deleteTracks(v_md_to_delete);
	}

	if(playlist_parser){
		delete playlist_parser; playlist_parser = nullptr;
	}

	result.remove_duplicates();

	return result;
}


void PlaylistParser::save_m3u_playlist(const QString& filename, const MetaDataList& v_md, bool relative)
{
	QString f = filename;
	if(!f.endsWith("m3u", Qt::CaseInsensitive)) {
		f.append(".m3u");
	}

	bool success;
	QString dir_str = f.left(f.lastIndexOf(QDir::separator()));
	QDir dir(dir_str);
	dir.cd(dir_str);

	QFile file(f);
	success = file.open(QIODevice::WriteOnly);
	if(!success){
		return;
	}

	file.write( QByteArray("#EXTM3U\n") );

	int64_t lines = 0;
	for(const MetaData& md : v_md) {
		QString str;
		if(relative) {
			str = dir.relativeFilePath(md.filepath());
		}

		else{
			str = md.filepath();
		}

		QString ext_data = "#EXTINF: " + QString::number(md.length_ms / 1000)  + ", " + md.artist() + " - " + md.title() + "\n";
		lines += file.write(ext_data.toLocal8Bit());
		lines += file.write(str.toLocal8Bit());
		lines += file.write( QByteArray("\n") );
	}

	file.close();
}
