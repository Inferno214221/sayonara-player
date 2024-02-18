/* GUI_SomaFM.cpp */

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

#include <QItemDelegate>
#include <QListView>
#include <QPixmap>
#include <QPushButton>

using SomaFM::GUI_SomaFM;

struct GUI_SomaFM::Private
{
	SomaFM::Library* library;
	Gui::ProgressBar* progressBar = nullptr;
	QList<QPushButton*> stationButtons;

	explicit Private(SomaFM::Library* library) :
		library(library) {}
};

GUI_SomaFM::GUI_SomaFM(SomaFM::Library* library, QWidget* parent) :
	Widget(parent)
{
	ui = new Ui::GUI_SomaFM();
	ui->setupUi(this);

	m = Pimpl::make<Private>(library);
	m->progressBar = new Gui::ProgressBar(ui->tvStations);
	m->progressBar->setPosition(Gui::ProgressBar::Position::Bottom);

	this->setFocusProxy(ui->tvStations);
	ui->tvStations->setItemDelegate(new Gui::StyledItemDelegate(0, ui->tvStations));
	ui->tvStations->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tvStations->setEnabled(false);
	ui->tvStations->setColumnWidth(0, 20);

	const auto logo = QPixmap(":/soma_icons/soma_logo.png")
		.scaled(QSize(200, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	ui->labImage->setPixmap(logo);

	const auto isDark = Style::isDark();
	const auto description =
		"Listener-supported, commercial-free, underground/alternative radio<br /><br />" +
		Util::createLink("https://somafm.com", isDark);

	ui->labDescription->setText(description);
	ui->labDonate->setText(Util::createLink("https://somafm.com/support/", isDark));

	connect(m->library, &SomaFM::Library::sigStationsLoaded, this, &GUI_SomaFM::stationsLoaded);
	connect(m->library, &SomaFM::Library::sigStationChanged, this, &GUI_SomaFM::stationChanged);
	connect(m->library, &SomaFM::Library::sigLoadingStarted, m->progressBar, &QWidget::show);
	connect(m->library, &SomaFM::Library::sigLoadingFinished, m->progressBar, &QWidget::hide);

	connect(ui->tvStations, &QListView::activated, this, &GUI_SomaFM::stationIndexChanged);
	connect(ui->tvStations, &QListView::clicked, this, &GUI_SomaFM::stationClicked);
	connect(ui->tvStations, &QListView::doubleClicked, this, &GUI_SomaFM::stationDoubleClicked);

	m->library->searchStations();
}

GUI_SomaFM::~GUI_SomaFM()
{
	if(m->library)
	{
		m->library->deleteLater();
		m->library = nullptr;
	}

	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

QFrame* GUI_SomaFM::headerFrame() const
{
	return ui->headerFrame;
}

void GUI_SomaFM::stationsLoaded(const QList<SomaFM::Station>& stations)
{
	if(!ui)
	{
		return;
	}

	spLog(Log::Debug, this) << "Stations loaded";
	auto* model = dynamic_cast<SomaFM::StationModel*>(ui->tvStations->model());
	model->setStations(stations);

	ui->tvStations->setEnabled(true);
	ui->tvStations->setDragEnabled(true);
	ui->tvStations->setDragDropMode(QAbstractItemView::DragDrop);
	ui->tvStations->resizeColumnToContents(0);
	ui->tvStations->resizeRowsToContents();
}

void GUI_SomaFM::stationChanged(const SomaFM::Station& station)
{
	auto* model = dynamic_cast<SomaFM::StationModel*>(ui->tvStations->model());
	model->replaceStation(station);
}

void GUI_SomaFM::stationDoubleClicked(const QModelIndex& idx)
{
	m->library->createPlaylistFromStation(idx.row());
}

void GUI_SomaFM::selectionChanged(const QModelIndexList& indexes)
{
	if(!indexes.isEmpty())
	{
		stationIndexChanged(indexes.first());
	}
}

SomaFM::Station GUI_SomaFM::getStation(int row) const
{
	auto* stationModel = dynamic_cast<SomaFM::StationModel*>(ui->tvStations->model());

	const auto index = stationModel->index(row, 1);
	const auto stationName = stationModel->data(index).toString();

	return m->library->station(stationName);
}

void GUI_SomaFM::stationClicked(const QModelIndex& idx)
{
	if(!idx.isValid())
	{
		return;
	}

	auto* stationModel = dynamic_cast<SomaFM::StationModel*>(ui->tvStations->model());
	if(!stationModel->hasStations() && idx.column() == 0)
	{
		stationModel->setWaiting();
		m->library->searchStations();

		return;
	}

	const auto station = getStation(idx.row());

	if(idx.column() == 0)
	{
		m->library->setStationLoved(station.name(), !station.isLoved());
	}

	stationIndexChanged(idx);
}

void GUI_SomaFM::stationIndexChanged(const QModelIndex& idx)
{
	if(!idx.isValid())
	{
		return;
	}

	const auto pixmap = QPixmap(":/soma_icons/soma_logo.png");
	ui->labImage->setPixmap(pixmap.scaled(QSize(200, 200), Qt::KeepAspectRatio, Qt::SmoothTransformation));

	const auto station = getStation(idx.row());
	ui->labDescription->setText(station.description());

	for(auto* stationButton: m->stationButtons)
	{
		ui->verticalLayout->removeWidget(stationButton);
		stationButton->deleteLater();
	}

	m->stationButtons.clear();

	const auto playlists = station.playlists();

	auto index = 0;
	for(const auto& playlist: playlists)
	{
		const auto text = station.urlTypeString(playlist);
		const auto icon = Gui::Icons::icon(Gui::Icons::Play);

		auto* button = new QPushButton(icon, text, this);

		connect(button, &QPushButton::clicked, this, [this, index]() {
			m->library->createPlaylistFromStreamlist(index);
		});

		ui->verticalLayout->addWidget(button);
		m->stationButtons << button;

		index++;
	}

	auto* coverLookup = new Cover::Lookup(station.coverLocation(), 1, this);
	connect(coverLookup, &Cover::LookupBase::sigCoverFound, this, &GUI_SomaFM::coverFound);

	coverLookup->start();
}

void GUI_SomaFM::playlistDoubleClicked(const QModelIndex& idx)
{
	m->library->createPlaylistFromStreamlist(idx.row());
}

void GUI_SomaFM::coverFound(const QPixmap& cover)
{
	auto* coverLookup = dynamic_cast<Cover::Lookup*>(sender());

	auto pixmap = cover.scaled(QSize(200, 200),
	                           Qt::KeepAspectRatio,
	                           Qt::SmoothTransformation);
	if(pixmap.isNull())
	{
		pixmap = QPixmap(":/soma_icons/soma_logo.png").scaled(QSize(200, 200),
		                                                      Qt::KeepAspectRatio,
		                                                      Qt::SmoothTransformation);
	}

	ui->labImage->setPixmap(pixmap);

	if(coverLookup)
	{
		coverLookup->deleteLater();
	}
}

