/* LibraryListModel.cpp */

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

#include "LibraryListModel.h"
#include "ChangeOperations.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/Algorithm.h"
#include "Utils/Library/LibraryInfo.h"

#include <QList>

using Library::Info;

namespace Algorithm=Util::Algorithm;

struct LibraryListModel::Private
{
	QList<Info> libraryInfo;
	QList<Info> shownLibraryInfo;
	QList<ChangeOperation*> operations;

	Private()
	{
		reload();
	}

	void reload()
	{
		libraryInfo = Library::Manager::instance()->allLibraries();
		shownLibraryInfo = libraryInfo;
	}

	void clear_operations()
	{
		for(ChangeOperation* op : Algorithm::AsConst(operations))
		{
			delete op;
		}

		operations.clear();
	}
};

LibraryListModel::LibraryListModel(QObject* parent) :
	QAbstractListModel(parent)
{
	m = Pimpl::make<Private>();
}

LibraryListModel::~LibraryListModel() = default;

int LibraryListModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return m->shownLibraryInfo.size();
}

QVariant LibraryListModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();

	if(row < 0 || row >= rowCount()){
		return QVariant();
	}

	if(role == Qt::DisplayRole)	{
		return m->shownLibraryInfo[row].name();
	}

	else if(role == Qt::ToolTipRole) {
		return m->shownLibraryInfo[row].path();
	}

	return QVariant();
}

void LibraryListModel::appendRow(const LibName& name, const LibPath& path)
{
	m->operations << new AddOperation(name, path);
	m->shownLibraryInfo << Info(name, path, -1);

	emit dataChanged(index(0), index(rowCount()));

}

void LibraryListModel::renameRow(int row, const LibName& new_name)
{
	if(!Util::between(row, m->shownLibraryInfo)) {
		return;
	}

	Info info = m->shownLibraryInfo[row];

	m->operations << new RenameOperation(info.id(), new_name);
	m->shownLibraryInfo[row] =
			Info(new_name, info.path(), info.id());
}

void LibraryListModel::changePath(int row, const LibPath& path)
{
	if(!Util::between(row, m->shownLibraryInfo)) {
		return;
	}

	Info info = m->shownLibraryInfo[row];

	m->operations << new ChangePathOperation(info.id(), path);
	m->shownLibraryInfo[row] =
			Info(info.name(), path, info.id());
}

void LibraryListModel::moveRow(int from, int to)
{
	if(!Util::between(from, m->shownLibraryInfo)) {
		return;
	}

	if(!Util::between(to, m->shownLibraryInfo)) {
		return;
	}

	m->operations << new MoveOperation(from, to);
	m->shownLibraryInfo.move(from, to);

	emit dataChanged(index(0), index(rowCount()));
}

void LibraryListModel::removeRow(int row)
{
	if(!Util::between(row, m->shownLibraryInfo)) {
		return;
	}

	Info info = m->shownLibraryInfo[row];

	m->operations << new RemoveOperation(info.id());
	m->shownLibraryInfo.removeAt(row);

	emit dataChanged(index(0), index(rowCount()));
}

QStringList LibraryListModel::allNames() const
{
	QStringList ret;

	for(const Info& info : Algorithm::AsConst(m->shownLibraryInfo))
	{
		ret << info.name();
	}

	return ret;
}

QStringList LibraryListModel::allPaths() const
{
	QStringList ret;

	for(const Info& info : Algorithm::AsConst(m->shownLibraryInfo))
	{
		ret << info.path();
	}

	return ret;
}

QString LibraryListModel::name(int idx) const
{
	if(Util::between(idx, m->shownLibraryInfo))
	{
		return m->shownLibraryInfo.at(idx).name();
	}

	return QString();
}

QString LibraryListModel::path(int idx) const
{
	if(Util::between(idx, m->shownLibraryInfo))
	{
		return m->shownLibraryInfo.at(idx).path();
	}

	return QString();
}

void LibraryListModel::reset()
{
	m->reload();
	emit dataChanged(index(0), index(rowCount()));
}

bool LibraryListModel::commit()
{
	if(m->operations.isEmpty()){
		return true;
	}

	bool success = true;
	for(ChangeOperation* op : Algorithm::AsConst(m->operations))
	{
		if(!op->exec()){
			success = false;
		}
	}

	m->reload();
	m->operations.clear();

	emit dataChanged(index(0), index(rowCount()));

	return success;
}

