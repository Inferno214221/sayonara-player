/* SomaFMStationView.cpp */

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

#include "SomaFMStationView.h"
#include "SomaFMStationModel.h"

#include <QHeaderView>

struct SomaFMStationView::Private
{
	SomaFM::StationModel* model;

	explicit Private(SomaFM::StationModel* model) :
		model {model} {}
};

SomaFMStationView::SomaFMStationView(QWidget* parent) :
	SearchableTableView(parent),
	m {Pimpl::make<Private>(new SomaFM::StationModel(this))}
{
	setModel(m->model);
}

SomaFMStationView::~SomaFMStationView() = default;

int SomaFMStationView::mapModelIndexToIndex(const QModelIndex& idx) const { return idx.row(); }

ModelIndexRange SomaFMStationView::mapIndexToModelIndexes(int idx) const
{
	const auto index = model()->index(idx, 0);
	return {index, index};
}

void SomaFMStationView::keyPressEvent(QKeyEvent* e)
{
	e->setAccepted(false);

	QTableView::keyPressEvent(e);
}

void SomaFMStationView::showEvent(QShowEvent* e)
{
	Gui::WidgetTemplate<QTableView>::showEvent(e);

	const auto height = fontMetrics().height();
	horizontalHeader()->setMinimumSectionSize(height * 2);
	resizeColumnToContents(0);
}

SearchModel* SomaFMStationView::searchModel() const { return m->model; }
