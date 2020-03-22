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

#include "Gui/Tagging/GUI_TagEdit.h"
#include "Gui/InfoDialog/GUI_Lyrics.h"
#include "Gui/Utils/Icons.h"

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

	ui->tab_widget->setTabText(0, Lang::get(Lang::Info));
	ui->tab_widget->setTabText(1, Lang::get(Lang::Lyrics));
	ui->tab_widget->setTabText(2, Lang::get(Lang::Edit));
	ui->btn_close1->setText(Lang::get(Lang::Close));
	ui->btn_write_cover_to_tracks->setText
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

	QTabBar* tab_bar = ui->tab_widget->tabBar();
	tab_bar->setTabIcon(0, Icons::icon(Icons::Info));
	tab_bar->setTabIcon(1, Icons::icon(Icons::Lyrics));
	tab_bar->setTabIcon(2, Icons::icon(Icons::Edit));
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

	int row	= 0;
	for(const StringPair& p : data)
	{
		QTableWidgetItem* i1 = new QTableWidgetItem(p.first);
		QFont f(i1->font());
		f.setBold(true);
		i1->setFont(f);
		QTableWidgetItem* i2 = new QTableWidgetItem(p.second);

		table->setItem(row, 0, i1);
		table->setItem(row, 1, i2);

		row++;
	}

	table->resizeColumnToContents(0);
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

	ui->btn_write_cover_to_tracks->setVisible(info->albums().size() == 1);
	ui->lab_title->setText(info->header());
	ui->lab_subheader->setText(info->subheader());

	const QList<StringPair> infoMap = info->infostringMap();
	prepareInfoTable(ui->table_info, infoMap);

	QStringList paths = info->paths();
	preparePaths(ui->list_paths, paths);

	m->coverLocation = info->coverLocation();
	prepareCover(m->coverLocation);
	ui->btn_image->setEnabled(m->coverLocation.isValid());

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
	int iTab = int(tab);
	if(!ui){
		init();
	}

	this->setBusy(m->tracks.isEmpty());

	if(m->tracks.isEmpty()){
		Dialog::show();
		return Tab::Info;
	}

	QTabWidget* tabWidget = ui->tab_widget;

	bool lyric_enabled = (m->tracks.size() == 1);
	bool tag_edit_enabled = Algorithm::contains(m->tracks, [](const MetaData& md){
		return (!Util::File::isWWW(md.filepath()));
	});

	tabWidget->setTabEnabled(int(Tab::Edit), tag_edit_enabled);
	tabWidget->setTabEnabled(int(Tab::Lyrics), lyric_enabled);

	if( !tabWidget->isTabEnabled(iTab))
	{
		tab = Tab::Info;
	}

	if(tabWidget->currentIndex() == iTab)
	{
		tabIndexChanged(tab);
	}

	else
	{
		tabWidget->setCurrentIndex(iTab);
	}

	Dialog::show();

	return Tab(tabWidget->currentIndex());
}

void GUI_InfoDialog::prepareCover(const Cover::Location& cl)
{
	ui->btn_image->setCoverLocation(cl);
}

void GUI_InfoDialog::init()
{
	if(ui){
		return;
	}

	ui = new Ui::InfoDialog();
	ui->setupUi(this);

	QTabWidget* tabWidget = ui->tab_widget;
	tabWidget->setFocusPolicy(Qt::NoFocus);

	connect(tabWidget, &QTabWidget::currentChanged, this, &GUI_InfoDialog::tabIndexChangedInt);
	connect(ui->btn_write_cover_to_tracks, &QPushButton::clicked, this, &GUI_InfoDialog::writeCoversToTracksClicked);
	connect(ui->btn_image, &Gui::CoverButton::sigRejected, this, &GUI_InfoDialog::writeCoversToTracksClicked);
	connect(ui->btn_image, &Gui::CoverButton::sigCoverChanged, this, &GUI_InfoDialog::coverChanged);

	ui->stackedWidget->setCurrentIndex(0);
	ui->btn_image->setStyleSheet("QPushButton:hover {background-color: transparent;}");

	this->setModal(false);
}

void GUI_InfoDialog::initTagEdit()
{
	if(!m->uiTagEditor)
	{
		QLayout* tab3Layout = ui->tab_3->layout();
		m->uiTagEditor = new GUI_TagEdit(ui->tab_3);
		tab3Layout->addWidget(m->uiTagEditor);

		connect(m->uiTagEditor, &GUI_TagEdit::sigCancelled, this, &GUI_InfoDialog::close);
	}
}

void GUI_InfoDialog::initLyrics()
{
	if(!m->uiLyrics)
	{
		QLayout* tab2Layout = ui->tab_2->layout();
		m->uiLyrics = new GUI_Lyrics(ui->tab_2);
		tab2Layout->addWidget(m->uiLyrics);

		connect(m->uiLyrics, &GUI_Lyrics::sigClosed, this, &GUI_InfoDialog::close);
	}
}

void GUI_InfoDialog::tabIndexChangedInt(int idx)
{
	idx = std::min( (int) GUI_InfoDialog::Tab::Edit, idx);
	idx = std::max( (int) GUI_InfoDialog::Tab::Info, idx);

	tabIndexChanged( (GUI_InfoDialog::Tab) idx );
}

void GUI_InfoDialog::tabIndexChanged(GUI_InfoDialog::Tab idx)
{
	if(!ui){
		return;
	}

	switch(idx)
	{
		case GUI_InfoDialog::Tab::Edit:
			showTagEditTab();
			break;

		case GUI_InfoDialog::Tab::Lyrics:
			showLyricsTab();
			break;

		default:
			shoInfoTab();
			break;
	}
}

void GUI_InfoDialog::writeCoversToTracksClicked()
{
	showCoverEditTab();
}

void GUI_InfoDialog::coverChanged()
{
	int w = ui->btn_image->width();
	ui->btn_image->resize(w, w);
}

void GUI_InfoDialog::shoInfoTab()
{
	prepareInfo(m->metadataInterpretation);

	ui->tab_widget->setCurrentWidget(ui->ui_info_widget);
	ui->ui_info_widget->show();
	prepareCover(m->coverLocation);
}

void GUI_InfoDialog::showLyricsTab()
{
	if(m->tracks.isEmpty())
	{
		ui->tab_widget->setCurrentIndex(0);
		return;
	}

	initLyrics();
	ui->tab_widget->setCurrentWidget(m->uiLyrics);

	m->uiLyrics->setTrack(m->tracks.first());
	m->uiLyrics->show();
}

void GUI_InfoDialog::showTagEditTab()
{
	const MetaDataList localTracks = m->localTracks();
	if(localTracks.isEmpty())
	{
		ui->tab_widget->setCurrentIndex(0);
		return;
	}

	initTagEdit();

	ui->tab_widget->setCurrentWidget(m->uiTagEditor);

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
