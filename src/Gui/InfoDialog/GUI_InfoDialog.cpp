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

#include "InfoDialogContainer.h"

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
#include <QTableWidget>
#include <QTableWidgetItem>

namespace Algorithm=Util::Algorithm;

struct GUI_InfoDialog::Private
{
	InfoDialogContainer*	infoDialogContainer=nullptr;
	GUI_TagEdit*			uiTagEditor=nullptr;
	GUI_Lyrics*				uiLyrics=nullptr;
	Cover::Location			coverLocation;
	MetaDataList			tracks;
	MD::Interpretation		metadataInterpretation;

	Private(InfoDialogContainer* container) :
		infoDialogContainer(container),
		metadataInterpretation(MD::Interpretation::None)
	{}

	MetaDataList localTracks() const
	{
		MetaDataList ret;

		std::copy_if(tracks.begin(), tracks.end(), std::back_inserter(ret), [](const MetaData& md){
			return (!Util::File::isWWW(md.filepath()));
		});

		return ret;
	}
};

GUI_InfoDialog::GUI_InfoDialog(InfoDialogContainer* container, QWidget* parent) :
	Dialog(parent)
{
	m = Pimpl::make<Private>(container);
}

GUI_InfoDialog::~GUI_InfoDialog() = default;

void GUI_InfoDialog::languageChanged()
{
	if(!ui){
		return;
	}

	ui->retranslateUi(this);

	prepareInfo(m->metadataInterpretation);

	ui->tabWidget->setTabText(0, Lang::get(Lang::Info));
	ui->tabWidget->setTabText(1, Lang::get(Lang::Lyrics));
	ui->tabWidget->setTabText(2, Lang::get(Lang::Edit));
	ui->btnWriteCoverToTracks->setText
	(
		tr("Write cover to tracks") + "..."
	);
}

void GUI_InfoDialog::skinChanged()
{
	if(!ui){
		return;
	}

	using namespace Gui;

	QTabBar* tabBar = ui->tabWidget->tabBar();
	tabBar->setTabIcon(0, Icons::icon(Icons::Info));
	tabBar->setTabIcon(1, Icons::icon(Icons::Lyrics));
	tabBar->setTabIcon(2, Icons::icon(Icons::Edit));
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

	int maxSize=0;
	int row	= 0;
	for(const StringPair& p : data)
	{
		auto* item1 = new QTableWidgetItem(p.first);

		item1->setFont(font);
		int w1 = Gui::Util::textWidth(fm, p.first) + 20;
		if(w1 > maxSize){
			maxSize = w1;
		}

		auto* item2 = new QTableWidgetItem(p.second);

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

	for(const QString& path : paths)
	{
		QLabel* label = new QLabel(pathListWidget);
		label->setText(path);
		label->setOpenExternalLinks(true);
		label->setTextFormat(Qt::RichText);

		QListWidgetItem* item = new QListWidgetItem(pathListWidget);
		pathListWidget->setItemWidget(item, label);
		pathListWidget->addItem(item);
	}
}

void GUI_InfoDialog::prepareInfo(MD::Interpretation mdInterpretation)
{
	if(!ui){
		return;
	}

	MetaDataInfo* info=nullptr;

	switch (mdInterpretation)
	{
		case MD::Interpretation::Artists:
			info = new ArtistInfo(m->tracks);
			break;
		case MD::Interpretation::Albums:
			info = new AlbumInfo(m->tracks);
			break;

		case MD::Interpretation::Tracks:
			info = new MetaDataInfo(m->tracks);
			break;

		default:
			return;
	}

	ui->btnWriteCoverToTracks->setVisible(info->albums().size() == 1);
	ui->labTitle->setText(info->header());
	ui->labSubheader->setText(info->subheader());

	const QList<StringPair> infoMap = info->infostringMap();
	prepareInfoTable(ui->tableInfo, infoMap);

	QStringList paths = info->paths();
	preparePaths(ui->listPaths, paths);

	m->coverLocation = info->coverLocation();
	prepareCover(m->coverLocation);
	ui->btnImage->setEnabled(m->coverLocation.isValid());

	delete info; info = nullptr;
}

void GUI_InfoDialog::setMetadata(const MetaDataList& tracks, MD::Interpretation md_interpretation)
{
	m->metadataInterpretation = md_interpretation;
	m->tracks = tracks;

	this->setBusy(m->tracks.isEmpty());
}

bool GUI_InfoDialog::hasMetadata() const
{
	return (m->tracks.size() > 0);
}

GUI_InfoDialog::Tab GUI_InfoDialog::show(GUI_InfoDialog::Tab tab)
{
	QSize size = GetSetting(Set::InfoDialog_Size);
	int iTab = int(tab);
	if(!ui){
		init();
	}

	bool lyricEnabled = (m->tracks.size() == 1);
	bool tagEditEnabled = Algorithm::contains(m->tracks, [](const MetaData& md){
		return (!Util::File::isWWW(md.filepath()));
	});

	ui->tabWidget->setTabEnabled(int(Tab::Edit), tagEditEnabled);
	ui->tabWidget->setTabEnabled(int(Tab::Lyrics), lyricEnabled);

	if(!ui->tabWidget->isTabEnabled(iTab) || m->tracks.isEmpty())
	{
		iTab = 0;
		tab = Tab::Info;
	}

	this->setBusy(m->tracks.isEmpty());
	this->prepareTab(tab);

	if(ui->tabWidget->currentIndex() != iTab) {
		ui->tabWidget->setCurrentIndex(iTab);
	}

	Dialog::show();
	if(size.isValid())
	{
		QTimer::singleShot(100, this, [size, this](){
			this->resize(size);
		});
	}

	return Tab(ui->tabWidget->currentIndex());
}

void GUI_InfoDialog::prepareCover(const Cover::Location& cl)
{
	ui->btnImage->setCoverLocation(cl);
}

void GUI_InfoDialog::init()
{
	if(ui){
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
		QLayout* tab3Layout = ui->tabEditor->layout();
		m->uiTagEditor = new GUI_TagEdit(ui->tabEditor);
		tab3Layout->addWidget(m->uiTagEditor);

		connect(m->uiTagEditor, &GUI_TagEdit::sigCancelled, this, &GUI_InfoDialog::close);
	}
}

void GUI_InfoDialog::initLyrics()
{
	if(!m->uiLyrics)
	{
		QLayout* tab2Layout = ui->tabLyrics->layout();
		m->uiLyrics = new GUI_Lyrics(ui->tabLyrics);
		tab2Layout->addWidget(m->uiLyrics);

		connect(m->uiLyrics, &GUI_Lyrics::sigClosed, this, &GUI_InfoDialog::close);
	}
}

void GUI_InfoDialog::tabIndexChangedInt(int index)
{
	index = std::min(int(GUI_InfoDialog::Tab::Edit), index);
	index = std::max(int(GUI_InfoDialog::Tab::Info), index);

	prepareTab( GUI_InfoDialog::Tab(index) );
}

void GUI_InfoDialog::prepareTab(GUI_InfoDialog::Tab index)
{
	if(!ui){
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
	int w = ui->btnImage->width();
	ui->btnImage->resize(w, w);
}

void GUI_InfoDialog::showInfoTab()
{
	prepareInfo(m->metadataInterpretation);

	ui->tabWidget->setCurrentWidget(ui->uiInfoWidget);
	ui->uiInfoWidget->show();
	prepareCover(m->coverLocation);
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
	const MetaDataList localTracks = m->localTracks();
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
	auto tab = show(GUI_InfoDialog::Tab::Edit);
	if(tab == GUI_InfoDialog::Tab::Edit)
	{
		m->uiTagEditor->showCoverTab();
	}
}

void GUI_InfoDialog::setBusy(bool b)
{
	if(ui){
		ui->stackedWidget->setCurrentIndex(b ? 1 : 0);
	}
}

void GUI_InfoDialog::closeEvent(QCloseEvent* e)
{
	Dialog::closeEvent(e);

	m->tracks.clear();
	m->infoDialogContainer->infoDialogClosed();
}

void GUI_InfoDialog::showEvent(QShowEvent* e)
{
	if(!ui){
		init();
	}

	Dialog::showEvent(e);
}

void GUI_InfoDialog::resizeEvent(QResizeEvent* e)
{
	if(this->isVisible()) {
		SetSetting(Set::InfoDialog_Size, e->size());
	}

	Dialog::resizeEvent(e);
}
