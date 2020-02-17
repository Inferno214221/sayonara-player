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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QCryptographicHash>

namespace Algorithm=::Util::Algorithm;

QString Util::File::cleanFilename(const QString& path)
{
	const QChar sep = QDir::separator();
	QString ret = path;

	QStringList forbiddenStrings
	{
		"\\",
		"/./",
		"//" // this must be at last place
	};

	for(const QString& forbidden : forbiddenStrings)
	{
		while(ret.contains(forbidden))
		{
			ret.replace(forbidden, sep);
		}
	}

	if(ret.endsWith(sep) && !QDir(ret).isRoot())
	{
		ret.remove(ret.size() - 1, 1);
	}

	return ret;
}

void Util::File::removeFilesInDirectory(const QString& dir_name)
{
	removeFilesInDirectory(dir_name, QStringList());
}

void Util::File::removeFilesInDirectory(const QString& dir_name, const QStringList& filters)
{
	if(dir_name.contains("..")){
		return;
	}

	bool success;
	QDir dir(dir_name);
	dir.setNameFilters(filters);

	QFileInfoList info_lst = dir.entryInfoList
	(
		QDir::Filters(QDir::System | QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)
	);

	for(const QFileInfo& info : info_lst)
	{
		QString path = info.absoluteFilePath();

		if(info.isSymLink())
		{
			QFile::remove(path);
		}

		else if(info.isDir())
		{
			removeFilesInDirectory(path);
			QDir().rmdir(path);
		}

		else if(info.isFile())
		{
			QFile::remove(path);
		}
	}

	QDir d = QDir::root();
	if(dir_name.contains(::Util::sayonaraPath()))
	{
		success = d.rmdir(dir_name);
		if(!success){
			spLog(Log::Warning, "FileUtils") << "Could not remove dir " << dir_name;
		}
	}
}


void Util::File::deleteFiles(const QStringList& paths)
{
	if(paths.isEmpty()){
		return;
	}

	spLog(Log::Develop, "Util::File") << "I will delete " << paths;
	QStringList sorted_paths = paths;
	Algorithm::sort(sorted_paths, [](const QString& str1, const QString& str2){
		return (str1.size() > str2.size());
	});

	for(const QString& path : Algorithm::AsConst(sorted_paths))
	{
		if(path.contains("..")){
			continue;
		}

		QFileInfo info(path);
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

		else {
			QFile::remove(path);
		}
	}
}


QString Util::File::getParentDirectory(const QString& filename)
{
	QString ret = cleanFilename(filename);
	int lastIndex = ret.lastIndexOf(QDir::separator());

	if(lastIndex > 0){
		return ret.left(lastIndex);
	}

	else if(lastIndex == 0)
	{
		return QDir::rootPath();
	}

	return ret;
}


QString Util::File::getFilenameOfPath(const QString& path)
{
	QString ret = cleanFilename(path);
	int last_idx = ret.lastIndexOf(QDir::separator());

	if(last_idx >= 0){
		return ret.mid(last_idx + 1);
	}

	return "";
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
	QStringList folders;
	for(const QString& file : files)
	{
		QString folder = getParentDirectory(file);
		if(!folders.contains(folder)){
			folders << folder;
		}
	}

	return folders;
}


QString Util::File::getAbsoluteFilename(const QString& filename)
{
	QString f, d;
	QString re_str = QString("(.*)") + QDir::separator() + "(.+)";
	QRegExp re(re_str);
	if(re.indexIn(filename) >= 0){
		d = re.cap(1);
		f = re.cap(2);
		QDir dir(d);
		return dir.absoluteFilePath(f);
	}

	return cleanFilename(filename);
}


QString Util::File::getFilesizeString(uint64_t filesize)
{
	uint64_t kb = 1 << 10;  // 1024
	uint64_t mb = kb << 10;
	uint64_t gb = mb << 10;

	QString size;
	if(filesize > gb) {
		size = QString::number(filesize / gb) + "." + QString::number((filesize / mb) % gb).left(2)  + " GB";
	}

	else if (filesize > mb) {
		size = QString::number(filesize / mb) + "." + QString::number((filesize / kb) % mb).left(2)  + " MB";
	}

	else {
		size = QString::number( filesize / kb) + " KB";
	}

	return size;
}


bool Util::File::isUrl(const QString& str)
{
	QStringList urls = {"file", "smb"};
	QString l = str.toLower().trimmed();

	if(isWWW(str)){
		return true;
	}

	return Algorithm::contains(urls, [&l](const QString& w){
		return l.startsWith(w + "://");
	});
}


bool Util::File::isWWW(const QString& str)
{
	QStringList www = {"http", "https", "ftp", "itpc", "feed"};
	QString l = str.toLower().trimmed();

	return Algorithm::contains(www, [&l](const QString& w){
		return l.startsWith(w + "://");
	});
}

bool Util::File::isDir(const QString& filename)
{
	if(!exists(filename)) return false;
	QFileInfo fileinfo(filename);
	return fileinfo.isDir();
}

bool Util::File::isFile(const QString& filename)
{
	if(!exists(filename)) return false;
	QFileInfo fileinfo(filename);
	return fileinfo.isFile();
}


bool Util::File::isSoundFile(const QString& filename)
{
	QStringList exts = Util::soundfileExtensions(true);

	return Algorithm::contains(exts, [&filename](const QString& ext)
	{
		return (filename.endsWith(ext.rightRef(4), Qt::CaseInsensitive));
	});
}


bool Util::File::isPlaylistFile(const QString& filename)
{
	QStringList exts = Util::playlistExtensions(true);

	return Algorithm::contains(exts, [&filename](const QString& ext)
	{
		return (filename.endsWith(ext.rightRef(4), Qt::CaseInsensitive));
	});
}

bool Util::File::isImageFile(const QString& filename)
{
	QStringList exts = Util::imageExtensions(true);

	return Algorithm::contains(exts, [&filename](const QString& ext)
	{
		return (filename.endsWith(ext.rightRef(4), Qt::CaseInsensitive));
	});
}


bool Util::File::createDirectories(const QString& path)
{
	if(exists(path)){
		return true;
	}

	QString cleaned_path = cleanFilename(path);
	const QStringList paths = cleaned_path.split(QDir::separator());
	QDir dir;
	if( isAbsolute(cleaned_path) ){
		dir = QDir::root();
	}

	else{
		dir = QDir(".");
	}

	for(const QString& p : paths)
	{
		QString abs_path = dir.absoluteFilePath(p);

		if(exists(abs_path)){
			dir.cd(p);
			continue;
		}

		bool success = dir.mkdir(p);
		if(!success){
			return false;
		}

		dir.cd(p);
	}

	return true;
}


bool Util::File::isAbsolute(const QString& filename)
{
	QDir dir(filename);
	return dir.isAbsolute();
}


bool Util::File::writeFile(const QByteArray& arr, const QString& filename)
{
	QFile f(filename);
	if(!f.open(QFile::WriteOnly)){
		return false;
	}

	int64_t bytes_written = f.write(arr);

	f.close();

	return(bytes_written >= arr.size());
}


bool Util::File::readFileIntoByteArray(const QString& filename, QByteArray& content)
{
	QFile file(filename);
	content.clear();


	if(!file.open(QIODevice::ReadOnly)){
		return false;
	}

	while(!file.atEnd()){
		QByteArray arr = file.read(4096);
		content.append(arr);
	}

	file.close();

	return (content.size() > 0);
}


bool Util::File::readFileIntoString(const QString& filename, QString& content)
{
	QFile file(filename);
	content.clear();
	if(!file.open(QIODevice::ReadOnly)) {
		return false;
	}

	while (!file.atEnd()) {
		content.append(QString::fromUtf8(file.readLine()));
	}

	file.close();

	if(content.size() > 0 ) {
		return true;
	}

	return false;
}


QString Util::File::getFileExtension(const QString& filename)
{
	int last_dot = filename.lastIndexOf(".");
	if(last_dot < 0){
		return QString("");
	}

	return filename.mid(last_dot + 1);
}


bool Util::File::checkFile(const QString& filepath)
{
	if(isWWW(filepath)){
		return true;
	}

	return exists(filepath);
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
			QDir d1(dir1);
			bool up = d1.cdUp();
			if(!up){
				return "";
			}
			dir1 = d1.absolutePath();
		}

		while(dir2.size() > dir1.size())
		{
			QDir d2(dir2);
			bool up = d2.cdUp();
			if(!up){
				return "";
			}

			dir2 = d2.absolutePath();
		}
	}

	return dir1;
}

QString Util::File::getCommonDirectory(const QStringList& paths)
{
	if(paths.isEmpty()){
		return QDir::rootPath();
	}

	if(paths.size() == 1){
		return QDir(paths[0]).absolutePath();
	}

	QString ret;
	QStringList absolute_paths;
	for(const QString& path : paths)
	{
		QString filename = getAbsoluteFilename(path);
		QFileInfo info(filename);
		if(info.isFile()){
			QDir d(filename);
			absolute_paths << d.absolutePath();
		}

		else if(info.isDir()){
			absolute_paths << getAbsoluteFilename(filename);
		}

		else if(info.isRoot()){
			return QDir::rootPath();
		}
	}

	if(absolute_paths.isEmpty()){
		return QDir::rootPath();
	}

	if(absolute_paths.size() == 1){
		return absolute_paths[0];
	}

	ret = absolute_paths[0];

	for(const QString& absolute_path : Algorithm::AsConst(absolute_paths))
	{
		ret = getCommonDirectory(ret, absolute_path);
	}

	return ret;
}

QStringList Util::File::splitDirectories(const QString& path)
{
	QStringList ret;

	QString currentDirectory;
	QFileInfo fi(path);

	if(fi.isFile()) {
		QString filename;
		splitFilename(path, currentDirectory, filename);
		ret << filename;
	}

	else {
		currentDirectory = path;
	}

	while(!currentDirectory.isEmpty())
	{
		QString parent_dir, last_dir;
		splitFilename(currentDirectory, parent_dir, last_dir);

		if(!last_dir.isEmpty()) {
			ret.push_front(last_dir);
		}

		currentDirectory = parent_dir;

		if(QDir(currentDirectory).isRoot()){
			break;
		}
	}

	return ret;
}


bool Util::File::createDir(const QString& dir_name)
{
	if(QDir(dir_name).exists()){
		return true;
	}

	return QDir().mkdir(dir_name);
}


bool Util::File::copyDir(const QString& sourceDirectory, const QString& targetDirectory, QString& new_filename)
{
	new_filename.clear();

	if(!canCopyDir(sourceDirectory, targetDirectory)){
		return false;
	}

	spLog(Log::Debug, "File") << "Copy " << sourceDirectory << " to " << targetDirectory;
	spLog(Log::Debug, "File") << "Create dir: " << targetDirectory;
	if(!createDir(targetDirectory)){
		return false;
	}

	const QDir src(sourceDirectory);
	QString copy_to = targetDirectory + "/" + src.dirName();

	spLog(Log::Debug, "File") << "Create dir: " << copy_to;
	if(!createDir(copy_to)) {
		return false;
	}

	QFileInfoList src_infos	= src.entryInfoList(QStringList(), (QDir::Files | QDir::Dirs | QDir::Filter::NoDotAndDotDot));
	for(const QFileInfo& info : src_infos)
	{
		if(info.isDir())
		{
			QString nf;
			bool success = copyDir(info.filePath(), copy_to, nf);
			if(!success){
				return false;
			}
		}

		else
		{
			QString old_filename = info.filePath();
			new_filename = old_filename;

			new_filename.remove(sourceDirectory);
			new_filename.prepend(copy_to);

			QFile f(old_filename);
			spLog(Log::Debug, "File") << "Copy file " << old_filename << " to " << new_filename;
			f.copy(new_filename);
		}
	}

	new_filename = QDir(targetDirectory).filePath(src.dirName());

	return true;
}

bool Util::File::moveDir(const QString& sourceDirectory, const QString& targetDirectory, QString& new_filename)
{
	new_filename = QString();

	QDir s(sourceDirectory);
	QDir t(targetDirectory);

	bool success = renameDir(sourceDirectory, t.filePath(s.dirName()));

	if(success) {
		new_filename = t.filePath(s.dirName());
	}

	return success;
}

bool Util::File::canCopyDir(const QString& src_dir, const QString& targetDirectory)
{
	if(src_dir.isEmpty()){
		return false;
	}

	if(targetDirectory.isEmpty()){
		return false;
	}

	if(QString(targetDirectory + "/").startsWith(src_dir + "/")){
		return false;
	}

	if(!exists(src_dir)){
		return false;
	}

	return true;
}

bool Util::File::renameDir(const QString& src_dir, const QString& new_dir)
{
	return QDir().rename(src_dir, new_dir);
}

QByteArray Util::File::getMD5Sum(const QString& filename)
{
	QFile f(filename);
	if (f.open(QFile::ReadOnly))
	{
		QCryptographicHash hash(QCryptographicHash::Md5);
		if (hash.addData(&f)) {
			return hash.result().toHex();
		}
	}

	return QByteArray();
}

bool Util::File::moveFiles(const QStringList& files, const QString& dir, QStringList& new_names)
{
	bool success = true;
	for(const QString& file : files)
	{
		QString new_name;
		success = moveFile(file, dir, new_name);
		if(!success){
			continue;
		}

		new_names << new_name;
	}

	return success;
}

bool Util::File::renameFile(const QString& old_name, const QString& new_name)
{
	QFileInfo info(old_name);
	if(!info.isFile()){
		return false;
	}

	QFile f(old_name);
	return f.rename(new_name);
}

bool Util::File::copyFiles(const QStringList& files, const QString& dir, QStringList& new_names)
{
	bool success = true;
	for(const QString& file : files)
	{
		QString new_name;
		success = copyFile(file, dir, new_name);
		if(!success){
			continue;
		}

		new_names << new_name;
	}

	return success;
}

bool Util::File::moveFile(const QString& file, const QString& dir, QString& new_name)
{
	bool success = copyFile(file, dir, new_name);
	if(success)
	{
		QFile f(file);
		return f.remove();
	}

	return false;
}

bool Util::File::copyFile(const QString& file, const QString& dir, QString& new_name)
{
	new_name.clear();

	QFileInfo di(dir);
	if(!di.isDir()){
		return false;
	}

	QFileInfo fi(file);
	if(!fi.isFile()){
		return false;
	}

	QDir d(dir);
	QFile f(file);

	QString pure_filename = getFilenameOfPath(file);
	new_name = d.absoluteFilePath(pure_filename);

	bool success = f.copy(new_name);
	return success;
}

bool Util::File::exists(const QString& filename)
{
	if(filename.isEmpty()){
		return false;
	}

	return QFile::exists(filename);
}

bool Util::File::isInSayonaraDir(const QString& path)
{
	QDir sayonara_dir(sayonaraPath());
	QDir p(path);

	while(!p.isRoot())
	{
		bool b = p.cdUp();
		if(!b){
			return false;
		}

		if(p == sayonara_dir){
			return true;
		}
	}

	return false;
}

bool Util::File::isSamePath(const QString& filename1, const QString& filename2)
{
	return (cleanFilename(filename1) == cleanFilename(filename2));
}

bool Util::File::isSubdir(const QString& dir, const QString& parentDir)
{
	if(isSamePath(dir, parentDir)){
		return false;
	}

	const QString cleanedDir = cleanFilename(dir);
	const QString cleanedParentDir = cleanFilename(parentDir);

	if(cleanedDir.isEmpty() || cleanedParentDir.isEmpty()){
		return false;
	}

	const QFileInfo info(cleanedDir);

	QDir d1(cleanedDir);
	if(info.exists() && info.isFile()){
		d1 = QDir(Util::File::getParentDirectory(cleanedDir));
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
