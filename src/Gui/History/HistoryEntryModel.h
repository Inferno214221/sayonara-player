/* HistoryEntryModel.h
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

#ifndef HISTORYENTRYMODEL_H
#define HISTORYENTRYMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

namespace Session
{
	class Manager;
}

class MetaDataList;

class HistoryEntryModel :
	public QAbstractTableModel
{
	Q_OBJECT
	PIMPL(HistoryEntryModel)

	signals:
		void sigRowsAdded();

	public:
		HistoryEntryModel(Session::Manager* sessionManager, Session::Timecode timecode, QObject* parent = nullptr);
		~HistoryEntryModel() override;

		[[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		[[nodiscard]] int rowCount(const QModelIndex& parent) const override;
		[[nodiscard]] int columnCount(const QModelIndex& parent) const override;
		[[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
		[[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
		[[nodiscard]] QMimeData* mimeData(const QModelIndexList& indexes) const override;
		[[nodiscard]] MetaDataList tracksByIndexes(const QModelIndexList& indexes) const;

	private:
		[[nodiscard]] const Session::Entry& entry(int row) const;

	private slots: // NOLINT(readability-redundant-access-specifiers)
		void historyChanged(Session::Id id);

	protected:
		void languageChanged();
};

#endif // HISTORYENTRYMODEL_H
