/* MetaDataScanner.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "MetaDataScanner.h"

#include "Database/Connector.h"

#include "Utils/DirectoryReader.h"
#include "Utils/FileSystem.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Utils.h"

#include <QStringList>
#include <QDir>

using Directory::MetaDataScanner;

struct MetaDataScanner::Private
{
	QStringList files;
	MetaDataList tracks;

	bool recursive;

	Private(QStringList files, bool recursive) :
		files(std::move(files)),
		recursive(recursive) {}
};

MetaDataScanner::~MetaDataScanner()
{
	DB::Connector::instance()->closeDatabase();
}

MetaDataScanner::MetaDataScanner(const QStringList& files, bool recursive, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(files, recursive);
}

void MetaDataScanner::start()
{
	const auto fileSystem = Util::FileSystem::create();
	const auto tagReader = Tagging::TagReader::create();
	const auto directoryReader = Util::DirectoryReader::create(fileSystem, tagReader);
	if(!m->recursive)
	{
		m->tracks.clear();

		const auto extensions = QStringList() << Util::soundfileExtensions();
		for(const auto& path: m->files)
		{
			emit sigCurrentProcessedPathChanged(path);

			const auto files = directoryReader->scanFilesInDirectory(QDir(path), extensions);
			m->tracks << directoryReader->scanMetadata(files);
		}
	}

	else
	{
		m->tracks = directoryReader->scanMetadata(m->files);
	}

	emit sigFinished();
}

MetaDataList MetaDataScanner::metadata() const { return m->tracks; }

QStringList MetaDataScanner::files() const { return m->files; }
