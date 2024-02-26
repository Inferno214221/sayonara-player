/* TrackView.cpp */

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
	AbstractLibrary* library = nullptr;
	TrackModel* model = nullptr;
};

TrackView::TrackView(QWidget* parent) :
	Library::TableView(parent)
{
	m = Pimpl::make<Private>();
}

TrackView::~TrackView() = default;

AbstractLibrary* TrackView::library() const { return m->library; }

void TrackView::initView(AbstractLibrary* library)
{
	m->library = library;

	m->model = new TrackModel(this, library);
	setModel(m->model);

	auto* trackDelegate = new RatingDelegate(static_cast<int>(ColumnIndex::Track::Rating), -1, this);
	setItemDelegate(trackDelegate);

	connect(library, &AbstractLibrary::sigAllTracksLoaded, this, &TrackView::fill);
}

ColumnHeaderList TrackView::columnHeaders() const
{
	const auto fm = fontMetrics();

	using ColumnIndex::Track;

	return {
		std::make_shared<ColumnHeaderTrack>(Track::TrackNumber,
		                                    true,
		                                    TrackSortorder::TrackNumberAsc,
		                                    TrackSortorder::TrackNumberDesc,
		                                    Gui::Util::textWidth(fm, "M888")),
		std::make_shared<ColumnHeaderTrack>(Track::Title,
		                                    false,
		                                    TrackSortorder::TitleAsc,
		                                    TrackSortorder::TitleDesc,
		                                    Gui::Util::textWidth(fm, "Some long song name"),
		                                    true),
		std::make_shared<ColumnHeaderTrack>(Track::Artist,
		                                    true,
		                                    TrackSortorder::ArtistAsc,
		                                    TrackSortorder::ArtistDesc,
		                                    Gui::Util::textWidth(fm, "Some long artist name"),
		                                    true),
		std::make_shared<ColumnHeaderTrack>(Track::Album,
		                                    true,
		                                    TrackSortorder::AlbumAsc,
		                                    TrackSortorder::AlbumDesc,
		                                    Gui::Util::textWidth(fm, "Some long album name"),
		                                    true),
		std::make_shared<ColumnHeaderTrack>(Track::Discnumber,
		                                    true,
		                                    TrackSortorder::DiscnumberAsc,
		                                    TrackSortorder::DiscnumberDesc,
		                                    Gui::Util::textWidth(fm, Lang::get(Lang::Disc) + " M888")),
		std::make_shared<ColumnHeaderTrack>(Track::Year,
		                                    true,
		                                    TrackSortorder::YearAsc,
		                                    TrackSortorder::YearDesc,
		                                    Gui::Util::textWidth(fm, "M8888")),
		std::make_shared<ColumnHeaderTrack>(Track::Length,
		                                    true,
		                                    TrackSortorder::LengthAsc,
		                                    TrackSortorder::LengthDesc,
		                                    Gui::Util::textWidth(fm, "8d 88h 88s")),
		std::make_shared<ColumnHeaderTrack>(Track::Bitrate,
		                                    true,
		                                    TrackSortorder::BitrateAsc,
		                                    TrackSortorder::BitrateDesc,
		                                    Gui::Util::textWidth(fm, "M8888 kBit/s")),
		std::make_shared<ColumnHeaderTrack>(Track::Filesize,
		                                    true,
		                                    TrackSortorder::SizeAsc,
		                                    TrackSortorder::SizeDesc,
		                                    Gui::Util::textWidth(fm, "M888.88 MB")),
		std::make_shared<ColumnHeaderTrack>(Track::Filetype,
		                                    true,
		                                    TrackSortorder::FiletypeAsc,
		                                    TrackSortorder::FiletypeDesc,
		                                    Gui::Util::textWidth(fm, "MFLAC")),
		std::make_shared<ColumnHeaderTrack>(Track::AddedDate,
		                                    true,
		                                    TrackSortorder::DateAddedAsc,
		                                    TrackSortorder::DateAddedDesc,
		                                    Gui::Util::textWidth(fm, "234/323/23423")),
		std::make_shared<ColumnHeaderTrack>(Track::ModifiedDate,
		                                    true,
		                                    TrackSortorder::DateModifiedAsc,
		                                    TrackSortorder::DateModifiedDesc,
		                                    Gui::Util::textWidth(fm, "234/323/23423")),
		std::make_shared<ColumnHeaderTrack>(Track::Rating,
		                                    true,
		                                    TrackSortorder::RatingAsc,
		                                    TrackSortorder::RatingDesc,
		                                    85), // NOLINT(*-magic-numbers)
	};
}

QByteArray TrackView::columnHeaderState() const { return GetSetting(Set::Lib_ColStateTracks); }

void TrackView::saveColumnHeaderState(const QByteArray& state) { SetSetting(Set::Lib_ColStateTracks, state); }

bool TrackView::autoResizeState() const { return GetSetting(Set::Lib_HeaderAutoResizeTracks); }

void TrackView::saveAutoResizeState(bool b) { SetSetting(Set::Lib_HeaderAutoResizeTracks, b); }

Library::ContextMenu::Entries TrackView::contextMenuEntries() const
{
	return (ItemView::contextMenuEntries() |
	        Library::ContextMenu::EntryLyrics |
	        Library::ContextMenu::EntryFilterExtension);
}

VariableSortorder TrackView::sortorder() const { return GetSetting(Set::Lib_Sorting).tracks; }

void TrackView::applySortorder(const VariableSortorder s)
{
	if(std::holds_alternative<TrackSortorder>(s))
	{
		m->library->changeTrackSortorder(std::get<TrackSortorder>(s));
	}
}

void TrackView::triggerSelectionChange(const IndexSet& lst)
{
	m->library->selectedTracksChanged(lst);
}

bool TrackView::isMergeable() const { return false; }

MD::Interpretation TrackView::metadataInterpretation() const { return MD::Interpretation::Tracks; }

ItemModel* TrackView::itemModel() const { return m->model; }

PlayActionEventHandler::TrackSet TrackView::trackSet() const { return PlayActionEventHandler::TrackSet::Selected; }

void TrackView::refreshView() { return m->library->refreshTracks(); }
