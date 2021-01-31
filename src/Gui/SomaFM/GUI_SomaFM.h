/* GUI_SomaFM.h */

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

/* GUI_SomaFM.h */

#ifndef GUI_SOMAFM_H
#define GUI_SOMAFM_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/Widget.h"

#include <QItemSelection>

class QComboBox;
class QFrame;

UI_FWD(GUI_SomaFM)
namespace SomaFM
{
	class Station;
	class Library;

	class GUI_SomaFM :
			public Gui::Widget
	{
		Q_OBJECT
		UI_CLASS(GUI_SomaFM)
		PIMPL(GUI_SomaFM)

		public:
			explicit GUI_SomaFM(SomaFM::Library* library, QWidget* parent);
			~GUI_SomaFM() override;

			QFrame* headerFrame() const;

		private slots:
			void stationsLoaded(const QList<SomaFM::Station>& stations);
			void stationChanged(const SomaFM::Station& station);

			void stationDoubleClicked(const QModelIndex& idx);
			void stationClicked(const QModelIndex& idx);
			void stationIndexChanged(const QModelIndex& idx);
			void playlistDoubleClicked(const QModelIndex& idx);
			void coverFound(const QPixmap& cover);

			void selectionChanged(const QModelIndexList& selected);

		private:
			SomaFM::Station getStation(int row) const;
	};
}

#endif // GUI_SOMAFM_H
