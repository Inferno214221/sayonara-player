/* SomaFMStationView.h */

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

#ifndef SOMAFMSTATIONVIEW_H
#define SOMAFMSTATIONVIEW_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/SearchableWidget/SearchableView.h"
#include "Gui/Utils/SearchableWidget/SelectionView.h"

class SomaFMStationView :
	public SearchableTableView
{
	PIMPL(SomaFMStationView)

	public:
		explicit SomaFMStationView(QWidget* parent = nullptr);
		~SomaFMStationView() override;

		[[nodiscard]] int mapModelIndexToIndex(const QModelIndex& idx) const override;
		[[nodiscard]] ModelIndexRange mapIndexToModelIndexes(int idx) const override;

	protected:
		[[nodiscard]] SearchModel* searchModel() const override;
		void keyPressEvent(QKeyEvent* e) override;
		void showEvent(QShowEvent* e) override;
};

#endif // SOMAFMSTATIONVIEW_H
