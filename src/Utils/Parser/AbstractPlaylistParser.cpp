/* AbstractPlaylistParser.cpp */

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

#include "AbstractPlaylistParser.h"
#include "PlaylistParser.h"
#include "Utils/FileUtils.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QDir>

struct AbstractPlaylistParser::Private
{
	MetaDataList		v_md;
	QString				file_content;
	QString				directory;
	bool				parsed;

	Private()
	{
		parsed = false;
	}
};


AbstractPlaylistParser::AbstractPlaylistParser(const QString& filename)
{
	m = Pimpl::make<AbstractPlaylistParser::Private>();

	QString pure_file;

	Util::File::splitFilename(filename, m->directory, pure_file);
	Util::File::readFileIntoString(filename, m->file_content);

}

AbstractPlaylistParser::~AbstractPlaylistParser() {}

MetaDataList AbstractPlaylistParser::tracks(bool force_parse)
{
	if(force_parse){
		m->parsed = false;
	}

	if(!m->parsed){
		m->v_md.clear();
		parse();
	}

	m->parsed = true;


	return m->v_md;
}

void AbstractPlaylistParser::addTrack(const MetaData& md)
{
	m->v_md << md;
}

void AbstractPlaylistParser::addTracks(const MetaDataList& v_md)
{
	m->v_md << v_md;
}

const QString& AbstractPlaylistParser::content() const
{
	return m->file_content;
}


QString AbstractPlaylistParser::getAbsoluteFilename(const QString& filename) const
{
	QString ret;

	if(filename.isEmpty()){
		return "";
	}

	if(Util::File::isWWW(filename)){
		return filename;
	}

	if(!Util::File::isAbsolute(filename)){
		ret = m->directory + "/" + filename;
	}
	else{
		ret = filename;
	}

	if(!Util::File::exists(ret)){
		ret = "";
	}

	return Util::File::cleanFilename(ret);
}
