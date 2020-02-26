/* TrackView.cpp */

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

#include "TrackView.h"
#include "TrackModel.h"
#include "RatingDelegate.h"

#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Library/Header/ColumnIndex.h"
#include "Gui/Utils/GuiUtils.h"

#include "Components/Library/AbstractLibrary.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Language/Language.h"
#include "Utils/ExtensionSet.h"

#include <QStringList>

using namespace Library;

struct TrackView::Private
{
	AbstractLibrary*	library = nullptr;
};

TrackView::TrackView(QWidget* parent) :
	Library::TableView(parent)
{
	m = Pimpl::make<Private>();
}

TrackView::~TrackView() = default;

AbstractLibrary* TrackView::library() const
{
	return m->library;
}

void TrackView::initView(AbstractLibrary* library)
{
	m->library = library;

	auto* trackModel = new TrackModel(this, library);
	auto* trackDelegate = new RatingDelegate(this, int(ColumnIndex::Track::Rating), true);

	this->setItemModel(trackModel);
	this->setItemDelegate(trackDelegate);

	connect(library, &AbstractLibrary::sigAllTracksLoaded, this, &TrackView::fill);
}

ColumnHeaderList TrackView::columnHeaders() const
{
	const QFontMetrics fm(this->font());

	return ColumnHeaderList
	{
		std::make_shared<ColumnHeader>(ColumnHeader::Sharp, true, SortOrder::TrackNumAsc, SortOrder::TrackNumDesc, Gui::Util::textWidth(fm, "M888")),
		std::make_shared<ColumnHeader>(ColumnHeader::Title, false, SortOrder::TrackTitleAsc, SortOrder::TrackTitleDesc, 200, true),
		std::make_shared<ColumnHeader>(ColumnHeader::Artist, true, SortOrder::TrackArtistAsc, SortOrder::TrackArtistDesc, 200, true),
		std::make_shared<ColumnHeader>(ColumnHeader::Album, true, SortOrder::TrackAlbumAsc, SortOrder::TrackAlbumDesc, 200, true),
		std::make_shared<ColumnHeader>(ColumnHeader::Discnumber, true, SortOrder::TrackDiscnumberAsc, SortOrder::TrackDiscnumberDesc, Gui::Util::textWidth(fm, Lang::get(Lang::Disc) + " M888") ),
		std::make_shared<ColumnHeader>(ColumnHeader::Year, true, SortOrder::TrackYearAsc, SortOrder::TrackYearDesc, Gui::Util::textWidth(fm, "M8888")),
		std::make_shared<ColumnHeader>(ColumnHeader::DurationShort, true, SortOrder::TrackLenghtAsc, SortOrder::TrackLengthDesc, Gui::Util::textWidth(fm, "8d 88h 88s")),
		std::make_shared<ColumnHeader>(ColumnHeader::Bitrate, true, SortOrder::TrackBitrateAsc, SortOrder::TrackBitrateDesc, Gui::Util::textWidth(fm, "M8888 kBit/s")),
		std::make_shared<ColumnHeader>(ColumnHeader::Filesize, true, SortOrder::TrackSizeAsc, SortOrder::TrackSizeDesc, Gui::Util::textWidth(fm, "M888.88 MB")),
		std::make_shared<ColumnHeader>(ColumnHeader::Filetype, true, SortOrder::TrackFiletypeAsc, SortOrder::TrackFiletypeDesc, Gui::Util::textWidth(fm, "MFLAC")),
		std::make_shared<ColumnHeader>(ColumnHeader::AddedDate, true, SortOrder::TrackDateAddedAsc, SortOrder::TrackDateAddedDesc, Gui::Util::textWidth(fm, "234/323/23423")),
		std::make_shared<ColumnHeader>(ColumnHeader::ModifiedDate, true, SortOrder::TrackDateAddedAsc, SortOrder::TrackDateAddedDesc, Gui::Util::textWidth(fm, "234/323/23423")),
		std::make_shared<ColumnHeader>(ColumnHeader::Rating, true, SortOrder::TrackRatingAsc, SortOrder::TrackRatingDesc, 85),
	};
}

QByteArray TrackView::columnHeaderState() const
{
	return GetSetting(Set::Lib_ColStateTracks);
}

void TrackView::saveColumnHeaderState(const QByteArray& state)
{
	SetSetting(Set::Lib_ColStateTracks, state);
}

Library::ContextMenu::Entries TrackView::contextMenuEntries() const
{
	return (ItemView::contextMenuEntries() |
			Library::ContextMenu::EntryLyrics |
			Library::ContextMenu::EntryFilterExtension);
}

SortOrder TrackView::sortorder() const
{
	Sortings so = GetSetting(Set::Lib_Sorting);
	return so.so_tracks;
}

void TrackView::applySortorder(SortOrder s)
{
	m->library->changeTrackSortorder(s);
}

void TrackView::selectedItemsChanged(const IndexSet& lst)
{
	TableView::selectedItemsChanged(lst);
	m->library->selectedTracksChanged(lst);
}


void TrackView::playClicked()
{
	m->library->prepareCurrentTracksForPlaylist(false);
}

void TrackView::playNewTabClicked()
{
	TableView::playNewTabClicked();
	m->library->prepareCurrentTracksForPlaylist(true);
}


void TrackView::playNextClicked()
{
	TableView::playNextClicked();
	m->library->playNextCurrentTracks();
}

void TrackView::appendClicked()
{
	TableView::appendClicked();
	m->library->appendCurrentTracks();
}

void TrackView::refreshClicked()
{
	TableView::refreshClicked();
	m->library->refreshTracks();
}

bool TrackView::isMergeable() const
{
	return false;
}

MD::Interpretation TrackView::metadataInterpretation() const
{
	return MD::Interpretation::Tracks;
}
