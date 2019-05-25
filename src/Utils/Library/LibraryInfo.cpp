/* LibraryInfo.cpp */

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

#include "LibraryInfo.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"

#include <QStringList>
#include <QFile>
#include <exception>

using Library::Info;

struct Info::Private
{
	QString path;
	QString name;
	LibraryId id;
};

Info::Info()
{
	m = Pimpl::make<Private>();
	m->id = -1;
}

Info::Info(const QString& name, const QString& path, LibraryId id) :
	Info()
{
	m->name = name;
	m->path = Util::File::clean_filename(path);
	m->id = id;
}

Info::Info(const Info& other) :
	Info()
{
	m->name = other.name();
	m->path = other.path();
	m->id = other.id();
}

Info::~Info() {}

Info& Info::operator =(const Info& other)
{
	m->name = other.name();
	m->path = other.path();
	m->id = other.id();

	return *this;
}


QString Info::name() const
{
	return m->name;
}

QString Info::path() const
{
	return m->path;
}

QString Info::symlink_path() const
{
	QString dir = Util::sayonara_path("Libraries");

	QString lib_path = this->path();
	if(lib_path.endsWith("/") || lib_path.endsWith("\\")){
		lib_path.remove(lib_path.size() - 1, 1);
	}

	QString pure_lib_name = Util::File::get_filename_of_path(lib_path);
	QString lib_name = this->name() + " - " + pure_lib_name;

	lib_name.remove("/");
	lib_name.remove("\\");
	lib_name.remove("#");
	lib_name.remove("#");
	lib_name.remove("*");
	lib_name.remove("{");
	lib_name.remove("}");
	lib_name.remove("?");
	lib_name.remove("\"");
	lib_name.remove("'");

	QString target = dir + "/" + lib_name;

	return target;
}

LibraryId Info::id() const
{
	return m->id;
}

bool Info::valid() const
{
	return (!m->name.isEmpty()) && (!m->path.isEmpty());
}

Info Info::fromString(const QString& str)
{
	QStringList lst = str.split("::");
	if(lst.size() != 3){
		throw  std::invalid_argument("Cannot convert Library::Info. Ignore");
	}

	bool ok;
	QString name = lst[0];
	QString path = lst[1];
	int id = lst[2].toInt(&ok);
	if(!ok){
		return Info();
	}

	return Info(name, path, id);
}

QString Info::toString() const
{
	QStringList lst;
	lst << m->name;
	lst << m->path;
	lst << QString::number(m->id);

	return lst.join("::");
}

bool Info::operator==(const Info& other) const
{
	return (other.toString() == this->toString());
}
