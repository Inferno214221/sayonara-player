/* GUI_SomaFM.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

/* GUI_SomaFM.cpp */

#include "GUI_SomaFM.h"
#include "Gui/SomaFM/ui_GUI_SomaFM.h"
#include "SomaFMStationModel.h"
#include "SomaFMPlaylistModel.h"
#include "Components/Streaming/SomaFM/SomaFMLibrary.h"
#include "Components/Streaming/SomaFM/SomaFMStation.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/Style.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/Widgets/ProgressBar.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookup.h"

#include <QPixmap>
#include <QItemDelegate>

using SomaFM::GUI_SomaFM;

struct GUI_SomaFM::Private
{
	SomaFM::Library*	library=nullptr;
	Gui::ProgressBar*	progress_bar=nullptr;

	Private(GUI_SomaFM* parent)
	{
		library	= new SomaFM::Library(parent);
	}
};


GUI_SomaFM::GUI_SomaFM(QWidget *parent) :
	Widget(parent)
{
	ui = new Ui::GUI_SomaFM();
	ui->setupUi(this);

	m = Pimpl::make<Private>(this);
	m->progress_bar = new Gui::ProgressBar(ui->tv_stations);
	m->progress_bar->set_position(Gui::ProgressBar::Position::Bottom);

	SomaFM::StationModel* model_stations = new SomaFM::StationModel(this);

	this->setFocusProxy(ui->tv_stations);
	ui->tv_stations->set_model(model_stations);
	ui->tv_stations->setItemDelegate(new QItemDelegate(ui->tv_stations));
	ui->tv_stations->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tv_stations->setEnabled(false);
	ui->tv_stations->setColumnWidth(0, 20);

	ui->lv_playlists->setModel(new SomaFM::PlaylistModel());
	ui->lv_playlists->setItemDelegate(new Gui::StyledItemDelegate(ui->lv_playlists));
	ui->lv_playlists->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QPixmap logo = QPixmap(":/soma_icons/soma_logo.png")
		.scaled(QSize(200, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	ui->lab_image->setPixmap(logo);

	bool dark = Style::is_dark();
	QString description =
		"Listener-supported, commercial-free, underground/alternative radio<br /><br />" +
		Util::create_link("https://somafm.com", dark);

	ui->lab_description->setText(description);
	ui->lab_donate->setText(Util::create_link("https://somafm.com/support/", dark));

	connect(m->library, &SomaFM::Library::sig_stations_loaded, this, &GUI_SomaFM::stations_loaded);
	connect(m->library, &SomaFM::Library::sig_station_changed, this, &GUI_SomaFM::station_changed);
	connect(m->library, &SomaFM::Library::sig_loading_started, m->progress_bar, &QWidget::show);
	connect(m->library, &SomaFM::Library::sig_loading_finished, m->progress_bar, &QWidget::hide);

	connect(ui->tv_stations, &QListView::activated, this, &GUI_SomaFM::station_index_changed);
	connect(ui->tv_stations, &QListView::clicked, this, &GUI_SomaFM::station_clicked);
	connect(ui->tv_stations, &QListView::doubleClicked, this, &GUI_SomaFM::station_double_clicked);

	connect(ui->lv_playlists, &QListView::doubleClicked, this, &GUI_SomaFM::playlist_double_clicked);
	connect(ui->lv_playlists, &QListView::activated, this, &GUI_SomaFM::playlist_double_clicked);

	m->library->search_stations();
}

GUI_SomaFM::~GUI_SomaFM()
{
	if(m->library) {
		m->library->deleteLater(); m->library = nullptr;
	}

	if(ui){
		delete ui; ui = nullptr;
	}
}

QFrame* GUI_SomaFM::header_frame() const
{
	return ui->header_frame;
}

void GUI_SomaFM::stations_loaded(const QList<SomaFM::Station>& stations)
{
	if(!ui){
		return;
	}

	sp_log(Log::Debug, this) << "Stations loaded";
	auto* model = static_cast<SomaFM::StationModel*>(ui->tv_stations->model());
	model->set_stations(stations);

	ui->tv_stations->setEnabled(true);
	ui->tv_stations->setDragEnabled(true);
	ui->tv_stations->setDragDropMode(QAbstractItemView::DragDrop);
	ui->tv_stations->resizeColumnToContents(0);
}

void GUI_SomaFM::station_changed(const SomaFM::Station& station)
{
	auto* model = static_cast<SomaFM::StationModel*>(ui->tv_stations->model());
	model->replace_station(station);
}

void GUI_SomaFM::station_double_clicked(const QModelIndex& idx)
{
	m->library->create_playlist_from_station(idx.row());
}

void GUI_SomaFM::selection_changed(const QModelIndexList& indexes)
{
	if(indexes.isEmpty()){
		return;
	}

	station_index_changed(indexes.first());
}


SomaFM::Station GUI_SomaFM::get_station(int row) const
{
	auto* station_model = static_cast<SomaFM::StationModel*>(ui->tv_stations->model());

	QModelIndex idx = station_model->index(row, 1);
	QString station_name = station_model->data(idx).toString();

	return m->library->station(station_name);
}

void GUI_SomaFM::station_clicked(const QModelIndex &idx)
{
	if(!idx.isValid()){
		return;
	}

	auto* station_model = static_cast<SomaFM::StationModel*>(ui->tv_stations->model());
	if(!station_model->has_stations() && idx.column() == 0)
	{
		station_model->set_waiting();
		m->library->search_stations();

		return;
	}

	SomaFM::Station station = get_station(idx.row());

	if(idx.column() == 0){
		m->library->set_station_loved( station.name(), !station.is_loved());
	}

	station_index_changed(idx);
}

void GUI_SomaFM::station_index_changed(const QModelIndex& idx)
{
	if(!idx.isValid()){
		return;
	}

	SomaFM::Station station = get_station(idx.row());

	auto pl_model = static_cast<SomaFM::PlaylistModel*>(ui->lv_playlists->model());
	pl_model->set_station(station);

	ui->lab_description->setText(station.description());

	Cover::Lookup* cl = new Cover::Lookup(station.cover_location(), 1, this);

	connect(cl, &Cover::LookupBase::sig_cover_found, this, &GUI_SomaFM::cover_found);

	cl->start();
}


void GUI_SomaFM::playlist_double_clicked(const QModelIndex& idx)
{
	m->library->create_playlist_from_playlist(idx.row());
}


void GUI_SomaFM::cover_found(const QPixmap& cover)
{
	auto* cl = static_cast<Cover::Lookup*>(sender());

	QPixmap pixmap = cover.scaled(QSize(200, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	if(pixmap.isNull()){
		pixmap = QPixmap(":/soma_icons/soma_logo.png").scaled(QSize(200, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	ui->lab_image->setPixmap(pixmap);

	if(cl){
		cl->deleteLater();
	}
}

