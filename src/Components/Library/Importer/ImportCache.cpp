/* ImportCache.cpp */

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

#include "ImportCache.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/Library/LibraryInfo.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QHash>
#include <QString>
#include <QStringList>

using Library::ImportCache;

struct ImportCache::Private
{
	QString					libraryPath;
	MetaDataList			tracks;
	QHash<QString, int>		pathIndexMap;
	QHash<QString, QString>	pathTargetMap;
	QStringList				files;

	Private(const QString& library_path) :
		libraryPath(library_path)
	{}

	Private(const Private& other) :
		CASSIGN(libraryPath),
		CASSIGN(tracks),
		CASSIGN(pathIndexMap),
		CASSIGN(pathTargetMap),
		CASSIGN(files)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(libraryPath);
		ASSIGN(tracks);
		ASSIGN(pathIndexMap);
		ASSIGN(pathTargetMap);
		ASSIGN(files);

		return *this;
	}
};

ImportCache::ImportCache(const QString& libraryPath)
{
	m = Pimpl::make<Private>(libraryPath);
}

ImportCache::ImportCache(const ImportCache& other)
{
	m = Pimpl::make<Private>(*(other.m));
}

ImportCache::~ImportCache() = default;

ImportCache& ImportCache::operator=(const ImportCache& other)
{
	*m = *(other.m);

	return *this;
}

void ImportCache::clear()
{
	m->files.clear();
	m->tracks.clear();
	m->pathTargetMap.clear();
}

void ImportCache::addSoundfile(const QString& filename)
{
	MetaData md(filename);
	bool success = Tagging::Utils::getMetaDataOfFile(md);
	if(success)
	{
		m->tracks << md;
		m->pathIndexMap[md.filepath()] = m->tracks.count() - 1;
	}
}

void ImportCache::addFile(const QString& filename)
{
	addFile(filename, QString());
}

void ImportCache::addFile(const QString& filename, const QString& parentDirectory)
{
	// filename: /path/to/dir/and/further/to/file.mp4
	// parent_dir: /path/to/dir
	// remainder: and/further/to/file.mp4

	const QString absoluteFilename = Util::File::cleanFilename(filename);
	const QString absoluteParentDir = Util::File::cleanFilename(parentDirectory);

	if(absoluteFilename.isEmpty()) {
		return;
	}

	QString remainder;
	if(!parentDirectory.isEmpty())
	{
		if(	(Util::File::isSamePath(absoluteFilename, absoluteParentDir)) ||
			(!Util::File::isSubdir(absoluteFilename, absoluteParentDir)) )
		{
			return;
		}

		remainder = filename.right(filename.size() - absoluteParentDir.size());
		while(remainder.startsWith('/') || remainder.startsWith('\\')) {
			remainder.remove(0, 1);
		}
	}

	else {
		remainder = Util::File::getFilenameOfPath(filename);
	}

	m->pathTargetMap[filename] = remainder;

	m->files << absoluteFilename;
	if(Util::File::isSoundFile(absoluteFilename))
	{
		addSoundfile(absoluteFilename);
	}
}

QStringList ImportCache::files() const
{
	return m->files;
}

MetaDataList ImportCache::soundfiles() const
{
	return m->tracks;
}

int ImportCache::count() const
{
	return m->files.count();
}

int ImportCache::soundFileCount() const
{
	return m->tracks.count();
}

QString ImportCache::targetFilename(const QString& src_filename, const QString& targetDirectoryectory) const
{
	if(m->libraryPath.isEmpty()){
		return QString();
	}

	QString original_path = Util::File::cleanFilename(src_filename);

	QString path = QString("%1/%2/%3")
				.arg(m->libraryPath)
				.arg(targetDirectoryectory)
				.arg(m->pathTargetMap[original_path]);

	return Util::File::cleanFilename(path);
}

MetaData ImportCache::metadata(const QString& filename) const
{
	if(!m->pathIndexMap.contains(filename))
	{
		spLog(Log::Warning, this) << filename  << " is no valid audio file";
		return MetaData();
	}

	int index = m->pathIndexMap[filename];
	return m->tracks[index];
}

void ImportCache::changeMetadata(const QList<QPair<MetaData, MetaData>>& changedTracks)
{
	for(const auto& trackPair : changedTracks)
	{
		int index = Util::Algorithm::indexOf(m->tracks, [&trackPair](const MetaData& cachedTrack) {
			return Util::File::isSamePath(trackPair.first.filepath(), cachedTrack.filepath());
		});

		if(index >= 0) {
			m->tracks[index] = trackPair.second;
		}
	}

	spLog(Log::Develop, this) << "Import cache updated";
}
