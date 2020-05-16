/* GUI_StationSearcher.cpp */

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

#include "GUI_StationSearcher.h"
#include "Gui/Plugins/ui_GUI_StationSearcher.h"
#include "Components/Streaming/StationSearcher/StationSearcher.h"

#include "Utils/Utils.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/EventFilter.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"

#include <QMenu>

struct GUI_StationSearcher::Private
{
	QList<RadioStation>		stations;
	StationSearcher*		searcher;
	StationSearcher::Mode	mode;
	QMenu*					contextMenu=nullptr;

	Private(GUI_StationSearcher* parent) :
		mode(StationSearcher::NewSearch)
	{
		searcher = new StationSearcher(parent);
	}

	void setFromToLabel(QLabel* label)
	{
		label->setVisible(stations.size() > 5);

		if(stations.size() < 5){
			return;
		}

		label->setText
		(
			tr("Show radio stations from %1 to %2")
				.arg("<b>" + stations.first().name + "</b>")
				.arg("<b>" + stations.last().name + "</b>" )
		);
	}

	void setPlaceholderText(QLineEdit* le)
	{
		if(mode == StationSearcher::Style)
		{
			le->setPlaceholderText(Lang::get(Lang::SearchVerb) + ": " + Lang::get(Lang::Genre));
		}

		else {
			le->setPlaceholderText(Lang::get(Lang::SearchVerb) + ": " + Lang::get(Lang::RadioStation));
		}
	}

};

GUI_StationSearcher::GUI_StationSearcher(QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>(this);

	ui = new Ui::GUI_StationSearcher();
	ui->setupUi(this);

	okButton()->setEnabled(false);
	ui->pbProgress->setVisible(false);
	ui->btnSearch->setEnabled(ui->leSearch->text().size() > 0);
	ui->btnSearchNext->setVisible(m->searcher->canSearchNext());
	ui->btnSearchPrev->setVisible(m->searcher->canSearchPrevious());
	ui->twStations->setEnabled(false);
	ui->twStreams->setEnabled(false);

	ui->splitter->setStretchFactor(0, 3);
	ui->splitter->setStretchFactor(1, 1);

	ui->twStations->setItemDelegate(new Gui::StyledItemDelegate(ui->twStations));
	ui->twStreams->setItemDelegate(new Gui::StyledItemDelegate(ui->twStreams));

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GUI_StationSearcher::okClicked);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_StationSearcher::close);
	connect(ui->leSearch, &QLineEdit::textChanged, this, &GUI_StationSearcher::searchTextChanged);
	connect(ui->leSearch, &QLineEdit::returnPressed, this, &GUI_StationSearcher::searchClicked);
	connect(ui->btnSearch, &QPushButton::clicked, this, &GUI_StationSearcher::searchClicked);
	connect(ui->btnSearchNext, &QPushButton::clicked, this, &GUI_StationSearcher::searchNextClicked);
	connect(ui->btnSearchPrev, &QPushButton::clicked, this, &GUI_StationSearcher::searchPreviousClicked);
	connect(ui->twStations, &QTableWidget::itemSelectionChanged, this, &GUI_StationSearcher::stationsChanged);
	connect(ui->twStreams, &QTableWidget::itemSelectionChanged, this, &GUI_StationSearcher::streamChanged);
	connect(m->searcher, &StationSearcher::sigStationsFound, this, &GUI_StationSearcher::stationsFetched);
}

GUI_StationSearcher::~GUI_StationSearcher() = default;

QAbstractButton* GUI_StationSearcher::okButton()
{
	return ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok);
}

void GUI_StationSearcher::initLineEdit()
{
	auto* cmf = new Gui::ContextMenuFilter(ui->leSearch);
	auto* menu = new QMenu(ui->leSearch);
	m->contextMenu = menu;
	connect(cmf, &Gui::ContextMenuFilter::sigContextMenu, this, [menu](const QPoint& p, QAction* action)
	{
		Q_UNUSED(action)
		menu->exec(p);
	});

	QAction* action1 = m->contextMenu->addAction(Lang::get(Lang::RadioStation));
	QAction* action2 = m->contextMenu->addAction(Lang::get(Lang::Genre));

	connect(action1, &QAction::triggered, this, [this](){
		this->changeMode(StationSearcher::Mode::NewSearch);
	});

	connect(action2, &QAction::triggered, this, [this](){
		this->changeMode(StationSearcher::Mode::Style);
	});

	ui->leSearch->installEventFilter(cmf);
}

void GUI_StationSearcher::checkListenButton()
{
	okButton()->setEnabled(false);

	int currentStationIndex = ui->twStations->currentRow();
	if(currentStationIndex < 0 || currentStationIndex >= m->stations.size()){
		return;
	}

	RadioStation station = m->stations[currentStationIndex];
	int currentStreamIndex = ui->twStreams->currentRow();
	if(currentStreamIndex < 0 || currentStreamIndex >= station.streams.size()){
		return;
	}

	okButton()->setEnabled(true);
}

void GUI_StationSearcher::clearStations()
{
	ui->twStations->clear();
	while(ui->twStations->rowCount() > 0){
		ui->twStations->removeRow(0);
	}

	ui->twStations->setEnabled(false);
}

void GUI_StationSearcher::clearStreams()
{
	ui->twStreams->clear();
	while(ui->twStreams->rowCount() > 0){
		ui->twStreams->removeRow(0);
	}

	ui->twStreams->setEnabled(false);
}

void GUI_StationSearcher::changeMode(StationSearcher::Mode mode)
{
	m->mode = mode;
	m->setPlaceholderText(ui->leSearch);
}

void GUI_StationSearcher::searchClicked()
{
	const QString text = ui->leSearch->text();
	if(text.isEmpty()){
		return;
	}

	m->stations.clear();

	clearStations();
	clearStreams();

	if(m->mode == StationSearcher::Style)
	{
		m->searcher->searchStyle(text);
	}

	else
	{
		m->searcher->searchStation(text);
	}


	ui->pbProgress->setVisible(true);
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

static
QTableWidgetItem* createTableWidgetItem(const QString& text, const QFontMetrics& fm)
{
	auto* item = new QTableWidgetItem(text);
	int width = std::min(Gui::Util::textWidth(fm, text + "bla"), 250);
	item->setSizeHint(QSize(width, 0));
	item->setToolTip(text);

	return item;
};

void GUI_StationSearcher::stationsFetched()
{
	const QList<RadioStation> stations = m->searcher->foundStations();

	ui->pbProgress->setVisible(false);
	ui->twStations->setEnabled(true);

	ui->btnSearchNext->setVisible(m->searcher->canSearchNext());
	ui->btnSearchPrev->setVisible(m->searcher->canSearchPrevious());

	if(stations.isEmpty())
	{
		if( m->searcher->mode() == StationSearcher::NewSearch ||
			m->searcher->mode() == StationSearcher::Style)
		{			
			ui->labFromTo->setVisible(false);

			clearStations();
			clearStreams();

			m->stations.clear();
		}

		return;
	}

	clearStations();

	m->stations = stations;
	m->setFromToLabel(ui->labFromTo);

	ui->twStations->setRowCount(m->stations.size());
	ui->twStations->setColumnCount(4);
	ui->twStations->setHorizontalHeaderItem(0, new QTableWidgetItem(Lang::get(Lang::Name)));
	ui->twStations->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Country")));
	ui->twStations->setHorizontalHeaderItem(2, new QTableWidgetItem(Lang::get(Lang::Info)));
	ui->twStations->setHorizontalHeaderItem(3, new QTableWidgetItem("Url"));

	ui->twStations->horizontalHeader()->setResizeContentsPrecision(20);

	const QFontMetrics fm = this->fontMetrics();
	int row=0;
	for(const RadioStation& station : m->stations)
	{
		auto* itemName = createTableWidgetItem(station.name, fm);
		auto* itemLocation = createTableWidgetItem(station.location, fm);
		auto* itemDesc = createTableWidgetItem(station.description, fm );
		auto* itemUrl = createTableWidgetItem(station.home_url, fm);

		ui->twStations->setItem(row, 0, itemName);
		ui->twStations->setItem(row, 1, itemLocation);
		ui->twStations->setItem(row, 2, itemDesc);
		ui->twStations->setItem(row, 3, itemUrl);

		itemName->setToolTip(station.description);

		row++;
	}

	ui->twStations->setEnabled(true);
	ui->twStations->resizeColumnsToContents();
}

void GUI_StationSearcher::okClicked()
{
	int currentStationIndex = ui->twStations->currentRow();
	const RadioStation station = m->stations.at(currentStationIndex);

	int currentStreamIndex = ui->twStreams->currentRow();
	const RadioUrl radioUrl = station.streams.at(currentStreamIndex);

	emit sigStreamSelected(station.name, radioUrl.url, ui->cbSave->isChecked());

	this->close();
}

void GUI_StationSearcher::searchTextChanged(const QString& text)
{
	ui->btnSearch->setEnabled(text.size() > 0);
	ui->btnSearchNext->setVisible(false);
	ui->btnSearchPrev->setVisible(false);

	if(text.startsWith("s:") || text.startsWith("n:"))
	{
		changeMode(StationSearcher::NewSearch);
		ui->leSearch->clear();
	}

	else if(text.startsWith("g:"))
	{
		changeMode(StationSearcher::Style);
		ui->leSearch->clear();
	}
}

void GUI_StationSearcher::stationsChanged()
{
	okButton()->setEnabled(false);

	int currentRow = ui->twStations->currentRow();
	if(currentRow < 0 || currentRow >= m->stations.count()){
		return;
	}

	RadioStation station = m->stations[currentRow];

	clearStreams();
	checkListenButton();

	ui->twStreams->setEnabled(true);
	ui->twStreams->setRowCount(station.streams.size());
	ui->twStreams->setColumnCount(3);
	ui->twStreams->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Type")));
	ui->twStreams->setHorizontalHeaderItem(1, new QTableWidgetItem(Lang::get(Lang::Bitrate)));
	ui->twStreams->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Url")));

	const QFontMetrics fm = this->fontMetrics();

	int row = 0;
	for(const RadioUrl& radioUrl : station.streams)
	{
		auto* itemType = createTableWidgetItem(radioUrl.type, fm);
		auto* itemBitrate = createTableWidgetItem(radioUrl.bitrate, fm);
		auto* itemUrl = createTableWidgetItem(radioUrl.url, fm);

		ui->twStreams->setItem(row, 0, itemType);
		ui->twStreams->setItem(row, 1, itemBitrate);
		ui->twStreams->setItem(row, 2, itemUrl);

		row++;
	}

	ui->twStreams->resizeColumnsToContents();
	ui->twStreams->setCurrentItem(ui->twStreams->item(0, 0));
	streamChanged();
}

void GUI_StationSearcher::streamChanged()
{
	checkListenButton();
}

void GUI_StationSearcher::showEvent(QShowEvent* e)
{
	ui->leSearch->setFocus();
	Gui::Dialog::showEvent(e);

	QSize sz = GetSetting(Set::Stream_SearchWindowSize);
	if(!sz.isEmpty())
	{
		this->resize(sz);
	}

	ui->leSearch->setFocus();
	ui->leSearch->setFocus();
}

void GUI_StationSearcher::closeEvent(QCloseEvent* e)
{
	SetSetting(Set::Stream_SearchWindowSize, this->size());

	Gui::Dialog::closeEvent(e);
}

void GUI_StationSearcher::languageChanged()
{
	ui->btnSearch->setText(Lang::get(Lang::SearchVerb));
	ui->btnSearchNext->setText(Lang::get(Lang::NextPage));
	ui->btnSearchPrev->setText(Lang::get(Lang::PreviousPage));

	const QString tooltip = QString("<b>%1</b><br />s:, n: %2<br />g: %3")
		.arg(Lang::get(Lang::SearchNoun))
		.arg(Lang::get(Lang::RadioStation))
		.arg(Lang::get(Lang::Genre));

	ui->leSearch->setToolTip(tooltip);
	ui->cbSave->setText(Lang::get(Lang::Save));

	m->setPlaceholderText(ui->leSearch);
	m->setFromToLabel(ui->labFromTo);
	ui->label->setText(Lang::get(Lang::SearchNoun) + ": " + Lang::get(Lang::RadioStation));
}

void GUI_StationSearcher::skinChanged()
{
	const QFontMetrics fm = this->fontMetrics();

	ui->twStations->horizontalHeader()->setMinimumHeight(std::max(fm.height() + 10, 20));
	ui->labLink->setText(Util::createLink("fmstream.org", Style::isDark(), true, "http://fmstream.org"));
}
