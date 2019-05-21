/* CoverHelper.cpp */

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

#include "CoverUtils.h"
#include "CoverLocation.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Components/Directories/DirectoryReader.h"
#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include <QFileInfo>
#include <QPixmap>
#include <QDir>
#include <QStringList>

namespace FileUtils=::Util::File;

QString Cover::Utils::calc_cover_token(const QString& artist, const QString& album)
{
	QByteArray str = QString(artist.trimmed() + album.trimmed()).toLower().toUtf8();

	return ::Util::calc_hash(str);
}

QString Cover::Utils::cover_directory()
{
	return cover_directory(QString());
}

QString Cover::Utils::cover_directory(const QString& append_filename)
{
	QString cover_dir = ::Util::sayonara_path("covers");
	if(!FileUtils::exists(cover_dir))
	{
		QDir().mkdir(cover_dir);
	}

	if(!append_filename.isEmpty()){
		cover_dir += "/" + append_filename;
	}

	return FileUtils::clean_filename(cover_dir);
}


void Cover::Utils::delete_temp_covers()
{
	QDir cover_dir = QDir(cover_directory());

	QStringList files, files_to_delete;

	DirectoryReader reader;
	reader.set_filter({"*.jpg", "*.png"});
	reader.scan_files(cover_dir, files);

	for(const QString& f : ::Util::AsConst(files))
	{
		QString pure_filename = FileUtils::get_filename_of_path(f);
		if(pure_filename.startsWith("tmp"))
		{
			files_to_delete << f;
		}
	}

	FileUtils::delete_files(files_to_delete);
}


bool Cover::Utils::add_temp_cover(const QPixmap& pm, const QString& hash)
{
	QDir cover_dir = QDir(cover_directory());
	QString path = cover_dir.filePath("tmp_" + hash + ".jpg");
	return pm.save(path);
}


void Cover::Utils::write_cover_to_sayonara_dir(const Cover::Location& cl, const QPixmap& pm)
{
	QFileInfo fi(cl.cover_path());
	if(fi.isSymLink()){
		QFile::remove(cl.cover_path());
	}

	pm.save(cl.cover_path());
}

void Cover::Utils::write_cover_to_db(const Cover::Location& cl, const QPixmap& pm)
{
	DB::Covers* dbc = DB::Connector::instance()->cover_connector();
	dbc->set_cover(cl.hash(), pm);
}

void Cover::Utils::write_cover_to_library(const Cover::Location& cl, const QPixmap& pm)
{
	QString filepath = cover_directory(cl.hash() + ".jpg");
	QFileInfo fi(filepath);
	if(fi.isSymLink()){
		Util::File::delete_files({filepath});
	}

	pm.save(filepath);
}
