/* GUI_StationSearcher.cpp */

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

#include "GUI_StationSearcher.h"
#include "Gui/Plugins/ui_GUI_StationSearcher.h"
#include "Gui/Utils/Icons.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Streaming/StationSearcher/FMStreamSearcher.h"
#include "Components/Streaming/StationSearcher/RadioBrowserSearcher.h"
#include "Components/Streaming/StationSearcher/RadioStation.h"

#include "Utils/Algorithm.h"
#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/EventFilter.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/Widgets/HeaderView.h"

#include <QComboBox>
#include <QMenu>

#include <optional>

namespace
{
	void setFromToLabel(QLabel* label, const QList<RadioStation>& stations)
	{
		constexpr const auto MinimumStations = 5;
		label->setVisible(stations.size() > MinimumStations); // NOLINT(readability-magic-numbers)

		if(stations.size() >= MinimumStations)
		{
			const auto text = QObject::tr("Show radio stations from %1 to %2")
				.arg("<b>" + stations.first().name + "</b>")
				.arg("<b>" + stations.last().name + "</b>");

			label->setText(text);
		}
	}

	QString getUrlFromStation(const RadioStation& radioStation)
	{
		if(Util::File::isWWW(radioStation.home_url))
		{
			return radioStation.home_url;
		}

		const auto index = Util::Algorithm::indexOf(radioStation.streams, [](const auto& stream) {
			return Util::File::isWWW(stream.url);
		});

		return (index < 0)
		       ? QString()
		       : radioStation.streams[index].url;
	}

	MetaData convertStationToTrack(const RadioStation& radioStation)
	{
		const auto url = getUrlFromStation(radioStation);

		MetaData track;
		track.setRadioStation(url, radioStation.name);
		track.setFilepath(url);
		track.setCoverDownloadUrls({radioStation.image});
		return track;
	}

	QTableWidgetItem* createTableWidgetItem(const QString& text, const QFontMetrics& fm)
	{
		auto* item = new QTableWidgetItem(text);

		const auto width = std::min(Gui::Util::textWidth(fm, text + "bla"), 250);

		item->setSizeHint({width, Gui::Util::viewRowHeight(fm)});
		item->setToolTip(text);

		return item;
	}

	void initTableWidget(QTableWidget* tableWidget)
	{
		tableWidget->setItemDelegate(new Gui::StyledItemDelegate(tableWidget));
		tableWidget->setEditTriggers(static_cast<QTableView::EditTriggers>(QTableView::NoEditTriggers));
		tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
		tableWidget->setHorizontalHeader(new Gui::HeaderView(Qt::Orientation::Horizontal, tableWidget));
		tableWidget->horizontalHeader()->setStretchLastSection(true);

		tableWidget->setEnabled(false);
	}

	enum class ServiceProvider :
		uint8_t
	{
		RadioBrowserInfo = 0U,
		FMStreamOrg
	};

	void initServiceComboBox(QComboBox* comboBox)
	{
		comboBox->addItem("radio-browser.info", static_cast<int>(ServiceProvider::RadioBrowserInfo));
		comboBox->addItem("fmstream.org", static_cast<int>(ServiceProvider::FMStreamOrg));

		const auto index = comboBox->findText(GetSetting(Set::Stream_RadioSearcher));
		comboBox->setCurrentIndex(std::max(index, 0));
	}

	StationSearcher::Mode getCurrentMode(QComboBox* comboBox)
	{
		return static_cast<StationSearcher::Mode>(comboBox->currentData().toInt());
	};

	void initTypeComboBox(QComboBox* comboBox)
	{
		comboBox->clear();
		comboBox->addItem(Lang::get(Lang::Name), static_cast<int>(StationSearcher::Mode::ByName));
		comboBox->addItem(Lang::get(Lang::Genre), static_cast<int>(StationSearcher::Mode::ByStyle));

		const auto index = comboBox->findData(GetSetting(Set::Stream_RadioSearcherType));
		comboBox->setCurrentIndex(std::max(index, 0));
	}

	void clearTableWidget(QTableWidget* tableWidget)
	{
		tableWidget->clear();
		while(tableWidget->rowCount() > 0)
		{
			tableWidget->removeRow(0);
		}

		tableWidget->horizontalHeader()->hide();
		tableWidget->setEnabled(false);
	}

	void populateStationWidget(QTableWidget* tableWidget, const QList<RadioStation>& stations)
	{
		tableWidget->setRowCount(stations.size());
		tableWidget->setColumnCount(4);
		tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(Lang::get(Lang::Name)));
		tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(QObject::tr("Country")));
		tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(Lang::get(Lang::Info)));
		tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem("Url"));
		tableWidget->horizontalHeader()->setResizeContentsPrecision(20); // NOLINT(readability-magic-numbers)
		tableWidget->horizontalHeader()->show();
		tableWidget->setEnabled(!stations.isEmpty());

		const auto fm = tableWidget->fontMetrics();
		auto row = 0;
		for(const auto& station: stations)
		{
			auto* itemName = createTableWidgetItem(station.name, fm);
			auto* itemLocation = createTableWidgetItem(station.location, fm);
			auto* itemDesc = createTableWidgetItem(station.description, fm);
			auto* itemUrl = createTableWidgetItem(station.home_url, fm);

			tableWidget->setItem(row, 0, itemName);
			tableWidget->setItem(row, 1, itemLocation);
			tableWidget->setItem(row, 2, itemDesc);
			tableWidget->setItem(row, 3, itemUrl);

			itemName->setToolTip(station.description);

			row++;
		}

		for(auto i = 0; i < tableWidget->columnCount() - 1; i++)
		{
			tableWidget->resizeColumnToContents(i);
		}
	}

	void populateStreamWidget(QTableWidget* tableWidget, const RadioStation& station)
	{
		const auto fm = tableWidget->fontMetrics();

		tableWidget->setRowCount(station.streams.size());
		tableWidget->setColumnCount(3);
		tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(QObject::tr("Type")));
		tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(Lang::get(Lang::Bitrate)));
		tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(QObject::tr("Url")));
		tableWidget->setEnabled(!station.streams.isEmpty());

		auto row = 0;
		for(const auto& radioUrl: station.streams)
		{
			auto* itemType = createTableWidgetItem(radioUrl.type, fm);
			auto* itemBitrate = createTableWidgetItem(radioUrl.bitrate, fm);
			auto* itemUrl = createTableWidgetItem(radioUrl.url, fm);

			tableWidget->setItem(row, 0, itemType);
			tableWidget->setItem(row, 1, itemBitrate);
			tableWidget->setItem(row, 2, itemUrl);

			row++;
		}

		if(!station.streams.isEmpty())
		{
			tableWidget->setCurrentItem(tableWidget->item(0, 0));
		}

		for(auto i = 0; i < tableWidget->columnCount() - 1; i++)
		{
			tableWidget->resizeColumnToContents(i);
		}
	}

	std::optional<RadioStation> stationAt(const QList<RadioStation>& stations, int row)
	{
		return (Util::between(row, stations))
		       ? std::optional {stations[row]}
		       : std::nullopt;
	}
}

struct GUI_StationSearcher::Private
{
	StationSearcher* searcher {nullptr};
};

GUI_StationSearcher::GUI_StationSearcher(QWidget* parent) :
	Gui::Dialog(parent),
	m {Pimpl::make<Private>()},
	ui {std::make_shared<Ui::GUI_StationSearcher>()}
{
	ui->setupUi(this);

	ui->btnListen->setEnabled(false);
	ui->btnSaveAndListen->setEnabled(false);
	ui->pbProgress->setVisible(false);
	ui->btnCover->setVisible(false);
	ui->btnSearch->setEnabled(false);
	ui->btnSearchNext->setVisible(false);
	ui->btnSearchPrev->setVisible(false);
	ui->labLink->setOpenExternalLinks(true);
	ui->leSearch->setPlaceholderText(QObject::tr("Search radio station"));

	ui->splitter->setStretchFactor(0, 3);
	ui->splitter->setStretchFactor(1, 1);

	initTableWidget(ui->twStations);
	initTableWidget(ui->twStreams);
	initServiceComboBox(ui->comboService);
	currentServiceChanged(ui->comboService->currentText());
	initTypeComboBox(ui->comboType);
	changeMode(getCurrentMode(ui->comboType));

	connect(ui->btnListen, &QPushButton::clicked, this, &GUI_StationSearcher::listenClicked);
	connect(ui->btnSaveAndListen, &QPushButton::clicked, this, &GUI_StationSearcher::saveAndListenClicked);
	connect(ui->btnCancel, &QPushButton::clicked, this, &GUI_StationSearcher::close);
	connect(ui->leSearch, &QLineEdit::textChanged, this, &GUI_StationSearcher::searchTextChanged);
	connect(ui->leSearch, &QLineEdit::returnPressed, this, &GUI_StationSearcher::searchClicked);
	connect(ui->btnSearch, &QPushButton::clicked, this, &GUI_StationSearcher::searchClicked);
	connect(ui->btnSearchNext, &QPushButton::clicked, this, &GUI_StationSearcher::searchNextClicked);
	connect(ui->btnSearchPrev, &QPushButton::clicked, this, &GUI_StationSearcher::searchPreviousClicked);
	connect(ui->twStations, &QTableWidget::itemSelectionChanged, this, &GUI_StationSearcher::currentStationChanged);
	connect(ui->comboService, &QComboBox::currentTextChanged, this, &GUI_StationSearcher::currentServiceChanged);
	connect(ui->comboType, combo_current_index_changed_int, this, [this](const auto& /*index*/) {
		changeMode(static_cast<StationSearcher::Mode>(ui->comboType->currentData().toInt()));
	});
}

GUI_StationSearcher::~GUI_StationSearcher() = default;

void GUI_StationSearcher::checkListenButtons()
{
	const auto& currentStation = stationAt(m->searcher->foundStations(), ui->twStations->currentRow());
	const auto currentStreamIndex = ui->twStreams->currentRow();
	const auto isEnabled = (currentStation && Util::between(currentStreamIndex, currentStation->streams));

	ui->btnListen->setEnabled(isEnabled);
	ui->btnSaveAndListen->setEnabled(isEnabled);
}

void GUI_StationSearcher::clearStations()
{
	clearTableWidget(ui->twStations);
	clearStreams();
}

void GUI_StationSearcher::clearStreams()
{
	clearTableWidget(ui->twStreams);
	ui->btnCover->setVisible(false);
}

void GUI_StationSearcher::changeMode(const StationSearcher::Mode mode)
{
	SetSetting(Set::Stream_RadioSearcherType, static_cast<int>(mode));
}

void GUI_StationSearcher::searchClicked()
{
	clearStations();
	checkListenButtons();

	if(const auto text = ui->leSearch->text(); !text.isEmpty())
	{
		const auto mode = getCurrentMode(ui->comboType);
		if(mode == StationSearcher::ByStyle)
		{
			m->searcher->searchStyle(text);
		}

		else
		{
			m->searcher->searchStation(text);
		}

		ui->pbProgress->setVisible(true);
	}
}

void GUI_StationSearcher::searchPreviousClicked()
{
	m->searcher->searchPrevious();
	ui->pbProgress->setVisible(true);
	ui->twStations->setEnabled(false);
	ui->twStreams->setEnabled(false);
}

void GUI_StationSearcher::searchNextClicked()
{
	m->searcher->searchNext();
	ui->pbProgress->setVisible(true);
	ui->twStations->setEnabled(false);
	ui->twStreams->setEnabled(false);
}

void GUI_StationSearcher::stationsFetched()
{
	ui->pbProgress->setVisible(false);
	ui->twStations->setEnabled(true);
	ui->btnSearchNext->setVisible(m->searcher->canSearchNext());
	ui->btnSearchPrev->setVisible(m->searcher->canSearchPrevious());

	const auto& stations = m->searcher->foundStations();
	if(!stations.isEmpty())
	{
		setFromToLabel(ui->labFromTo, stations);
		clearStations();
		populateStationWidget(ui->twStations, stations);
	}
}

void GUI_StationSearcher::listenClicked() { listen(false); }

void GUI_StationSearcher::saveAndListenClicked() { listen(true); }

void GUI_StationSearcher::listen(const bool save)
{
	const auto currentStation = stationAt(m->searcher->foundStations(), ui->twStations->currentRow());
	const auto currentStreamIndex = ui->twStreams->currentRow();
	const auto& stream = currentStation->streams[currentStreamIndex];

	emit sigStreamSelected(currentStation->name, stream.url, save);

	close();
}

void GUI_StationSearcher::searchTextChanged(const QString& text)
{
	ui->btnSearch->setEnabled(!text.isEmpty());
	ui->btnSearchNext->setVisible(false);
	ui->btnSearchPrev->setVisible(false);
}

void GUI_StationSearcher::currentStationChanged()
{
	clearStreams();

	const auto currentStation = stationAt(m->searcher->foundStations(), ui->twStations->currentRow());
	if(currentStation)
	{
		populateStreamWidget(ui->twStreams, *currentStation);
		setupCoverButton(*currentStation);
	}

	checkListenButtons();
}

void GUI_StationSearcher::currentServiceChanged(const QString& service)
{
	ui->labLink->setText(Util::createLink(QString("https://%1").arg(service)));
	SetSetting(Set::Stream_RadioSearcher, service);

	const auto serviceProvider = static_cast<ServiceProvider>(ui->comboService->currentData().toInt());

	delete m->searcher;
	m->searcher = (serviceProvider == ServiceProvider::FMStreamOrg)
	              ? static_cast<StationSearcher*>(new FMStreamSearcher(nullptr))
	              : static_cast<StationSearcher*>(new RadioBrowserSearcher(nullptr));
	connect(m->searcher, &StationSearcher::sigStationsFound, this, &GUI_StationSearcher::stationsFetched);
}

void GUI_StationSearcher::setupCoverButton(const RadioStation& station)
{
	ui->btnCover->setVisible(false);

	const auto track = convertStationToTrack(station);
	if(track.radioMode() == RadioMode::Station)
	{
		const auto coverLocation = Cover::Location::coverLocation(track);
		if(coverLocation.isValid())
		{
			ui->btnCover->setCoverLocation(coverLocation);
			ui->btnCover->setVisible(true);
		}
	}
}

void GUI_StationSearcher::showEvent(QShowEvent* e)
{
	Gui::Dialog::showEvent(e);

	const auto windowSize = GetSetting(Set::Stream_SearchWindowSize);
	if(windowSize.isValid())
	{
		resize(windowSize);
	}

	ui->leSearch->setFocus();
	ui->labLink->setText(Util::createLink(QString("https://%1").arg(ui->comboService->currentText())));
}

void GUI_StationSearcher::closeEvent(QCloseEvent* e)
{
	SetSetting(Set::Stream_SearchWindowSize, size());

	Gui::Dialog::closeEvent(e);
}

void GUI_StationSearcher::languageChanged()
{
	ui->btnSearch->setText(Lang::get(Lang::SearchVerb));
	ui->btnSearchNext->setText(Lang::get(Lang::NextPage));
	ui->btnSearchPrev->setText(Lang::get(Lang::PreviousPage));
	ui->btnCancel->setText(Lang::get(Lang::Cancel));
	ui->btnListen->setText(Lang::get(Lang::Listen));
	ui->btnSaveAndListen->setText(
		QString("%1 && %2")
			.arg(Lang::get(Lang::Save))
			.arg(Lang::get(Lang::Listen)));

	const auto tooltip = QString("<b>%1</b><br>s:, n: %2<br>g: %3")
		.arg(Lang::get(Lang::SearchNoun))
		.arg(Lang::get(Lang::RadioStation))
		.arg(Lang::get(Lang::Genre));

	ui->leSearch->setToolTip(tooltip);

	ui->leSearch->setPlaceholderText(QObject::tr("Search radio station"));
	setFromToLabel(ui->labFromTo, m->searcher->foundStations());
	ui->label->setText(Lang::get(Lang::SearchNoun) + ": " + Lang::get(Lang::RadioStation));

	ui->comboType->blockSignals(true);
	initTypeComboBox(ui->comboType);
	ui->comboType->blockSignals(false);
}

void GUI_StationSearcher::skinChanged()
{
	const auto fontHeight = fontMetrics().height();

	constexpr const auto HeightOffset = 10;
	constexpr const auto RowHeight = 20;

	const auto rowHeight = std::max(fontHeight + HeightOffset, RowHeight);

	ui->twStations->horizontalHeader()->setMinimumHeight(rowHeight);
	ui->labLink->setText(Util::createLink("fmstream.org", Style::isDark(), true, "http://fmstream.org"));

	ui->btnCancel->setIcon(Gui::Icons::icon(Gui::Icons::Close));
	ui->btnListen->setIcon(Gui::Icons::icon(Gui::Icons::Play));
	ui->btnSaveAndListen->setIcon(Gui::Icons::icon(Gui::Icons::Save));
}
