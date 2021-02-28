/* GUI_InfoDialog.cpp

 * Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Michael Lugmair (Lucio Carreras),
 * Jul 19, 2012
 *
 */

#include "GUI_InfoDialog.h"
#include "Gui/InfoDialog/ui_GUI_InfoDialog.h"

#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Tagging/GUI_TagEdit.h"
#include "Gui/InfoDialog/GUI_Lyrics.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/GuiUtils.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/MetaDataInfo/MetaDataInfo.h"
#include "Components/MetaDataInfo/AlbumInfo.h"
#include "Components/MetaDataInfo/ArtistInfo.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"

#include <QTimer>
#include <QTabBar>
#include <QTableWidgetItem>

namespace Algorithm = Util::Algorithm;

struct GUI_InfoDialog::Private
{
	GUI_TagEdit* uiTagEditor = nullptr;
	GUI_Lyrics* uiLyrics = nullptr;

	Cover::Location coverLocation;
	MetaDataList tracks;
	MD::Interpretation metadataInterpretation{MD::Interpretation::None};

	MetaDataList localTracks() const
	{
		MetaDataList result;

		std::copy_if(tracks.begin(), tracks.end(), std::back_inserter(result), [](const auto& track) {
			return (!Util::File::isWWW(track.filepath()));
		});

		return result;
	}
};

GUI_InfoDialog::GUI_InfoDialog(QWidget* parent) :
	Dialog(parent)
{
	m = Pimpl::make<Private>();
}

GUI_InfoDialog::~GUI_InfoDialog() = default;

void GUI_InfoDialog::languageChanged()
{
	if(ui)
	{
		ui->retranslateUi(this);

		prepareInfo(m->metadataInterpretation);

		ui->tabWidget->setTabText(0, Lang::get(Lang::Info));
		ui->tabWidget->setTabText(1, Lang::get(Lang::Lyrics));
		ui->tabWidget->setTabText(2, Lang::get(Lang::Edit));
		ui->btnWriteCoverToTracks->setText(tr("Write cover to tracks") + "...");
	}
}

void GUI_InfoDialog::skinChanged()
{
	if(ui)
	{
		using namespace Gui;

		auto* tabBar = ui->tabWidget->tabBar();
		tabBar->setTabIcon(0, Icons::icon(Icons::Info));
		tabBar->setTabIcon(1, Icons::icon(Icons::Lyrics));
		tabBar->setTabIcon(2, Icons::icon(Icons::Edit));
	}
}

static void prepareInfoTable(QTableWidget* table, const QList<StringPair>& data)
{
	table->clear();
	table->setRowCount(data.count());
	table->setColumnCount(2);
	table->setAlternatingRowColors(true);
	table->setShowGrid(false);
	table->setEditTriggers(QTableView::NoEditTriggers);
	table->setSelectionBehavior(QTableView::SelectRows);
	table->setItemDelegate(new Gui::StyledItemDelegate(table));

	QFont font(table->font());
	font.setBold(true);

	const QFontMetrics fm(font);

	int maxSize = 0;
	int row = 0;
	for(const auto& stringPair : data)
	{
		auto* item1 = new QTableWidgetItem(stringPair.first);

		item1->setFont(font);

		const auto textWidth = Gui::Util::textWidth(fm, stringPair.first) + 20;
		maxSize = std::max(textWidth, maxSize);

		auto* item2 = new QTableWidgetItem(stringPair.second);

		table->setItem(row, 0, item1);
		table->setItem(row, 1, item2);

		row++;
	}

	table->horizontalHeader()->resizeSection(0, maxSize);
	table->resizeRowsToContents();
}

static void preparePaths(QListWidget* pathListWidget, const QStringList& paths)
{
	pathListWidget->clear();
	pathListWidget->setAlternatingRowColors(true);
	pathListWidget->setEditTriggers(QListView::NoEditTriggers);
	pathListWidget->setTextElideMode(Qt::TextElideMode::ElideLeft);

	for(const auto& path : paths)
	{
		auto* label = new QLabel(pathListWidget);
		label->setText(path);
		label->setOpenExternalLinks(true);
		label->setTextFormat(Qt::RichText);

		auto* item = new QListWidgetItem(pathListWidget);
		pathListWidget->setItemWidget(item, label);
		pathListWidget->addItem(item);
	}
}

void GUI_InfoDialog::prepareInfo(MD::Interpretation mdInterpretation)
{
	if(!ui)
	{
		return;
	}

	MetaDataInfo* info;
	switch(mdInterpretation)
	{
		case MD::Interpretation::Artists:
			info = new ArtistInfo(m->tracks);
			break;
		case MD::Interpretation::Albums:
			info = new AlbumInfo(m->tracks);
			break;
		case MD::Interpretation::None:
		case MD::Interpretation::Tracks:
		default:
			info = new MetaDataInfo(m->tracks);
			break;
	}

	ui->btnWriteCoverToTracks->setVisible(info->albums().size() == 1);
	ui->labTitle->setText(info->header());
	ui->labSubheader->setText(info->subheader());

	const auto infoMap = info->infostringMap();
	prepareInfoTable(ui->tableInfo, infoMap);

	const auto paths = info->paths();
	preparePaths(ui->listPaths, paths);

	m->coverLocation = info->coverLocation();
	prepareCover(m->coverLocation);
	ui->btnImage->setEnabled(m->coverLocation.isValid());

	delete info;
}

void GUI_InfoDialog::setMetadata(const MetaDataList& tracks, MD::Interpretation interpretation)
{
	m->metadataInterpretation = interpretation;
	m->tracks = tracks;

	this->setBusy(m->tracks.isEmpty());
}

bool GUI_InfoDialog::hasMetadata() const
{
	return (!m->tracks.isEmpty());
}

GUI_InfoDialog::Tab GUI_InfoDialog::show(GUI_InfoDialog::Tab tab)
{
	const auto size = GetSetting(Set::InfoDialog_Size);

	if(!ui)
	{
		init();
	}

	const auto lyricEnabled = (m->tracks.size() == 1);
	const auto tagEditEnabled = Algorithm::contains(m->tracks, [](const auto& track) {
		return (!Util::File::isWWW(track.filepath()));
	});

	ui->tabWidget->setTabEnabled(int(Tab::Edit), tagEditEnabled);
	ui->tabWidget->setTabEnabled(int(Tab::Lyrics), lyricEnabled);

	if(!ui->tabWidget->isTabEnabled(+tab) || m->tracks.isEmpty())
	{
		tab = Tab::Info;
	}

	this->setBusy(m->tracks.isEmpty());
	this->prepareTab(tab);

	if(ui->tabWidget->currentIndex() != +tab)
	{
		ui->tabWidget->setCurrentIndex(+tab);
	}

	Dialog::show();
	if(size.isValid())
	{
		QTimer::singleShot(100, this, [&, size]() {
			this->resize(size);
		});
	}

	return Tab(ui->tabWidget->currentIndex());
}

void GUI_InfoDialog::prepareCover(const Cover::Location& coverLocation)
{
	ui->btnImage->setCoverLocation(coverLocation);
}

void GUI_InfoDialog::init()
{
	if(ui)
	{
		return;
	}

	ui = new Ui::InfoDialog();
	ui->setupUi(this);

	ui->tabWidget->setFocusPolicy(Qt::NoFocus);
	ui->tableInfo->setItemDelegate(new Gui::StyledItemDelegate(ui->tableInfo));
	ui->listPaths->setItemDelegate(new Gui::StyledItemDelegate(ui->listPaths));

	connect(ui->tabWidget, &QTabWidget::currentChanged, this, &GUI_InfoDialog::tabIndexChangedInt);
	connect(ui->btnWriteCoverToTracks, &QPushButton::clicked, this, &GUI_InfoDialog::writeCoversToTracksClicked);
	connect(ui->btnImage, &Gui::CoverButton::sigRejected, this, &GUI_InfoDialog::writeCoversToTracksClicked);
	connect(ui->btnImage, &Gui::CoverButton::sigCoverChanged, this, &GUI_InfoDialog::coverChanged);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_InfoDialog::close);

	ui->stackedWidget->setCurrentIndex(0);
	ui->btnImage->setStyleSheet("QPushButton:hover {background-color: transparent;}");

	this->setModal(false);
}

void GUI_InfoDialog::initTagEdit()
{
	if(!m->uiTagEditor)
	{
		auto* tab3Layout = ui->tabEditor->layout();
		m->uiTagEditor = new GUI_TagEdit(ui->tabEditor);
		tab3Layout->addWidget(m->uiTagEditor);

		connect(m->uiTagEditor, &GUI_TagEdit::sigCancelled, this, &GUI_InfoDialog::close);
	}
}

void GUI_InfoDialog::initLyrics()
{
	if(!m->uiLyrics)
	{
		m->uiLyrics = new GUI_Lyrics(ui->tabLyrics);

		auto* tab2Layout = ui->tabLyrics->layout();
		tab2Layout->addWidget(m->uiLyrics);

		connect(m->uiLyrics, &GUI_Lyrics::sigClosed, this, &GUI_InfoDialog::close);
	}
}

void GUI_InfoDialog::tabIndexChangedInt(int index)
{
	index = std::min(int(GUI_InfoDialog::Tab::Edit), index);
	index = std::max(int(GUI_InfoDialog::Tab::Info), index);

	prepareTab(GUI_InfoDialog::Tab(index));
}

void GUI_InfoDialog::prepareTab(GUI_InfoDialog::Tab index)
{
	if(!ui)
	{
		return;
	}

	switch(index)
	{
		case GUI_InfoDialog::Tab::Edit:
			showTagEditTab();
			break;

		case GUI_InfoDialog::Tab::Lyrics:
			showLyricsTab();
			break;

		default:
			showInfoTab();
			break;
	}
}

void GUI_InfoDialog::writeCoversToTracksClicked()
{
	showCoverEditTab();
}

void GUI_InfoDialog::coverChanged()
{
	const auto width = ui->btnImage->width();
	ui->btnImage->resize(width, width);
}

void GUI_InfoDialog::showInfoTab()
{
	prepareInfo(m->metadataInterpretation);
	prepareCover(m->coverLocation);

	ui->tabWidget->setCurrentWidget(ui->uiInfoWidget);
	ui->uiInfoWidget->show();
}

void GUI_InfoDialog::showLyricsTab()
{
	if(m->tracks.isEmpty())
	{
		ui->tabWidget->setCurrentIndex(0);
		return;
	}

	initLyrics();
	ui->tabWidget->setCurrentWidget(m->uiLyrics);

	m->uiLyrics->setTrack(m->tracks.first());
	m->uiLyrics->show();
}

void GUI_InfoDialog::showTagEditTab()
{
	const auto localTracks = m->localTracks();
	if(localTracks.isEmpty())
	{
		ui->tabWidget->setCurrentIndex(0);
		return;
	}

	initTagEdit();

	m->uiTagEditor->setMetadata(localTracks);
	ui->tabWidget->setCurrentWidget(m->uiTagEditor);

	m->uiTagEditor->setMetadata(localTracks);
	m->uiTagEditor->showDefaultTab();
	m->uiTagEditor->show();
}

void GUI_InfoDialog::showCoverEditTab()
{
	const auto tab = show(GUI_InfoDialog::Tab::Edit);
	if(tab == GUI_InfoDialog::Tab::Edit)
	{
		m->uiTagEditor->showCoverTab();
	}
}

void GUI_InfoDialog::setBusy(bool b)
{
	if(ui)
	{
		ui->stackedWidget->setCurrentIndex(b ? 1 : 0);
	}
}

void GUI_InfoDialog::closeEvent(QCloseEvent* e)
{
	Dialog::closeEvent(e);
	m->tracks.clear();
}

void GUI_InfoDialog::showEvent(QShowEvent* e)
{
	init();
	Dialog::showEvent(e);
}

void GUI_InfoDialog::resizeEvent(QResizeEvent* e)
{
	if(this->isVisible())
	{
		SetSetting(Set::InfoDialog_Size, e->size());
	}

	Dialog::resizeEvent(e);
}
