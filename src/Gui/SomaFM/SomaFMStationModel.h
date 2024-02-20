/* SomaFMStationModel.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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


/* SomaFMStationModel.h */

#ifndef SomaFMStationModel_H
#define SomaFMStationModel_H

#include "Gui/Utils/SearchableWidget/SearchableModel.h"
#include "Utils/Pimpl.h"

#include <QMimeData>
#include <QAbstractTableModel>

namespace SomaFM
{
	class Station;
	class StationModel :
		public SearchableTableModel
	{
		Q_OBJECT
		PIMPL(StationModel)

		public:
			explicit StationModel(QObject* parent = nullptr);
			~StationModel() override;

		private:
			enum class Status :
				char
			{
				Waiting,
				Error,
				OK
			};

		public:
			[[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
			[[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
			[[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
			[[nodiscard]] QMimeData* mimeData(const QModelIndexList& indexes) const override;
			[[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;

			void setStations(const QList<SomaFM::Station>& stations);
			void replaceStation(const SomaFM::Station& station);
			[[nodiscard]] bool hasStations() const;
			void setWaiting();

		protected:
			[[nodiscard]] int itemCount() const override;
			[[nodiscard]] QString searchableString(int index, const QString& prefix) const override;
	};
}

#endif // SomaFMStationModel_H
