/* GUI_SomaFM.cpp */

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

/* GUI_SomaFM.cpp */

#include "GUI_SomaFM.h"
#include "Gui/SomaFM/ui_GUI_SomaFM.h"
#include "SomaFMStationModel.h"
#include "Components/Streaming/SomaFM/SomaFMLibrary.h"
#include "Components/Streaming/SomaFM/SomaFMStation.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/Style.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/Widgets/ProgressBar.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookup.h"

#include <QPixmap>
#include <QItemDelegate>
#include <QPushButton>

using SomaFM::GUI_SomaFM;

struct GUI_SomaFM::Private
{
	SomaFM::Library*	library=nullptr;
	Gui::ProgressBar*	progressBar=nullptr;

	QList<QPushButton*> stationButtons;

	Private(GUI_SomaFM* parent)
	{
		library	= new SomaFM::Library(parent);
	}
};


GUI_SomaFM::GUI_SomaFM(QWidget* parent) :
	Widget(parent)
{
	ui = new Ui::GUI_SomaFM();
	ui->setupUi(this);

	m = Pimpl::make<Private>(this);
	m->progressBar = new Gui::ProgressBar(ui->tv_stations);
	m->progressBar->setPosition(Gui::ProgressBar::Position::Bottom);

	SomaFM::StationModel* model_stations = new SomaFM::StationModel(this);

	this->setFocusProxy(ui->tv_stations);
	ui->tv_stations->setSearchableModel(model_stations);
	ui->tv_stations->setItemDelegate(new QItemDelegate(ui->tv_stations));
	ui->tv_stations->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tv_stations->setEnabled(false);
	ui->tv_stations->setColumnWidth(0, 20);

	QPixmap logo = QPixmap(":/soma_icons/soma_logo.png")
		.scaled(QSize(200, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	ui->lab_image->setPixmap(logo);

	bool dark = Style::isDark();
	QString description =
		"Listener-supported, commercial-free, underground/alternative radio<br /><br />" +
		Util::createLink("https://somafm.com", dark);

	ui->lab_description->setText(description);
	ui->lab_donate->setText(Util::createLink("https://somafm.com/support/", dark));

	connect(m->library, &SomaFM::Library::sigStationsLoaded, this, &GUI_SomaFM::stationsLoaded);
	connect(m->library, &SomaFM::Library::sigStationChanged, this, &GUI_SomaFM::stationChanged);
	connect(m->library, &SomaFM::Library::sigLoadingStarted, m->progressBar, &QWidget::show);
	connect(m->library, &SomaFM::Library::sigLoadingFinished, m->progressBar, &QWidget::hide);

	connect(ui->tv_stations, &QListView::activated, this, &GUI_SomaFM::stationIndexChanged);
	connect(ui->tv_stations, &QListView::clicked, this, &GUI_SomaFM::stationClicked);
	connect(ui->tv_stations, &QListView::doubleClicked, this, &GUI_SomaFM::stationDoubleClicked);

	m->library->searchStations();
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

QFrame* GUI_SomaFM::headerFrame() const
{
	return ui->header_frame;
}

void GUI_SomaFM::stationsLoaded(const QList<SomaFM::Station>& stations)
{
	if(!ui){
		return;
	}

	spLog(Log::Debug, this) << "Stations loaded";
	auto* model = static_cast<SomaFM::StationModel*>(ui->tv_stations->model());
	model->setStations(stations);

	ui->tv_stations->setEnabled(true);
	ui->tv_stations->setDragEnabled(true);
	ui->tv_stations->setDragDropMode(QAbstractItemView::DragDrop);
	ui->tv_stations->resizeColumnToContents(0);
}

void GUI_SomaFM::stationChanged(const SomaFM::Station& station)
{
	auto* model = static_cast<SomaFM::StationModel*>(ui->tv_stations->model());
	model->replaceStation(station);
}

void GUI_SomaFM::stationDoubleClicked(const QModelIndex& idx)
{
	m->library->createPlaylistFromStation(idx.row());
}

void GUI_SomaFM::selectionChanged(const QModelIndexList& indexes)
{
	if(indexes.isEmpty()){
		return;
	}

	stationIndexChanged(indexes.first());
}

SomaFM::Station GUI_SomaFM::getStation(int row) const
{
	auto* station_model = static_cast<SomaFM::StationModel*>(ui->tv_stations->model());

	QModelIndex idx = station_model->index(row, 1);
	QString station_name = station_model->data(idx).toString();

	return m->library->station(station_name);
}

void GUI_SomaFM::stationClicked(const QModelIndex& idx)
{
	if(!idx.isValid()){
		return;
	}

	auto* station_model = static_cast<SomaFM::StationModel*>(ui->tv_stations->model());
	if(!station_model->hasStations() && idx.column() == 0)
	{
		station_model->setWaiting();
		m->library->searchStations();

		return;
	}

	SomaFM::Station station = getStation(idx.row());

	if(idx.column() == 0){
		m->library->setStationLoved( station.name(), !station.isLoved());
	}

	stationIndexChanged(idx);
}


void GUI_SomaFM::stationIndexChanged(const QModelIndex& idx)
{
	if(!idx.isValid()){
		return;
	}

	const QPixmap pixmap(":/soma_icons/soma_logo.png");
	ui->lab_image->setPixmap(pixmap.scaled(QSize(200, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation));

	const SomaFM::Station station = getStation(idx.row());
	ui->lab_description->setText(station.description());

	for(QPushButton* btn : m->stationButtons)
	{
		ui->verticalLayout->removeWidget(btn);
		btn->deleteLater();
	}

	m->stationButtons.clear();

	const QStringList playlists = station.playlists();

	int index=0;
	for(const QString& playlist : playlists)
	{
		const QString text = station.urlTypeString(playlist);
		const QIcon icon = Gui::Icons::icon(Gui::Icons::Play);

		auto* button = new QPushButton(icon, text, this);

		connect(button, &QPushButton::clicked, this, [this, index](){
			m->library->createPlaylistFromStreamlist(index);
		});

		ui->verticalLayout->addWidget(button);
		m->stationButtons << button;

		index++;
	}

	auto* cl = new Cover::Lookup(station.coverLocation(), 1, this);
	connect(cl, &Cover::LookupBase::sigCoverFound, this, &GUI_SomaFM::coverFound);

	cl->start();
}

void GUI_SomaFM::playlistDoubleClicked(const QModelIndex& idx)
{
	m->library->createPlaylistFromStreamlist(idx.row());
}

void GUI_SomaFM::coverFound(const QPixmap& cover)
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

