/* ArtistView.cpp */

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

#include "ArtistView.h"
#include "ArtistModel.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Tagging/UserTaggingOperations.h"

#include "Gui/Library/Header/ColumnIndex.h"
#include "Gui/Library/Header/ColumnHeader.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Library/MergeData.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"

using namespace Library;

struct ArtistView::Private
{
	AbstractLibrary*	library=nullptr;
	QAction*			albumArtistAction=nullptr;
};

ArtistView::ArtistView(QWidget* parent) :
	Library::TableView(parent)
{
	m = Pimpl::make<Private>();
}

ArtistView::~ArtistView() = default;

AbstractLibrary* ArtistView::library() const
{
	return m->library;
}

void ArtistView::initView(AbstractLibrary* library)
{
	m->library = library;

	ArtistModel* artist_model = new ArtistModel(this, m->library);

	this->setItemModel(artist_model);
	this->setItemDelegate(new Gui::StyledItemDelegate(this));

	connect(m->library, &AbstractLibrary::sigAllArtistsLoaded, this, &ArtistView::fill);

	ListenSetting(Set::Lib_UseViewClearButton, ArtistView::useClearButtonChanged);
}

void ArtistView::initContextMenu()
{
	ShortcutHandler* sch = ShortcutHandler::instance();

	ItemView::initContextMenu();

	Library::ContextMenu* menu = contextMenu();

	m->albumArtistAction = new QAction(menu);
	m->albumArtistAction->setCheckable(true);
	m->albumArtistAction->setChecked(GetSetting(Set::Lib_ShowAlbumArtists));
	m->albumArtistAction->setShortcut(sch->shortcut(ShortcutIdentifier::AlbumArtists).sequence());

	ListenSetting(Set::Lib_ShowAlbumArtists, ArtistView::showAlbumArtistsChanged);

	connect(m->albumArtistAction, &QAction::triggered, this, &ArtistView::albumArtistsTriggered);

	auto* beforeAction = contextMenu()->action(ContextMenu::Entry::EntryReload);
	contextMenu()->insertAction(beforeAction, m->albumArtistAction);

	languageChanged();
}

ColumnHeaderList ArtistView::columnHeaders() const
{
	const QFontMetrics fm(this->font());
	using ColumnIndex::Artist;

	ColumnHeaderList columns
	{
		std::make_shared<ColumnHeaderArtist>(Artist::Name, false, SortOrder::ArtistNameAsc, SortOrder::ArtistNameDesc, 160, true),
		std::make_shared<ColumnHeaderArtist>(Artist::Tracks, true, SortOrder::ArtistTrackcountAsc, SortOrder::ArtistTrackcountDesc, Gui::Util::textWidth(fm, "M 8888"))
	};

	return columns;
}

QByteArray ArtistView::columnHeaderState() const
{
	return GetSetting(Set::Lib_ColStateArtists);
}

void ArtistView::saveColumnHeaderState(const QByteArray& state)
{
	SetSetting(Set::Lib_ColStateArtists, state);
}

SortOrder ArtistView::sortorder() const
{
	Library::Sortings so = GetSetting(Set::Lib_Sorting);
	return so.so_artists;
}

void ArtistView::applySortorder(SortOrder s)
{
	m->library->changeArtistSortorder(s);
}

void ArtistView::languageChanged()
{
	TableView::languageChanged();

	if(m->albumArtistAction)
	{
		auto* sch = ShortcutHandler::instance();
		m->albumArtistAction->setText(Lang::get(Lang::ShowAlbumArtists));
		m->albumArtistAction->setShortcut(sch->shortcut(ShortcutIdentifier::AlbumArtists).sequence());
	}
}

bool ArtistView::isMergeable() const
{
	return true;
}

MD::Interpretation ArtistView::metadataInterpretation() const
{
	return MD::Interpretation::Artists;
}

void ArtistView::selectedItemsChanged(const IndexSet& indexes)
{
	TableView::selectedItemsChanged(indexes);
	m->library->selectedArtistsChanged(indexes);
}


void ArtistView::playClicked()
{
	TableView::playClicked();
	m->library->prepareFetchedTracksForPlaylist(false);
}

void ArtistView::playNewTabClicked()
{
	TableView::playNewTabClicked();
	m->library->prepareFetchedTracksForPlaylist(true);
}

void ArtistView::playNextClicked()
{
	TableView::playNextClicked();
	m->library->playNextFetchedTracks();
}

void ArtistView::appendClicked()
{
	TableView::appendClicked();
	m->library->appendFetchedTracks();
}

void ArtistView::refreshClicked()
{
	TableView::refreshClicked();
	m->library->refreshArtists();
}

void ArtistView::useClearButtonChanged()
{
	bool b = GetSetting(Set::Lib_UseViewClearButton);
	useClearButton(b);
}

void ArtistView::albumArtistsTriggered(bool b)
{
	Q_UNUSED(b)
	SetSetting(Set::Lib_ShowAlbumArtists, m->albumArtistAction->isChecked());
}

void ArtistView::runMergeOperation(const Library::MergeData& mergedata)
{
	Tagging::UserOperations* uto = new Tagging::UserOperations(mergedata.libraryId(), this);

	connect(uto, &Tagging::UserOperations::sigFinished, uto, &Tagging::UserOperations::deleteLater);

	uto->mergeArtists(mergedata.sourceIds(), mergedata.targetId());
}


void ArtistView::showAlbumArtistsChanged()
{
	m->albumArtistAction->setChecked(GetSetting(Set::Lib_ShowAlbumArtists));
	setupColumnNames();
}
