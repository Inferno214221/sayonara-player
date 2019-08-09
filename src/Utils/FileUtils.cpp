/* FileUtils.cpp */

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

QString Util::File::clean_filename(const QString& path)
{
	QChar sep = QDir::separator();
	QString ret = path;
	while(ret.contains("/./") || ret.contains("\\.\\")){
		ret.replace("/./", sep);
		ret.replace("\\.\\", sep);
	}

	while(ret.contains("//") || ret.contains("\\\\")){
		ret.replace("//", sep);
		ret.replace("\\\\", sep);
	}

	ret.replace("\\", sep);

	if(ret.endsWith(sep))
	{
		ret.remove(ret.size() - 1, 1);
	}

	return ret;
}


QString Util::File::calc_file_extension(const QString& filename)
{
	return get_file_extension(filename);
}


void Util::File::remove_files_in_directory(const QString& dir_name)
{
	remove_files_in_directory(dir_name, QStringList());
}

void Util::File::remove_files_in_directory(const QString& dir_name, const QStringList& filters)
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
			remove_files_in_directory(path);
			QDir().rmdir(path);
		}

		else if(info.isFile())
		{
			QFile::remove(path);
		}
	}

	QDir d = QDir::root();
	if(dir_name.contains(::Util::sayonara_path()))
	{
		success = d.rmdir(dir_name);
		if(!success){
			sp_log(Log::Warning, "FileUtils") << "Could not remove dir " << dir_name;
		}
	}
}


void Util::File::delete_files(const QStringList& paths)
{
	if(paths.isEmpty()){
		return;
	}

	sp_log(Log::Develop, "Util::File") << "I will delete " << paths;
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
			remove_files_in_directory(path);
			QDir().rmdir(path);
		}

		else {
			QFile::remove(path);
		}
	}
}


QString Util::File::get_parent_directory(const QString& filename)
{
	QString ret = clean_filename(filename);
	int last_idx = ret.lastIndexOf(QDir::separator());

	if(last_idx >= 0){
		return ret.left(last_idx);
	}

	return ret;
}


QString Util::File::get_filename_of_path(const QString& path)
{
	QString ret = clean_filename(path);
	int last_idx = ret.lastIndexOf(QDir::separator());

	if(last_idx >= 0){
		return ret.mid(last_idx + 1);
	}

	return "";
}


void Util::File::split_filename(const QString& src, QString& path, QString& filename)
{
	path = Util::File::get_parent_directory(src);
	filename = Util::File::get_filename_of_path(src);
}


QStringList Util::File::get_parent_directories(const QStringList& files)
{
	QStringList folders;
	for(const QString& file : files)
	{
		QString folder = get_parent_directory(file);
		if(!folders.contains(folder)){
			folders << folder;
		}
	}

	return folders;
}


QString Util::File::get_absolute_filename(const QString& filename)
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

	return clean_filename(filename);
}


QString Util::File::calc_filesize_str(uint64_t filesize)
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


bool Util::File::is_url(const QString& str)
{
	QStringList urls = {"file", "smb"};
	QString l = str.toLower().trimmed();

	if(is_www(str)){
		return true;
	}

	return Algorithm::contains(urls, [&l](const QString& w){
		return l.startsWith(w + "://");
	});
}


bool Util::File::is_www(const QString& str)
{
	QStringList www = {"http", "https", "ftp", "itpc", "feed"};
	QString l = str.toLower().trimmed();

	return Algorithm::contains(www, [&l](const QString& w){
		return l.startsWith(w + "://");
	});
}

bool Util::File::is_dir(const QString& filename)
{
	if(!exists(filename)) return false;
	QFileInfo fileinfo(filename);
	return fileinfo.isDir();
}

bool Util::File::is_file(const QString& filename)
{
	if(!exists(filename)) return false;
	QFileInfo fileinfo(filename);
	return fileinfo.isFile();
}


bool Util::File::is_soundfile(const QString& filename)
{
	QStringList exts = Util::soundfile_extensions(true);

	return Algorithm::contains(exts, [&filename](const QString& ext)
	{
		return (filename.endsWith(ext.rightRef(4), Qt::CaseInsensitive));
	});
}


bool Util::File::is_playlistfile(const QString& filename)
{
	QStringList exts = Util::playlist_extensions(true);

	return Algorithm::contains(exts, [&filename](const QString& ext)
	{
		return (filename.endsWith(ext.rightRef(4), Qt::CaseInsensitive));
	});
}

bool Util::File::is_imagefile(const QString& filename)
{
	QStringList exts = Util::image_extensions(true);

	return Algorithm::contains(exts, [&filename](const QString& ext)
	{
		return (filename.endsWith(ext.rightRef(4), Qt::CaseInsensitive));
	});
}


bool Util::File::create_directories(const QString& path)
{
	if(exists(path)){
		return true;
	}

	QString cleaned_path = clean_filename(path);
	const QStringList paths = cleaned_path.split(QDir::separator());
	QDir dir;
	if( is_absolute(cleaned_path) ){
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


bool Util::File::is_absolute(const QString& filename)
{
	QDir dir(filename);
	return dir.isAbsolute();
}


bool Util::File::write_file(const QByteArray& arr, const QString& filename)
{
	QFile f(filename);
	if(!f.open(QFile::WriteOnly)){
		return false;
	}

	int64_t bytes_written = f.write(arr);

	f.close();

	return(bytes_written >= arr.size());
}


bool Util::File::read_file_into_byte_arr(const QString& filename, QByteArray& content)
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


bool Util::File::read_file_into_str(const QString& filename, QString& content)
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


QString Util::File::get_file_extension(const QString& filename)
{
	int last_dot = filename.lastIndexOf(".");
	if(last_dot < 0){
		return QString("");
	}

	return filename.mid(last_dot + 1);
}


bool Util::File::check_file(const QString& filepath)
{
	if(is_www(filepath)){
		return true;
	}

	return exists(filepath);
}

bool Util::File::create_symlink(const QString& source, const QString& target)
{
	QFile f(source);
	return f.link(target);
}

QString Util::File::get_common_directory(QString dir1, QString dir2)
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

QString Util::File::get_common_directory(const QStringList& paths)
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
		QString filename = get_absolute_filename(path);
		QFileInfo info(filename);
		if(info.isFile()){
			QDir d(filename);
			absolute_paths << d.absolutePath();
		}

		else if(info.isDir()){
			absolute_paths << get_absolute_filename(filename);
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
		ret = get_common_directory(ret, absolute_path);
	}

	return ret;
}

QStringList Util::File::split_directories(const QString& path)
{
	QStringList ret;

	QString current_dir;
	QFileInfo fi(path);

	if(fi.isDir()) {
		current_dir = path;
	}

	else if(fi.isFile()){
		QString filename;
		split_filename(path, current_dir, filename);
	}

	while(!QDir(current_dir).isRoot())
	{
		QString last_dir;
		QString parent_dir;
		split_filename(current_dir, parent_dir, last_dir);

		ret << last_dir;
		current_dir = parent_dir;
	}

	return ret;
}


bool Util::File::create_dir(const QString& dir_name)
{
	if(QDir(dir_name).exists()){
		return true;
	}

	return QDir().mkdir(dir_name);
}


bool Util::File::copy_dir(const QString& source_dir, const QString& target_dir)
{
	if(!can_copy_dir(source_dir, target_dir)){
		return false;
	}

	sp_log(Log::Debug, "File") << "Copy " << source_dir << " to " << target_dir;
	sp_log(Log::Debug, "File") << "Create dir: " << target_dir;
	if(!create_dir(target_dir)){
		return false;
	}

	QDir src(source_dir);
	QString copy_to = target_dir + "/" + src.dirName();

	sp_log(Log::Debug, "File") << "Create dir: " << copy_to;
	if(!create_dir(copy_to)) {
		return false;
	}

	QFileInfoList src_infos	= src.entryInfoList(QStringList(), (QDir::Files | QDir::Dirs | QDir::Filter::NoDotAndDotDot));
	for(const QFileInfo& info : src_infos)
	{
		if(info.isDir())
		{
			bool success = copy_dir(info.filePath(), copy_to);
			if(!success){
				return false;
			}
		}

		else
		{
			QString old_filename = info.filePath();
			QString new_filename(old_filename);

			new_filename.remove(source_dir);
			new_filename.prepend(copy_to);

			QFile f(old_filename);
			sp_log(Log::Debug, "File") << "Copy file " << old_filename << " to " << new_filename;
			f.copy(new_filename);
		}
	}

	return true;
}

QString Util::File::move_dir(const QString& source_dir, const QString& target_dir)
{
	QDir s(source_dir);
	QDir t(target_dir);

	bool success = rename_dir(source_dir, t.filePath(s.dirName()));

	if(success) {
		return t.filePath(s.dirName());
	}

	return QString();
}

bool Util::File::can_copy_dir(const QString& src_dir, const QString& target_dir)
{
	if(src_dir.isEmpty()){
		return false;
	}

	if(target_dir.isEmpty()){
		return false;
	}

	if(QString(target_dir + "/").startsWith(src_dir + "/")){
		return false;
	}

	if(!exists(src_dir)){
		return false;
	}

	return true;
}

bool Util::File::rename_dir(const QString& src_dir, const QString& new_dir)
{
	return QDir().rename(src_dir, new_dir);
}

QByteArray Util::File::calc_md5_sum(const QString& filename)
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

bool Util::File::move_files(const QStringList& files, const QString& dir)
{
	bool success = true;
	for(const QString& file : files){
		success = move_file(file, dir);
		if(!success){
			break;
		}
	}

	return success;
}

bool Util::File::rename_file(const QString& old_name, const QString& new_name)
{
	QFileInfo info(old_name);
	if(!info.isFile()){
		return false;
	}

	QFile f(old_name);
	return f.rename(new_name);
}

bool Util::File::copy_files(const QStringList& files, const QString& dir)
{
	bool success = true;
	for(const QString& file : files){
		success = copy_file(file, dir);
		if(!success){
			break;
		}
	}

	return success;
}

bool Util::File::move_file(const QString& file, const QString& dir)
{
	bool success = copy_file(file, dir);
	if(success){
		QFile f(file);
		return f.remove();
	}

	return success;
}

bool Util::File::copy_file(const QString& file, const QString& dir)
{
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


	QString pure_filename = get_filename_of_path(file);
	QString target_filename = d.absoluteFilePath(pure_filename);

	bool success = f.copy(target_filename);
	return success;
}

bool Util::File::exists(const QString& filename)
{
	if(filename.isEmpty()){
		return false;
	}

	return QFile::exists(filename);
}

bool Util::File::is_in_sayonara_dir(const QString& path)
{
	QDir sayonara_dir(sayonara_path());
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
