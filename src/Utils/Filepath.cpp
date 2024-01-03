/* Filepath.cpp
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

#include "Filepath.h"
#include "StandardPaths.h"
#include "Utils.h"
#include "FileUtils.h"
#include <QString>

using Util::Filepath;

struct Filepath::Private
{
	QString path;

	Private(const QString& path) :
		path(path)
	{}
};

Filepath::Filepath(const QString& path)
{
	m = Pimpl::make<Private>(path);
}

Filepath::Filepath(const Filepath& other) :
	Filepath(other.path())
{}

Filepath::~Filepath() = default;

Filepath& Filepath::operator=(const QString& path)
{
	m->path = Util::File::cleanFilename(path);
	return (*this);
}

Filepath& Filepath::operator=(const Filepath& path)
{
	m->path = path.path();
	return (*this);
}

bool Filepath::operator==(const QString& path) const
{
	return (m->path == Util::File::cleanFilename(path));
}

bool Filepath::operator==(const Filepath& path) const
{
	return (m->path == path.path());
}

QString Filepath::path() const
{
	return m->path;
}

QString Filepath::fileystemPath() const
{
	if( isResource() )
	{
		QString dir, filename;
		Util::File::splitFilename(m->path, dir, filename);

		const auto localPath = Util::tempPath(filename);

		QString newName;
		Util::File::copyFile(m->path, Util::tempPath(), newName);

		return localPath;
	}

	return path();
}

bool Filepath::isResource() const
{
	return (m->path.startsWith(":"));
}

bool Filepath::isFilesystemPath() const
{
	if(isUrl() || isResource()){
		return false;
	}

	return true;
}

bool Filepath::isUrl() const
{
	return Util::File::isWWW(m->path);
}
