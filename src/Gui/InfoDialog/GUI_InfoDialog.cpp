/* GUI_InfoDialog.cpp

 * Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#include "Components/Covers/CoverLocation.h"
#include "Components/MetaDataInfo/AlbumInfo.h"
#include "Components/MetaDataInfo/ArtistInfo.h"
#include "Components/MetaDataInfo/MetaDataInfo.h"
#include "Gui/Lyrics/GUI_Lyrics.h"
#include "Gui/Tagging/GUI_TagEdit.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"

#include <QTimer>
#include <QTabBar>
#include <QTableWidgetItem>

namespace
{
	enum StackedWidgetTab
	{
		StandardTab = 0,
		BusyTab = 1
	};

	std::unique_ptr<LibraryItemInfo>
	getMetadataInfo(const MD::Interpretation& metadataInterpretation, const MetaDataList& tracks)
	{
		switch(metadataInterpretation)
		{
			case MD::Interpretation::Artists:
				return std::make_unique<ArtistInfo>(tracks);
			case MD::Interpretation::Albums:
				return std::make_unique<AlbumInfo>(tracks);
			case MD::Interpretation::None:
			case MD::Interpretation::Tracks:
			default:
				return std::make_unique<MetaDataInfo>(tracks);
		}
	}

	[[nodiscard]] MetaDataList filterLocalTracks(MetaDataList tracks)
	{
		tracks.removeTracks([](const auto& track) {
			return Util::File::isWWW(track.filepath());
		});

		return tracks;
	}

	void initInfoTable(QTableWidget* table, const int rows)
	{
		table->clear();
		table->setRowCount(rows);
		table->setColumnCount(2);
		table->setAlternatingRowColors(true);
		table->setShowGrid(false);
		table->setEditTriggers(QTableView::NoEditTriggers);
		table->setSelectionBehavior(QTableView::SelectRows);
		table->setItemDelegate(new Gui::StyledItemDelegate(table));
		table->setMouseTracking(true);
	}

	[[nodiscard]] QTableWidgetItem* createDescriptionCell(const QString& key, const QFont& font)
	{
		auto* item = new QTableWidgetItem(key);
		item->setFont(font);

		return item;
	}

	[[nodiscard]] QWidget* createValueCell(const QString& data)
	{
		auto* label = new QLabel();
		label->setTextFormat(Qt::TextFormat::RichText);
		label->setWordWrap(true);
		label->setOpenExternalLinks(true);
		label->setText(data);

		return label;
	}

	void prepareInfoTable(QTableWidget* table, const QList<StringPair>& data)
	{
		initInfoTable(table, data.count());

		auto font = table->font();
		font.setBold(true);

		const auto fm = QFontMetrics(font);

		auto maxSize = 0;
		auto row = 0;
		for(const auto& [key, value]: data)
		{
			const auto textWidth = Gui::Util::textWidth(fm, key) + 20;
			maxSize = std::max(textWidth, maxSize);

			table->setItem(row, 0, createDescriptionCell(key, font));
			table->setCellWidget(row, 1, createValueCell(value));

			row++;
		}

		table->horizontalHeader()->resizeSection(0, maxSize);
		table->resizeRowsToContents();
	}

	void preparePaths(QListWidget* pathListWidget, const QStringList& paths)
	{
		pathListWidget->clear();
		pathListWidget->setAlternatingRowColors(true);
		pathListWidget->setEditTriggers(QListView::NoEditTriggers);
		pathListWidget->setTextElideMode(Qt::TextElideMode::ElideLeft);

		for(const auto& path: paths)
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
}

struct GUI_InfoDialog::Private
{
	GUI_TagEdit* uiTagEditor = nullptr;
	GUI_Lyrics* uiLyrics = nullptr;

	Cover::Location coverLocation;
	MetaDataList tracks;
	MD::Interpretation metadataInterpretation {MD::Interpretation::None};
};

GUI_InfoDialog::GUI_InfoDialog(QWidget* parent) :
	Dialog(parent),
	m {Pimpl::make<Private>()} {}

GUI_InfoDialog::~GUI_InfoDialog() = default;

void GUI_InfoDialog::init()
{
	if(ui)
	{
		return;
	}

	ui = std::make_shared<Ui::InfoDialog>();
	ui->setupUi(this);

	ui->tabWidget->setFocusPolicy(Qt::NoFocus);
	ui->tableInfo->setItemDelegate(new Gui::StyledItemDelegate(ui->tableInfo));
	ui->listPaths->setItemDelegate(new Gui::StyledItemDelegate(ui->listPaths));

	connect(ui->tabWidget, &QTabWidget::currentChanged, this, &GUI_InfoDialog::tabIndexChanged);
	connect(ui->btnWriteCoverToTracks, &QPushButton::clicked, this, &GUI_InfoDialog::writeCoversToTracksClicked);
	connect(ui->btnImage, &Gui::CoverButton::sigRejected, this, &GUI_InfoDialog::writeCoversToTracksClicked);
	connect(ui->btnImage, &Gui::CoverButton::sigCoverChanged, this, &GUI_InfoDialog::coverChanged);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_InfoDialog::close);
	connect(ui->btnChangeCover, &QPushButton::clicked, ui->btnImage, &Gui::CoverButton::sigTriggered);

	ui->stackedWidget->setCurrentIndex(StandardTab);
	ui->btnImage->setStyleSheet("QPushButton:hover {background-color: transparent;}");

	setModal(false);
}

void GUI_InfoDialog::prepareInfo(const MD::Interpretation mdInterpretation)
{
	const auto metadataInfo = getMetadataInfo(mdInterpretation, m->tracks);

	ui->btnWriteCoverToTracks->setVisible(metadataInfo->albums().size() == 1);
	ui->labTitle->setText(metadataInfo->header());
	ui->labSubheader->setText(metadataInfo->subheader());

	const auto infoMap = metadataInfo->additionalInfo();
	prepareInfoTable(ui->tableInfo, infoMap);

	const auto paths = metadataInfo->paths();
	preparePaths(ui->listPaths, paths);

	m->coverLocation = metadataInfo->coverLocation();
	prepareCover(m->coverLocation);
	ui->btnImage->setEnabled(m->coverLocation.isValid());
}

void GUI_InfoDialog::setMetadata(const MetaDataList& tracks, const MD::Interpretation interpretation)
{
	m->metadataInterpretation = interpretation;
	m->tracks = tracks;

	setBusy(m->tracks.isEmpty());
}

bool GUI_InfoDialog::hasMetadata() const { return !m->tracks.isEmpty(); }

GUI_InfoDialog::Tab GUI_InfoDialog::show(const GUI_InfoDialog::Tab tab)
{
	const auto size = GetSetting(Set::InfoDialog_Size);

	init();
	setBusy(m->tracks.isEmpty());
	enableTabs();
	switchTab(ui->tabWidget->isTabEnabled(+tab) ? tab : Tab::Info);
	show();

	if(size.isValid())
	{
		QTimer::singleShot(100, this, [this, size]() { // NOLINT(readability-magic-numbers)
			resize(size);
		});
	}

	return static_cast<Tab>(ui->tabWidget->currentIndex());
}

void GUI_InfoDialog::prepareCover(const Cover::Location& coverLocation)
{
	ui->btnImage->setCoverLocation(coverLocation);
}

void GUI_InfoDialog::initTagEdit()
{
	m->uiTagEditor = new GUI_TagEdit(ui->tabEditor);

	auto* tagEditLayout = ui->tabEditor->layout();
	tagEditLayout->addWidget(m->uiTagEditor);

	connect(m->uiTagEditor, &GUI_TagEdit::sigCancelled, this, &GUI_InfoDialog::close);
}

void GUI_InfoDialog::initLyrics()
{
	m->uiLyrics = new GUI_Lyrics(ui->tabLyrics);

	auto* lyricsLayout = ui->tabLyrics->layout();
	lyricsLayout->addWidget(m->uiLyrics);

	connect(m->uiLyrics, &GUI_Lyrics::sigClosed, this, &GUI_InfoDialog::close);
}

void GUI_InfoDialog::enableTabs()
{
	const auto lyricEnabled = (m->tracks.size() == 1);
	const auto tagEditEnabled = Util::Algorithm::contains(m->tracks, [](const auto& track) {
		return !Util::File::isWWW(track.filepath());
	});

	ui->tabWidget->setTabEnabled(+Tab::Edit, tagEditEnabled);
	ui->tabWidget->setTabEnabled(+Tab::Lyrics, lyricEnabled);
}

void GUI_InfoDialog::tabIndexChanged(int index)
{
	index = std::min(static_cast<int>(GUI_InfoDialog::Tab::Edit), index);
	index = std::max(static_cast<int>(GUI_InfoDialog::Tab::Info), index);

	switchTab(static_cast<GUI_InfoDialog::Tab>(index));
}

void GUI_InfoDialog::switchTab(const GUI_InfoDialog::Tab tab)
{
	if(tab == GUI_InfoDialog::Tab::Info || m->tracks.isEmpty())
	{
		showInfoTab();
	}

	else if(tab == GUI_InfoDialog::Tab::Edit)
	{
		showTagEditTab();
	}

	else if(tab == GUI_InfoDialog::Tab::Lyrics)
	{
		showLyricsTab();
	}

	if(ui->tabWidget->currentIndex() != +tab)
	{
		ui->tabWidget->setCurrentIndex(+tab);
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
	if(!m->uiLyrics)
	{
		initLyrics();
	}

	ui->tabWidget->setCurrentWidget(m->uiLyrics);
	m->uiLyrics->setTrack(m->tracks[0]);
	m->uiLyrics->show();
}

void GUI_InfoDialog::showTagEditTab()
{
	if(!m->uiTagEditor)
	{
		initTagEdit();
	}

	ui->tabWidget->setCurrentWidget(m->uiTagEditor);
	m->uiTagEditor->setMetadata(filterLocalTracks(m->tracks));
	m->uiTagEditor->showDefaultTab();
	m->uiTagEditor->show();
}

void GUI_InfoDialog::showCoverEditTab()
{
	const auto tab = show(GUI_InfoDialog::Tab::Edit);
	Q_ASSERT(ui);
	if(tab == GUI_InfoDialog::Tab::Edit)
	{
		m->uiTagEditor->showCoverTab();
	}
}

void GUI_InfoDialog::setBusy(const bool b)
{
	if(ui)
	{
		ui->stackedWidget->setCurrentIndex(b ? BusyTab : StandardTab);
	}
}

void GUI_InfoDialog::closeEvent(QCloseEvent* e)
{
	Dialog::closeEvent(e);
	m->tracks.clear();
}

void GUI_InfoDialog::resizeEvent(QResizeEvent* e)
{
	if(isVisible())
	{
		SetSetting(Set::InfoDialog_Size, e->size());
	}

	Dialog::resizeEvent(e);
}

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
