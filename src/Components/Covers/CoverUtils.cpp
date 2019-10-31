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
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include <QFileInfo>
#include <QPixmap>
#include <QDir>
#include <QStringList>

namespace Algorithm=Util::Algorithm;
namespace FileUtils=Util::File;

QString Cover::Utils::calc_cover_token(const QString& artist, const QString& album)
{
	QByteArray str = QString(artist.trimmed() + album.trimmed()).toLower().toUtf8();

	return Util::calc_hash(str);
}

QString Cover::Utils::cover_directory()
{
	return cover_directory(QString());
}

QString Cover::Utils::cover_directory(const QString& append_filename)
{
	QString cover_dir = Util::sayonara_path("covers");
	if(!FileUtils::exists(cover_dir))
	{
		QDir().mkdir(cover_dir);
	}

	if(!append_filename.isEmpty()){
		cover_dir += "/" + append_filename;
	}

	return FileUtils::clean_filename(cover_dir);
}

QString Cover::Utils::cover_temp_directory()
{
	return cover_directory("tmp");
}


void Cover::Utils::delete_temp_covers()
{
	QString dir = cover_temp_directory();

	if(QFile::exists(dir))
	{
		FileUtils::remove_files_in_directory(dir);
	}
}


bool Cover::Utils::add_temp_cover(const QPixmap& pm, const QString& hash)
{
	QDir cover_temp_dir = QDir(cover_temp_directory());
	QString path = cover_temp_dir.filePath("tmp_" + hash + ".jpg");
	return pm.save(path);
}


void Cover::Utils::write_cover_to_sayonara_dir(const Cover::Location& cl, const QPixmap& pm)
{
	QString path = cl.cover_path();
	QFileInfo fi(path);
	if(fi.isSymLink()){
		QFile::remove(path);
	}

	pm.save(path);
}

void Cover::Utils::write_cover_to_db(const Cover::Location& cl, const QPixmap& pm)
{
	write_cover_to_db(cl, pm, DB::Connector::instance());
}

void Cover::Utils::write_cover_to_db(const Cover::Location& cl, const QPixmap& pm, DB::Connector* db)
{
	DB::Covers* dbc = db->cover_connector();
	dbc->set_cover(cl.hash(), pm);
}

void Cover::Utils::write_cover_to_library(const Cover::Location& cl, const QPixmap& pm)
{
	QString local_dir = cl.local_path_dir();
	if(local_dir.isEmpty()){
		return;
	}

	QString cover_template = GetSetting(Set::Cover_TemplatePath);
	cover_template.replace("<h>", cl.hash());

	QString filepath = QDir(local_dir).absoluteFilePath(cover_template);
	QFileInfo fi(filepath);

	pm.save(filepath);
}
