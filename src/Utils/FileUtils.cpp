/* FileUtils.cpp */

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

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QCryptographicHash>

namespace Algorithm = ::Util::Algorithm;

QString Util::File::cleanFilename(const QString& path)
{
	return (path.trimmed().isEmpty()) ?
	       QString("") :
	       QDir::cleanPath(path);
}

void Util::File::removeFilesInDirectory(const QString& dirName)
{
	removeFilesInDirectory(dirName, QStringList());
}

void Util::File::removeFilesInDirectory(const QString& dirName, const QStringList& filters)
{
	if(dirName.contains(".."))
	{
		return;
	}

	auto dir = QDir(dirName);
	dir.setNameFilters(filters);

	const auto fileInfoList = dir.entryInfoList
		(
			QDir::Filters(QDir::System | QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)
		);

	for(const auto& fileInfo : fileInfoList)
	{
		const auto path = fileInfo.absoluteFilePath();

		if(fileInfo.isSymLink())
		{
			QFile::remove(path);
		}

		else if(fileInfo.isDir())
		{
			removeFilesInDirectory(path);
			QDir().rmdir(path);
		}

		else if(fileInfo.isFile())
		{
			QFile::remove(path);
		}
	}
}

void Util::File::deleteFiles(const QStringList& paths)
{
	if(paths.isEmpty())
	{
		return;
	}

	spLog(Log::Develop, "Util::File") << "I will delete " << paths;
	auto sortedPaths = paths;
	Algorithm::sort(sortedPaths, [](const QString& str1, const QString& str2) {
		return (str1.size() > str2.size());
	});

	for(const auto& path : Algorithm::AsConst(sortedPaths))
	{
		if(path.contains(".."))
		{
			continue;
		}

		const auto info = QFileInfo(path);
		if(!info.exists())
		{
			continue;
		}

		if(info.isSymLink())
		{
			QFile::remove(info.absoluteFilePath());
		}

		else if(info.isDir())
		{
			removeFilesInDirectory(path);
			QDir().rmdir(path);
		}

		else
		{
			QFile::remove(path);
		}
	}
}

QString Util::File::getParentDirectory(const QString& filename)
{
	const auto cleaned = cleanFilename(filename);
	const auto index = cleaned.lastIndexOf(QDir::separator());

	return (index > 0)
	       ? cleanFilename(cleaned.left(index))
	       : QDir::rootPath();
}

QString Util::File::getFilenameOfPath(const QString& path)
{
	return QDir(cleanFilename(path)).dirName();
}

void Util::File::splitFilename(const QString& src, QString& path, QString& filename)
{
	path = Util::File::getParentDirectory(src);
	filename = Util::File::getFilenameOfPath(src);
}

std::pair<QString, QString> Util::File::splitFilename(const QString& src)
{
	std::pair<QString, QString> ret;
	splitFilename(src, ret.first, ret.second);

	return ret;
}

QStringList Util::File::getParentDirectories(const QStringList& files)
{
	Util::Set<QString> folders;
	for(const auto& file : files)
	{
		const auto folder = getParentDirectory(file);
		folders.insert(folder);
	}

	return QStringList(folders.toList());
}

QString Util::File::getAbsoluteFilename(const QString& filename)
{
	return QDir(filename).absolutePath();
}

QString Util::File::getFilesizeString(Filesize filesize)
{
	Filesize kb = 1 << 10;  // 1024
	Filesize mb = kb << 10;
	Filesize gb = mb << 10;

	QString size;
	if(filesize > gb)
	{
		size =
			QString::number(filesize / gb) + "." + QString::number((filesize / mb) % gb).left(2) +
			" " + Lang::get(Lang::GB);
	}

	else if(filesize > mb)
	{
		size =
			QString::number(filesize / mb) + "." + QString::number((filesize / kb) % mb).left(2) +
			" " + Lang::get(Lang::MB);
	}

	else
	{
		size = QString::number(filesize / kb) + " " + +" " + Lang::get(Lang::KB);
	}

	return size;
}

bool Util::File::isUrl(const QString& str)
{
	return QUrl(str).isValid();
}

bool Util::File::isWWW(const QString& str)
{
	const auto schemes = QStringList{"http", "https", "ftp", "itpc", "feed"};
	const auto url = QUrl(str);
	const auto scheme = url.scheme().toLower();

	return (url.isValid() && schemes.contains(scheme));
}

bool Util::File::isDir(const QString& filename)
{
	return QFileInfo(filename).isDir();
}

bool Util::File::isFile(const QString& filename)
{
	return QFileInfo(filename).isFile();
}

bool Util::File::isSoundFile(const QString& filename)
{
	const auto extensions = Util::soundfileExtensions(true);
	return Algorithm::contains(extensions, [&filename](const auto& extension) {
		return (filename.endsWith(extension.rightRef(4), Qt::CaseInsensitive));
	});
}

bool Util::File::isPlaylistFile(const QString& filename)
{
	const auto extensions = Util::playlistExtensions(true);
	return Algorithm::contains(extensions, [&filename](const auto& extension) {
		return (filename.endsWith(extension.rightRef(4), Qt::CaseInsensitive));
	});
}

bool Util::File::isImageFile(const QString& filename)
{
	const auto extensions = Util::imageExtensions(true);
	return Algorithm::contains(extensions, [&filename](const auto& extension) {
		return (filename.endsWith(extension.rightRef(4), Qt::CaseInsensitive));
	});
}

bool Util::File::createDirectories(const QString& path)
{
	return QDir().mkpath(path);
}

bool Util::File::isAbsolute(const QString& filename)
{
	return QDir::isAbsolutePath(filename);
}

bool Util::File::writeFile(const QByteArray& arr, const QString& filename)
{
	QFile f(filename);
	if(!f.open(QFile::WriteOnly))
	{
		return false;
	}

	const auto bytesWritten = f.write(arr);

	f.close();

	return (bytesWritten >= arr.size());
}

bool Util::File::readFileIntoByteArray(const QString& filename, QByteArray& content)
{
	QFile file(filename);
	content.clear();

	if(!file.open(QIODevice::ReadOnly))
	{
		return false;
	}

	while(!file.atEnd())
	{
		const auto arr = file.read(4096);
		content.append(arr);
	}

	file.close();

	return (content.size() > 0);
}

bool Util::File::readFileIntoString(const QString& filename, QString& content)
{
	QFile file(filename);
	content.clear();
	if(!file.open(QIODevice::ReadOnly))
	{
		return false;
	}

	while(!file.atEnd())
	{
		content.append(QString::fromUtf8(file.readLine()));
	}

	file.close();

	return !content.isEmpty();
}

// hallo.txta
// . = 5
QString Util::File::getFileExtension(const QString& filename)
{
	const auto lastDot = filename.lastIndexOf(".");
	const auto lastSep = filename.lastIndexOf(QDir::separator());

	if(lastDot < 0 && lastDot < lastSep)
	{
		return QString();
	}

	const auto ext = filename.mid(lastDot + 1);
	return (ext.size() > 4) ? QString() : ext;
}

bool Util::File::checkFile(const QString& filepath)
{
	return (isWWW(filepath) || exists(filepath));
}

bool Util::File::createSymlink(const QString& source, const QString& target)
{
	QFile f(source);
	return f.link(target);
}

QString Util::File::getCommonDirectory(QString dir1, QString dir2)
{
	while(dir1.compare(dir2) != 0)
	{
		while(dir1.size() > dir2.size())
		{
			auto d1 = QDir(dir1);
			const auto canUp = d1.cdUp();
			if(!canUp)
			{
				return QDir::rootPath();
			}
			dir1 = d1.absolutePath();
		}

		while(dir2.size() > dir1.size())
		{
			auto d2 = QDir(dir2);
			const auto canUp = d2.cdUp();
			if(!canUp)
			{
				return QDir::rootPath();
			}

			dir2 = d2.absolutePath();
		}
	}

	return dir1;
}

QString Util::File::getCommonDirectory(const QStringList& paths)
{
	if(paths.isEmpty())
	{
		return QDir::rootPath();
	}

	if(paths.size() == 1)
	{
		return QDir(paths[0]).absolutePath();
	}

	QString ret;
	QStringList absolutePaths;
	for(const auto& path : paths)
	{
		const auto filename = getAbsoluteFilename(path);
		const auto info = QFileInfo(filename);

		if(info.isFile() || info.isDir())
		{
			absolutePaths << QDir(filename).absolutePath();
		}

		else if(info.isRoot())
		{
			return QDir::rootPath();
		}
	}

	if(absolutePaths.isEmpty())
	{
		return QDir::rootPath();
	}

	if(absolutePaths.size() == 1)
	{
		return absolutePaths[0];
	}

	ret = absolutePaths[0];

	for(const auto& absolutePath : Algorithm::AsConst(absolutePaths))
	{
		ret = getCommonDirectory(ret, absolutePath);
	}

	return ret;
}

QStringList Util::File::splitDirectories(const QString& path)
{
	auto ret = path.split(QDir::separator());
	ret.removeAll("");

	return ret;
}

bool Util::File::createDir(const QString& dirName)
{
	return (QDir(dirName).exists() || QDir().mkdir(dirName));
}

bool Util::File::copyDir(const QString& sourceDirectory, const QString& targetDirectory,
                         QString& newFilename)
{
	newFilename.clear();

	if(!canCopyDir(sourceDirectory, targetDirectory))
	{
		return false;
	}

	spLog(Log::Debug, "File") << "Copy " << sourceDirectory << " to " << targetDirectory;
	spLog(Log::Debug, "File") << "Create dir: " << targetDirectory;
	if(!createDir(targetDirectory))
	{
		return false;
	}

	const auto src = QDir(sourceDirectory);
	const auto copyTo = QDir(targetDirectory).absoluteFilePath(src.dirName());

	spLog(Log::Debug, "File") << "Create dir: " << copyTo;
	if(!createDir(copyTo))
	{
		return false;
	}

	const auto srcInfos = src.entryInfoList(QStringList(),
	                                        (QDir::Files | QDir::Dirs |
	                                         QDir::Filter::NoDotAndDotDot));
	for(const auto& info : srcInfos)
	{
		if(info.isDir())
		{
			QString nf;
			const auto success = copyDir(info.filePath(), copyTo, nf);
			if(!success)
			{
				return false;
			}
		}

		else if(info.isSymLink())
		{
			newFilename = info.filePath();
			newFilename.remove(sourceDirectory);
			newFilename.prepend(copyTo);

			const auto target = info.symLinkTarget();
			createSymlink(info.symLinkTarget(), newFilename);
		}

		else
		{
			const auto oldFilename = info.filePath();

			newFilename = oldFilename;
			newFilename.remove(sourceDirectory);
			newFilename.prepend(copyTo);

			auto file = QFile(oldFilename);
			spLog(Log::Debug, "File") << "Copy file " << oldFilename << " to " << newFilename;
			file.copy(newFilename);
		}
	}

	newFilename = QDir(targetDirectory).filePath(src.dirName());

	return true;
}

bool Util::File::moveDir(const QString& sourceDirectory, const QString& targetDirectory,
                         QString& newFilename)
{
	newFilename = QString();

	QDir s(sourceDirectory);
	QDir t(targetDirectory);

	bool success = renameDir(sourceDirectory, t.filePath(s.dirName()));

	if(success)
	{
		newFilename = t.filePath(s.dirName());
	}

	return success;
}

bool Util::File::canCopyDir(const QString& srcDir, const QString& targetDirectory)
{
	if(srcDir.isEmpty())
	{
		return false;
	}

	if(targetDirectory.isEmpty())
	{
		return false;
	}

	if(QString(targetDirectory + "/").startsWith(srcDir + "/"))
	{
		return false;
	}

	if(!exists(srcDir))
	{
		return false;
	}

	return true;
}

bool Util::File::renameDir(const QString& srcDir, const QString& newDir)
{
	return QDir().rename(srcDir, newDir);
}

QByteArray Util::File::getMD5Sum(const QString& filename)
{
	QFile f(filename);
	if(f.open(QFile::ReadOnly))
	{
		QCryptographicHash hash(QCryptographicHash::Md5);
		if(hash.addData(&f))
		{
			return hash.result().toHex();
		}
	}

	return QByteArray();
}

bool Util::File::moveFiles(const QStringList& files, const QString& dir, QStringList& newNames)
{
	bool success = true;
	for(const auto& file : files)
	{
		QString newName;
		success = moveFile(file, dir, newName);
		if(!success)
		{
			continue;
		}

		newNames << newName;
	}

	return success;
}

bool Util::File::renameFile(const QString& oldName, const QString& newName)
{
	QFileInfo info(oldName);
	if(!info.isFile())
	{
		return false;
	}

	return QFile(oldName).rename(newName);
}

bool Util::File::copyFiles(const QStringList& files, const QString& dir, QStringList& newNames)
{
	bool success = true;
	for(const auto& file : files)
	{
		QString newName;
		success = copyFile(file, dir, newName);
		if(!success)
		{
			continue;
		}

		newNames << newName;
	}

	return success;
}

bool Util::File::moveFile(const QString& file, const QString& dir, QString& newName)
{
	if(copyFile(file, dir, newName))
	{
		return QFile(file).remove();
	}

	return false;
}

bool Util::File::copyFile(const QString& file, const QString& dir, QString& newName)
{
	newName.clear();

	if(!QFileInfo(dir).isDir())
	{
		return false;
	}

	if(!QFileInfo(file).isFile())
	{
		return false;
	}

	const auto pureFilename = getFilenameOfPath(file);
	newName = QDir(dir).absoluteFilePath(pureFilename);

	return QFile(file).copy(newName);
}

bool Util::File::exists(const QString& filename)
{
	return (!filename.isEmpty()) && QFile::exists(filename);
}

bool Util::File::isSamePath(const QString& filename1, const QString& filename2)
{
	const auto cleaned1 = cleanFilename(filename1);
	const auto cleaned2 = cleanFilename(filename2);

#ifdef Q_OS_UNIX
	return (cleaned1.compare(cleaned2) == 0);
#else
	return (cleaned1.compare(cleaned2, Qt::CaseInsensitive) == 0);
#endif
}

bool Util::File::isSubdir(const QString& dir, const QString& parentDir)
{
	if(isSamePath(dir, parentDir))
	{
		return false;
	}

	const auto cleanedDir = cleanFilename(dir);
	const auto cleanedParentDir = cleanFilename(parentDir);

	if(cleanedDir.isEmpty() || cleanedParentDir.isEmpty())
	{
		return false;
	}

	const QFileInfo info(cleanedDir);

	QDir d1(cleanedDir);
	if(info.exists() && info.isFile())
	{
		const auto d1String = Util::File::getParentDirectory(cleanedDir);
		if(isSamePath(d1String, parentDir))
		{
			return true;
		}

		d1 = QDir(d1String);

	}

	const QDir d2(cleanedParentDir);

	while(!d1.isRoot())
	{
		d1 = QDir(Util::File::getParentDirectory(d1.absolutePath()));
		if(isSamePath(d1.absolutePath(), d2.absolutePath()))
		{
			return true;
		}
	}

	return false;
}

QList<QChar> Util::File::invalidFilenameChars()
{
	return
		{
			'*', '?', '/', '\\'
		};
}
