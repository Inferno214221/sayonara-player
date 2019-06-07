/* SomaFMStationView.h */

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


#ifndef SOMAFMSTATIONVIEW_H
#define SOMAFMSTATIONVIEW_H

#include "Gui/Utils/SearchableWidget/SearchableView.h"

class SomaFMStationView :
	public SearchableTableView
{
public:
	SomaFMStationView(QWidget* parent=nullptr);
	~SomaFMStationView();

public:
	// SayonaraSelectionView interface
	int index_by_model_index(const QModelIndex& idx) const override;
	ModelIndexRange model_indexrange_by_index(int idx) const override;

	int viewport_height() const override;

protected:
	void keyPressEvent(QKeyEvent *e) override;
};

#endif // SOMAFMSTATIONVIEW_H