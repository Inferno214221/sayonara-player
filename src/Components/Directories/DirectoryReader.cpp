/* DirectoryReader.cpp */

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

#include "DirectoryReader.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QDirIterator>
#include <QStringList>
#include <QDir>

namespace Algorithm = Util::Algorithm;

struct DirectoryReader::Private
{
	QStringList nameFilters;

	Private(const QStringList& filter) :
		nameFilters(filter)
	{}
};

DirectoryReader::DirectoryReader(const QStringList& filter)
{
	m = Pimpl::make<Private>(filter);
}

DirectoryReader::DirectoryReader() :
	DirectoryReader(QStringList())
{}

DirectoryReader::~DirectoryReader() = default;

void DirectoryReader::setFilter(const QStringList& filter)
{
	m->nameFilters = filter;
}

void DirectoryReader::setFilter(const QString& filter)
{
	m->nameFilters.clear();
	m->nameFilters << filter;
}

void DirectoryReader::scanFilesRecursive(const QDir& baseDirOrig, QStringList& files) const
{
	QDir baseDir(baseDirOrig);
	const QStringList tmpFiles = baseDir.entryList(m->nameFilters,
	                                               QDir::Filters(QDir::Files | QDir::NoDotAndDotDot));
	const QStringList dirs = baseDir.entryList(QDir::Filters(QDir::Dirs | QDir::NoDotAndDotDot));

	for(const QString& dir : dirs)
	{
		baseDir.cd(dir);
		scanFilesRecursive(baseDir, files);
		baseDir.cdUp();
	}

	Util::Algorithm::transform(tmpFiles, files, [&baseDir](const QString& filename)
	{
		return baseDir.absoluteFilePath(filename);
	});
}

void DirectoryReader::scanFiles(const QDir& base_dir, QStringList& files) const
{
	const QStringList tmp_files = base_dir.entryList
		(
			m->nameFilters,
			QDir::Filters(QDir::Files | QDir::NoDotAndDotDot)
		);

	for(const QString& filename : tmp_files)
	{
		files << base_dir.absoluteFilePath(filename);
	}
}

MetaDataList DirectoryReader::scanMetadata(const QStringList& fileList)
{
	// fetch sound and playlist files
	QStringList filter;
	{
		filter << Util::soundfileExtensions();
		filter << Util::playlistExtensions();
	}

	setFilter(filter);

	QStringList soundFiles, playlistFiles;
	for(const QString& filename : fileList)
	{
		if(!Util::File::exists(filename))
		{
			continue;
		}

		if(Util::File::isDir(filename))
		{
			QDir dir(filename);
			dir.cd(filename);

			QStringList files;
			scanFilesRecursive(dir, files);
			for(const QString& file : Algorithm::AsConst(files))
			{
				if(Util::File::isSoundFile(file))
				{
					soundFiles << file;
				}
			}
		}

		else if(Util::File::isSoundFile(filename))
		{
			soundFiles << filename;
		}

		else if(Util::File::isPlaylistFile(filename))
		{
			playlistFiles << filename;
		}
	}

	MetaDataList tracks;
	{ // fetch tracks from DB
		auto* db = DB::Connector::instance();
		auto* libraryDatabase = db->libraryDatabase(-1, 0);
		libraryDatabase->getMultipleTracksByPath(soundFiles, tracks);
	}

	for(auto it = tracks.begin(); it != tracks.end(); it++)
	{
		if(it->id() >= 0)
		{
			continue;
		}

		it->setExtern(true);
		if(!Tagging::Utils::getMetaDataOfFile(*it))
		{
			it->setTitle(it->filepath());
		}
	}

	for(const QString& playlistFile : playlistFiles)
	{
		tracks << PlaylistParser::parsePlaylist(playlistFile);
	}

	return tracks;
}

QStringList DirectoryReader::findFilesRecursive(const QDir& dirOrig, const QString& filename)
{
	if(dirOrig.canonicalPath().isEmpty())
	{
		return QStringList();
	}

	if(filename.isEmpty())
	{
		return QStringList();
	}

	QDir dir(dirOrig);

	const QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	const QStringList files = dir.entryList(QDir::Files);

	QStringList ret;
	for(const QString& d : dirs)
	{
		if(d.isEmpty())
		{
			continue;
		}

		if(dir.cd(d))
		{
			ret += findFilesRecursive(dir, filename);
			dir.cdUp();
		}
	}

	for(const QString& file : files)
	{
		if(file.contains(filename))
		{
			ret += dir.absoluteFilePath(file);
		}
	}

	return ret;
}
