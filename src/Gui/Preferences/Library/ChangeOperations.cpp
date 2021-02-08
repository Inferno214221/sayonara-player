/* ChangeOperations.cpp */

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

#include "ChangeOperations.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include <QString>

struct ChangeOperation::Private
{
	Library::Manager* libraryManager;

	Private(Library::Manager* libraryManager) :
		libraryManager {libraryManager} {}
};

ChangeOperation::ChangeOperation(Library::Manager* libraryManager)
{
	m = Pimpl::make<Private>(libraryManager);
}

ChangeOperation::~ChangeOperation() = default;

Library::Manager* ChangeOperation::manager() const
{
	return m->libraryManager;
}

struct MoveOperation::Private
{
	int from, to;

	Private(int from, int to) :
		from(from),
		to(to) {}
};

MoveOperation::MoveOperation(Library::Manager* libraryManager, int from, int to) :
	ChangeOperation(libraryManager)
{
	m = Pimpl::make<Private>(from, to);
}

MoveOperation::~MoveOperation() = default;

bool MoveOperation::exec()
{
	return manager()->moveLibrary(m->from, m->to);
}

struct RenameOperation::Private
{
	LibraryId id;
	QString newName;

	Private(LibraryId id, const QString& newName) :
		id(id),
		newName(newName) {}
};

RenameOperation::RenameOperation(Library::Manager* libraryManager, LibraryId id, const QString& newName) :
	ChangeOperation(libraryManager)
{
	m = Pimpl::make<Private>(id, newName);
}

RenameOperation::~RenameOperation() = default;

bool RenameOperation::exec()
{
	return manager()->renameLibrary(m->id, m->newName);
}

struct RemoveOperation::Private
{
	LibraryId id;

	Private(LibraryId id) :
		id(id) {}
};

RemoveOperation::RemoveOperation(Library::Manager* libraryManager, LibraryId id) :
	ChangeOperation(libraryManager)
{
	m = Pimpl::make<Private>(id);
}

RemoveOperation::~RemoveOperation() = default;

bool RemoveOperation::exec()
{
	return manager()->removeLibrary(m->id);
}

struct AddOperation::Private
{
	QString name, path;

	Private(const QString& name, const QString& path) :
		name(name),
		path(path) {}
};

AddOperation::AddOperation(Library::Manager* libraryManager, const QString& name, const QString& path) :
	ChangeOperation(libraryManager)
{
	m = Pimpl::make<Private>(name, path);
}

AddOperation::~AddOperation() = default;

bool AddOperation::exec()
{
	return (manager()->addLibrary(m->name, m->path) >= 0);
}

struct ChangePathOperation::Private
{
	LibraryId id;
	QString newPath;

	Private(LibraryId id, const QString& newPath) :
		id(id),
		newPath(newPath) {}
};

ChangePathOperation::ChangePathOperation(Library::Manager* libraryManager, LibraryId id, const QString& newPath) :
	ChangeOperation(libraryManager)
{
	m = Pimpl::make<Private>(id, newPath);
}

ChangePathOperation::~ChangePathOperation() = default;

bool ChangePathOperation::exec()
{
	return manager()->changeLibraryPath(m->id, m->newPath);
}
