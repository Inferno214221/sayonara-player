/* LibraryViewAlbum.cpp */

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

#include "AlbumView.h"
#include "AlbumModel.h"
#include "RatingDelegate.h"

#include "Gui/Library/Utils/DiscPopupMenu.h"
#include "Gui/Library/Header/ColumnIndex.h"
#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"

#include "Components/Tagging/UserTaggingOperations.h"
#include "Components/Library/AbstractLibrary.h"

#include "Utils/Library/MergeData.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Tagging/TagWriter.h"
#include "Utils/Utils.h"

#include <QHeaderView>
#include <QVBoxLayout>

using namespace Library;

struct AlbumView::Private
{
	AbstractLibrary* library = nullptr;
	DiscPopupMenu* discmenu = nullptr;
	QPoint discmenuPoint;
	Tagging::TagReaderPtr tagReader {Tagging::TagReader::create()};
	Tagging::TagWriterPtr tagWriter {Tagging::TagWriter::create()};
};

AlbumView::AlbumView(QWidget* parent) :
	TableView(parent)
{
	m = Pimpl::make<Private>();

	connect(this, &QTableView::clicked, this, &AlbumView::indexClicked);
}

AlbumView::~AlbumView() = default;

void AlbumView::initView(AbstractLibrary* library)
{
	m->library = library;

	auto* model = new AlbumModel(m->tagReader, m->tagWriter, m->library, this);
	auto* delegate = new RatingDelegate(static_cast<int>(ColumnIndex::Album::Rating), 0, this);

	this->setItemModel(model);
	this->setItemDelegate(delegate);

	connect(m->library, &AbstractLibrary::sigAllAlbumsLoaded, this, &AlbumView::fill);

	ListenSetting(Set::Lib_UseViewClearButton, AlbumView::useClearButtonChanged);
}

AbstractLibrary* AlbumView::library() const
{
	return m->library;
}

ColumnHeaderList AlbumView::columnHeaders() const
{
	const QFontMetrics fm(this->font());
	using ColumnIndex::Album;

	return ColumnHeaderList
		{
			std::make_shared<ColumnHeaderAlbum>(Album::MultiDisc,
			                                    true,
			                                    AlbumSortorder::NoSorting,
			                                    AlbumSortorder::NoSorting,
			                                    Gui::Util::textWidth(fm, "MM")),
			std::make_shared<ColumnHeaderAlbum>(Album::Name,
			                                    false,
			                                    AlbumSortorder::NameAsc,
			                                    AlbumSortorder::NameDesc,
			                                    160,
			                                    true),
			//	std::make_shared<ColumnHeaderAlbum>(Album::AlbumArtist, true, SortOrder::NoSorting, SortOrder::NoSorting, 160, true),
			std::make_shared<ColumnHeaderAlbum>(Album::Duration,
			                                    true,
			                                    AlbumSortorder::DurationAsc,
			                                    AlbumSortorder::DurationDesc,
			                                    Gui::Util::textWidth(fm, "888h 888h 888m")),
			std::make_shared<ColumnHeaderAlbum>(Album::NumSongs,
			                                    true,
			                                    AlbumSortorder::TracksAsc,
			                                    AlbumSortorder::TracksDesc,
			                                    Gui::Util::textWidth(fm, "num tracks")),
			std::make_shared<ColumnHeaderAlbum>(Album::Year,
			                                    true,
			                                    AlbumSortorder::YearAsc,
			                                    AlbumSortorder::YearDesc,
			                                    Gui::Util::textWidth(fm, "M 8888")),
			std::make_shared<ColumnHeaderAlbum>(Album::Rating,
			                                    true,
			                                    AlbumSortorder::RatingAsc,
			                                    AlbumSortorder::RatingDesc,
			                                    85)
		};
}

QByteArray AlbumView::columnHeaderState() const
{
	return GetSetting(Set::Lib_ColStateAlbums);
}

void AlbumView::saveColumnHeaderState(const QByteArray& state)
{
	SetSetting(Set::Lib_ColStateAlbums, state);
}

VariableSortorder AlbumView::sortorder() const
{
	return GetSetting(Set::Lib_Sorting).album;
}

void AlbumView::applySortorder(const VariableSortorder s)
{
	if(std::holds_alternative<AlbumSortorder>(s))
	{
		m->library->changeAlbumSortorder(std::get<AlbumSortorder>(s));
	}
}

void AlbumView::showContextMenu(const QPoint& p)
{
	deleteDiscmenu();
	TableView::showContextMenu(p);
}

void AlbumView::indexClicked(const QModelIndex& idx)
{
	if(idx.column() == int(ColumnIndex::Album::MultiDisc))
	{
		QModelIndexList selections = this->selectionModel()->selectedRows();
		if(selections.size() == 1)
		{
			initDiscmenu(idx);
			showDiscmenu();
		}
	}
}

/* where to show the popup */
void AlbumView::calcDiscmenuPoint(QModelIndex idx)
{
	QHeaderView* verticalHeader = this->verticalHeader();

	m->discmenuPoint = QCursor::pos();

	QRect box = this->geometry();
	box.moveTopLeft(this->parentWidget()->mapToGlobal(box.topLeft()));

	if(!box.contains(m->discmenuPoint))
	{
		m->discmenuPoint.setX(box.x() + (box.width() * 2) / 3);
		m->discmenuPoint.setY(box.y());

		QPoint discMenuPoint = parentWidget()->pos() - QPoint(0, verticalHeader->sizeHint().height());
		while(idx.row() != indexAt(discMenuPoint).row())
		{
			discMenuPoint.setY(discMenuPoint.y() + 10);
			m->discmenuPoint.setY(m->discmenuPoint.y() + 10);
		}
	}
}

void AlbumView::initDiscmenu(QModelIndex idx)
{
	int row = idx.row();
	deleteDiscmenu();

	if(!idx.isValid() ||
	   (row >= model()->rowCount()) ||
	   (row < 0))
	{
		return;
	}

	const Album& album = m->library->albums().at(size_t(row));
	if(album.discnumbers().size() < 2)
	{
		return;
	}

	calcDiscmenuPoint(idx);

	m->discmenu = new DiscPopupMenu(this, album.discnumbers());

	connect(m->discmenu, &DiscPopupMenu::sigDiscPressed, this, &AlbumView::sigDiscPressed);
}

void AlbumView::deleteDiscmenu()
{
	if(!m->discmenu)
	{
		return;
	}

	m->discmenu->hide();
	m->discmenu->close();

	disconnect(m->discmenu, &DiscPopupMenu::sigDiscPressed, this, &AlbumView::sigDiscPressed);

	m->discmenu->deleteLater();
	m->discmenu = nullptr;
}

void AlbumView::showDiscmenu()
{
	if(!m->discmenu) { return; }

	m->discmenu->popup(m->discmenuPoint);
}

void AlbumView::playClicked()
{
	TableView::playClicked();
	m->library->prepareFetchedTracksForPlaylist(false);
}

void AlbumView::playNewTabClicked()
{
	TableView::playNewTabClicked();
	m->library->prepareFetchedTracksForPlaylist(true);
}

void AlbumView::playNextClicked()
{
	TableView::playNextClicked();
	m->library->playNextFetchedTracks();
}

void AlbumView::appendClicked()
{
	TableView::appendClicked();
	m->library->appendFetchedTracks();
}

void AlbumView::selectedItemsChanged(const IndexSet& indexes)
{
	TableView::selectedItemsChanged(indexes);
	m->library->selectedAlbumsChanged(indexes);
}

void AlbumView::refreshClicked()
{
	TableView::refreshClicked();
	m->library->refreshAlbums();
}

void AlbumView::runMergeOperation(const MergeData& mergedata)
{
	auto* uto = new Tagging::UserOperations(m->tagReader, m->tagWriter, mergedata.libraryId(), this);
	connect(uto, &Tagging::UserOperations::sigFinished, uto, &Tagging::UserOperations::deleteLater);

	uto->mergeAlbums(mergedata.sourceIds(), mergedata.targetId());
}

bool AlbumView::isMergeable() const
{
	return true;
}

MD::Interpretation AlbumView::metadataInterpretation() const
{
	return MD::Interpretation::Albums;
}

bool AlbumView::autoResizeState() const
{
	return GetSetting(Set::Lib_HeaderAutoResizeAlbums);
}

void AlbumView::saveAutoResizeState(bool b)
{
	SetSetting(Set::Lib_HeaderAutoResizeAlbums, b);
}

void AlbumView::useClearButtonChanged()
{
	bool b = GetSetting(Set::Lib_UseViewClearButton);
	useClearButton(b);
}

