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
#include "Utils/Tagging/TagReader.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"

#include <QHash>
#include <QString>
#include <QStringList>
#include <utility>

using Library::ImportCache;

struct ImportCache::Private
{
	QString libraryPath;
	Tagging::TagReaderPtr tagReader;
	MetaDataList tracks;
	QHash<QString, int> pathIndexMap;
	QHash<QString, QString> pathTargetMap;
	QStringList files;

	Private(QString libraryPath, Tagging::TagReaderPtr tagReader) :
		libraryPath(std::move(libraryPath)),
		tagReader(std::move(tagReader)) {}

	Private(const Private& other) = default;
	Private& operator=(const Private& other) = default;
};

ImportCache::ImportCache(const QString& libraryPath, const Tagging::TagReaderPtr& tagReader)
{
	m = Pimpl::make<Private>(libraryPath, tagReader);
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
	const auto track = m->tagReader->readMetadata(filename);
	if(track.has_value())
	{
		m->tracks << track.value();
		m->pathIndexMap[track->filepath()] = m->tracks.count() - 1;
	}
}

void ImportCache::addFile(const QString& filename, const QString& parentDirectory)
{
	// filename: /path/to/dir/and/further/to/file.mp4
	// parent_dir: /path/to/dir
	// remainder: and/further/to/file.mp4

	const QString absoluteFilename = Util::File::cleanFilename(filename);
	const QString absoluteParentDir = Util::File::cleanFilename(parentDirectory);

	if(absoluteFilename.isEmpty())
	{
		return;
	}

	QString remainder;
	if(!parentDirectory.isEmpty())
	{
		if((Util::File::isSamePath(absoluteFilename, absoluteParentDir)) ||
		   (!Util::File::isSubdir(absoluteFilename, absoluteParentDir)))
		{
			return;
		}

		remainder = filename.right(filename.size() - absoluteParentDir.size());
		while(remainder.startsWith('/') || remainder.startsWith('\\'))
		{
			remainder.remove(0, 1);
		}
	}

	else
	{
		remainder = Util::File::getFilenameOfPath(filename);
	}

	m->pathTargetMap[filename] = remainder;

	m->files << absoluteFilename;
	if(Util::File::isSoundFile(absoluteFilename))
	{
		addSoundfile(absoluteFilename);
	}
}

QStringList ImportCache::files() const { return m->files; }

MetaDataList ImportCache::soundfiles() const { return m->tracks; }

int ImportCache::count() const { return m->files.count(); }

int ImportCache::soundFileCount() const { return m->tracks.count(); }

QString ImportCache::targetFilename(const QString& src_filename, const QString& targetDirectoryectory) const
{
	if(m->libraryPath.isEmpty())
	{
		return {};
	}

	const auto originalPath = Util::File::cleanFilename(src_filename);
	const auto path = QString("%1/%2/%3")
		.arg(m->libraryPath)
		.arg(targetDirectoryectory)
		.arg(m->pathTargetMap[originalPath]);

	return Util::File::cleanFilename(path);
}

MetaData ImportCache::metadata(const QString& filename) const
{
	if(!m->pathIndexMap.contains(filename))
	{
		spLog(Log::Warning, this) << filename << " is no valid audio file";
		return {};
	}

	const auto index = m->pathIndexMap[filename];
	return m->tracks[index];
}

void ImportCache::changeMetadata(const QList<QPair<MetaData, MetaData>>& changedTracks)
{
	for(const auto& trackPair: changedTracks)
	{
		const auto index = Util::Algorithm::indexOf(m->tracks, [&trackPair](const auto& cachedTrack) {
			return Util::File::isSamePath(trackPair.first.filepath(), cachedTrack.filepath());
		});

		if(index >= 0)
		{
			m->tracks[index] = trackPair.second;
		}
	}

	spLog(Log::Develop, this) << "Import cache updated";
}
