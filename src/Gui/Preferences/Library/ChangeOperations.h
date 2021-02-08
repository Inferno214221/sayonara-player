/* ChangeOperations.h */

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

#ifndef CHANGEOPERATIONS_H
#define CHANGEOPERATIONS_H

#include "Utils/Pimpl.h"

namespace Library
{
	class Manager;
}

class ChangeOperation
{
	PIMPL(ChangeOperation)
	public:
		ChangeOperation(Library::Manager* libraryManager);
		virtual ~ChangeOperation();
		virtual bool exec() = 0;

		Library::Manager* manager() const;
};

class MoveOperation :
	public ChangeOperation
{
	PIMPL(MoveOperation)

	public:
		MoveOperation(Library::Manager* libraryManager, int from, int to);
		~MoveOperation();

		bool exec() override;
};

class RenameOperation :
	public ChangeOperation
{
	PIMPL(RenameOperation)

	public:
		RenameOperation(Library::Manager* libraryManager, LibraryId id, const QString& newName);
		~RenameOperation();

		bool exec() override;
};

class RemoveOperation :
	public ChangeOperation
{
	PIMPL(RemoveOperation)

	public:
		RemoveOperation(Library::Manager* libraryManager, LibraryId id);
		~RemoveOperation();

		bool exec() override;
};

class AddOperation :
	public ChangeOperation
{
	PIMPL(AddOperation)

	public:
		AddOperation(Library::Manager* libraryManager, const QString& name, const QString& path);
		~AddOperation();

		bool exec() override;
};

class ChangePathOperation :
	public ChangeOperation
{
	PIMPL(ChangePathOperation)

	public:
		ChangePathOperation(Library::Manager* libraryManager, LibraryId id, const QString& newPath);
		~ChangePathOperation();

		bool exec() override;
};

#endif // CHANGEOPERATIONS_H
