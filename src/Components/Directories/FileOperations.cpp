/* FileOperations.cpp */

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

#include "FileOperations.h"
#include "FileOperationWorkerThread.h"

#include "Interfaces/LibraryInfoAccessor.h"
#include "Components/Library/LocalLibrary.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Database/LibraryDatabase.h"
#include "Database/Connector.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Tagging/Tagging.h"

#include <QStringList>

struct FileOperations::Private
{
	LibraryInfoAccessor* libraryInfoAccessor;

	Private(LibraryInfoAccessor* libraryInfoAccessor) :
		libraryInfoAccessor {libraryInfoAccessor} {}
};

FileOperations::FileOperations(LibraryInfoAccessor* libraryInfoAccessor, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(libraryInfoAccessor);
}

FileOperations::~FileOperations() = default;

static void refreshLibraries(LibraryInfoAccessor* libraryInfoAccessor, const QList<LibraryId>& libraries)
{
	for(const auto& id : libraries)
	{
		auto* library = libraryInfoAccessor->libraryInstance(id);
		if(library)
		{
			library->refetch();
		}
	}
}

bool FileOperations::renamePath(const QString& path, const QString& newName)
{
	auto* t = new FileRenameThread(m->libraryInfoAccessor, path, newName, this);

	connect(t, &QThread::started, this, &FileOperations::sigStarted);
	connect(t, &QThread::finished, this, &FileOperations::moveThreadFinished);

	t->start();

	return true;
}

bool FileOperations::movePaths(const QStringList& paths, const QString& targetDir)
{
	auto* t = new FileMoveThread(m->libraryInfoAccessor, paths, targetDir, this);

	connect(t, &QThread::started, this, &FileOperations::sigStarted);
	connect(t, &QThread::finished, this, &FileOperations::moveThreadFinished);

	t->start();

	return true;
}

void FileOperations::moveThreadFinished()
{
	auto* thread = dynamic_cast<FileOperationThread*>(sender());

	refreshLibraries(m->libraryInfoAccessor, thread->sourceIds());
	refreshLibraries(m->libraryInfoAccessor, thread->targetIds());

	thread->deleteLater();
	emit sigFinished();
}

bool FileOperations::copyPaths(const QStringList& paths, const QString& targetDir)
{
	auto* t = new FileCopyThread(m->libraryInfoAccessor, paths, targetDir, this);

	connect(t, &QThread::started, this, &FileOperations::sigStarted);
	connect(t, &QThread::finished, this, &FileOperations::copyThreadFinished);

	t->start();

	return true;
}

void FileOperations::copyThreadFinished()
{
	auto* thread = dynamic_cast<FileOperationThread*>(sender());
	refreshLibraries(m->libraryInfoAccessor, thread->targetIds());

	thread->deleteLater();
	emit sigFinished();
}

bool FileOperations::deletePaths(const QStringList& paths)
{
	spLog(Log::Info, this) << "Try to delete " << paths;

	auto* t = new FileDeleteThread(m->libraryInfoAccessor, paths, this);
	connect(t, &QThread::started, this, &FileOperations::sigStarted);
	connect(t, &QThread::finished, this, &FileOperations::deleteThreadFinished);

	t->start();

	return true;
}

void FileOperations::deleteThreadFinished()
{
	auto* thread = dynamic_cast<FileOperationThread*>(sender());
	refreshLibraries(m->libraryInfoAccessor, thread->sourceIds());

	thread->deleteLater();
	emit sigFinished();
}

QStringList FileOperations::supportedReplacementTags()
{
	return QStringList
		{
			"<title>", "<album>", "<artist>", "<year>", "<bitrate>", "<tracknum>", "<disc>"
		};
}

static QString replaceTag(const QString& expression, const MetaData& track)
{
	QString ret(expression);
	ret.replace("<title>", track.title());
	ret.replace("<album>", track.album());
	ret.replace("<artist>", track.artist());
	ret.replace("<year>", QString::number(track.year()));
	ret.replace("<bitrate>", QString::number(track.bitrate() / 1000));

	auto trackNrString = QString::number(track.trackNumber());
	if(track.trackNumber() < 10)
	{
		trackNrString.prepend("0");
	}

	ret.replace("<tracknum>", trackNrString);
	ret.replace("<disc>", QString::number(int(track.discnumber())));

	return ret;
}

static QString incrementFilename(const QString& filename)
{
	if(!Util::File::exists(filename))
	{
		return filename;
	}

	auto[dir, pureFilename] = Util::File::splitFilename(filename);
	const auto extension = Util::File::getFileExtension(filename);

	for(int i = 1; i < 1000; i++)
	{
		const auto pureNewName = QString("%1-%2").arg(pureFilename).arg(i);
		const auto fullNewName = QString("%1/%2.%3").arg(dir).arg(pureNewName).arg(extension);
		if(!Util::File::exists(fullNewName))
		{
			return Util::File::cleanFilename(fullNewName);
		}
	}

	return QString();
}

bool FileOperations::renameByExpression(const QString& originalFilepath, const QString& expression) const
{
	auto* db = DB::Connector::instance();
	auto* libraryDb = db->libraryDatabase(-1, db->databaseId());

	auto originalTrack = libraryDb->getTrackByPath(originalFilepath);
	if(originalTrack.id() < 0)
	{
		Tagging::Utils::getMetaDataOfFile(originalTrack);
	}

	const auto pureFilename = replaceTag(expression, originalTrack);
	if(pureFilename.isEmpty())
	{
		spLog(Log::Error, this) << "Target filename is empty";
		return false;
	}

	if(pureFilename.contains("<") || pureFilename.contains(">"))
	{
		spLog(Log::Error, this) << "<, > are not allowed. Maybe an invalid tag was specified?";
		return false;
	}

	const auto dir = Util::File::getParentDirectory(originalTrack.filepath());
	const auto ext = Util::File::getFileExtension(originalTrack.filepath());

	auto fullNewName = QString("%1/%2.%3").arg(dir).arg(pureFilename).arg(ext);
	fullNewName = incrementFilename(fullNewName);

	if(originalTrack.id() < 0)
	{
		return Util::File::renameFile(fullNewName, originalFilepath);
	}

	else
	{
		auto success = Util::File::renameFile(originalFilepath, fullNewName);
		if(success)
		{
			auto newTrack = originalTrack;
			newTrack.setFilepath(fullNewName);

			success = libraryDb->updateTrack(newTrack);
			if(!success)
			{
				Util::File::renameFile(fullNewName, originalFilepath);
			}

			else
			{
				auto* changeNotifier = Tagging::ChangeNotifier::instance();
				changeNotifier->changeMetadata
					(
						QList<MetaDataPair> {MetaDataPair(originalTrack, newTrack)}
					);
			}
		}

		return success;
	}
}
