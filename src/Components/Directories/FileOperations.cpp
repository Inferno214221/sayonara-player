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

#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Library/LocalLibrary.h"

#include "Database/LibraryDatabase.h"
#include "Database/Connector.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Tagging/Tagging.h"

#include <QStringList>

namespace Algorithm=Util::Algorithm;

FileOperations::FileOperations(QObject* parent) :
	QObject(parent)
{}

FileOperations::~FileOperations() = default;

static void refreshLibraries(const QList<LibraryId>& libraries)
{
	for(LibraryId id : libraries)
	{
		LocalLibrary* library = Library::Manager::instance()->libraryInstance(id);
		if(library) {
			library->refetch();
		}
	}
}

bool FileOperations::renamePath(const QString& path, const QString& newName)
{
	auto* t = new FileRenameThread(path, newName, this);

	connect(t, &QThread::started, this, &FileOperations::sigStarted);
	connect(t, &QThread::finished, this, &FileOperations::moveThreadFinished);

	t->start();

	return true;
}

bool FileOperations::movePaths(const QStringList& paths, const QString& targetDir)
{
	auto* t = new FileMoveThread(paths, targetDir, this);

	connect(t, &QThread::started, this, &FileOperations::sigStarted);
	connect(t, &QThread::finished, this, &FileOperations::moveThreadFinished);

	t->start();

	return true;
}

void FileOperations::moveThreadFinished()
{
	auto* thread = static_cast<FileOperationThread*>(sender());

	refreshLibraries(thread->sourceIds());
	refreshLibraries(thread->targetIds());

	thread->deleteLater();
	emit sigFinished();
}

bool FileOperations::copyPaths(const QStringList& paths, const QString& targetDir)
{
	auto* t = new FileCopyThread(paths, targetDir, this);

	connect(t, &QThread::started, this, &FileOperations::sigStarted);
	connect(t, &QThread::finished, this, &FileOperations::copyThreadFinished);

	t->start();

	return true;
}

void FileOperations::copyThreadFinished()
{
	auto* thread = static_cast<FileOperationThread*>(sender());
	refreshLibraries(thread->targetIds());

	thread->deleteLater();
	emit sigFinished();
}

bool FileOperations::deletePaths(const QStringList& paths)
{
	spLog(Log::Info, this) << "Try to delete " << paths;

	auto* t = new FileDeleteThread(paths, this);
	connect(t, &QThread::started, this, &FileOperations::sigStarted);
	connect(t, &QThread::finished, this, &FileOperations::deleteThreadFinished);

	t->start();

	return true;
}

void FileOperations::deleteThreadFinished()
{
	auto* thread = static_cast<FileOperationThread*>(sender());
	refreshLibraries(thread->sourceIds());

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

static QString replaceTag(const QString& expression, const MetaData& md)
{
	QString ret(expression);
	ret.replace("<title>", md.title());
	ret.replace("<album>", md.album());
	ret.replace("<artist>", md.artist());
	ret.replace("<year>", QString::number(md.year()));
	ret.replace("<bitrate>", QString::number(md.bitrate() / 1000));

	QString s_track_nr = QString::number(md.trackNumber());
	if(md.trackNumber() < 10)
	{
		s_track_nr.prepend("0");
	}

	ret.replace("<tracknum>", s_track_nr);
	ret.replace("<disc>", QString::number(int(md.discnumber())));

	return ret;
}

static QString incrementFilename(const QString& filename)
{
	if(!Util::File::exists(filename)){
		return filename;
	}

	QString dir, pure_filename;
	Util::File::splitFilename(filename, dir, pure_filename);

	const QString ext = Util::File::getFileExtension(filename);

	for(int i=1; i<1000; i++)
	{
		QString pure_new_name_nr = pure_filename + "-" + QString::number(i);
		QString full_new_name_nr = dir + "/" + pure_new_name_nr + "." + ext;
		if(!Util::File::exists(full_new_name_nr))
		{
			return full_new_name_nr;
		}
	}

	return QString();
}

bool FileOperations::renameByExpression(const QString& old_name, const QString& expression)
{
	auto* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->libraryDatabase(-1, db->databaseId());

	MetaData md = library_db->getTrackByPath(Util::File::cleanFilename(old_name));
	if(md.id() < 0) {
		Tagging::Utils::getMetaDataOfFile(md);
	}

	const QString pureFilename = replaceTag(expression, md);
	if(pureFilename.isEmpty()) {
		spLog(Log::Error, this) << "Target filename is empty";
		return false;
	}

	if(pureFilename.contains("<") || pureFilename.contains(">")){
		spLog(Log::Error, this) << "<, > are not allowed. Maybe an invalid tag was specified?";
		return false;
	}

	const QString dir = Util::File::getParentDirectory(md.filepath());
	const QString ext = Util::File::getFileExtension(md.filepath());

	QString fullNewName = dir + "/" + pureFilename + "." + ext;
	fullNewName = incrementFilename(fullNewName);

	if(md.id() < 0) {
		return Util::File::renameFile(fullNewName, old_name);
	}

	else
	{
		bool success = Util::File::renameFile(old_name, fullNewName);
		if(success)
		{
			md.setFilepath(fullNewName);
			success = library_db->updateTrack(md);
			if(!success){
				Util::File::renameFile(fullNewName, old_name);
			}
		}

		return success;
	}
}
