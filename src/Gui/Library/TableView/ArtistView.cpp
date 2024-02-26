/* ArtistView.cpp */

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

#include "Utils/Language/Language.h"
#include "Utils/Library/MergeData.h"
#include "Utils/Library/Sorting.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Tagging/TagReader.h"
#include "Utils/Tagging/TagWriter.h"

using namespace Library;

struct ArtistView::Private
{
	AbstractLibrary* library = nullptr;
	ArtistModel* model = nullptr;
	QAction* albumArtistAction = nullptr;
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
	m->model = new ArtistModel(this, m->library);

	setModel(m->model);
	setItemDelegate(new Gui::StyledItemDelegate(this));

	connect(m->library, &AbstractLibrary::sigAllArtistsLoaded, this, &ArtistView::fill);

	ListenSetting(Set::Lib_UseViewClearButton, ArtistView::useClearButtonChanged);
}

void ArtistView::initContextMenu()
{
	ShortcutHandler* sch = ShortcutHandler::instance();

	ItemView::initContextMenu();

	auto* menu = contextMenu();
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
	using ColumnIndex::Artist;

	return {
		std::make_shared<ColumnHeaderArtist>(Artist::Name,
		                                     false,
		                                     ArtistSortorder::NameAsc,
		                                     ArtistSortorder::NameDesc,
		                                     160, // NOLINT(*-magic-numbers)
		                                     true),
		std::make_shared<ColumnHeaderArtist>(Artist::Tracks,
		                                     true,
		                                     ArtistSortorder::TrackcountAsc,
		                                     ArtistSortorder::TrackcountDesc,
		                                     Gui::Util::textWidth(fontMetrics(), "M 8888"))
	};
}

QByteArray ArtistView::columnHeaderState() const { return GetSetting(Set::Lib_ColStateArtists); }

void ArtistView::saveColumnHeaderState(const QByteArray& state)
{
	SetSetting(Set::Lib_ColStateArtists, state);
}

VariableSortorder ArtistView::sortorder() const { return GetSetting(Set::Lib_Sorting).artist; }

void ArtistView::applySortorder(const VariableSortorder s)
{
	if(std::holds_alternative<ArtistSortorder>(s))
	{
		m->library->changeArtistSortorder(std::get<ArtistSortorder>(s));
	}
}

bool ArtistView::autoResizeState() const { return GetSetting(Set::Lib_HeaderAutoResizeArtists); }

void ArtistView::saveAutoResizeState(bool b)
{
	SetSetting(Set::Lib_HeaderAutoResizeArtists, b);
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

bool ArtistView::isMergeable() const { return true; }

MD::Interpretation ArtistView::metadataInterpretation() const { return MD::Interpretation::Artists; }

void ArtistView::triggerSelectionChange(const IndexSet& indexes)
{
	m->library->selectedArtistsChanged(indexes);
}

void ArtistView::useClearButtonChanged()
{
	const auto b = GetSetting(Set::Lib_UseViewClearButton);
	useClearButton(b);
}

void ArtistView::albumArtistsTriggered(const bool /*b*/)
{
	SetSetting(Set::Lib_ShowAlbumArtists, m->albumArtistAction->isChecked());
}

void ArtistView::runMergeOperation(const Library::MergeData& mergedata)
{
	auto* uto = new Tagging::UserOperations(Tagging::TagReader::create(), Tagging::TagWriter::create(),
	                                        mergedata.libraryId(), this);
	connect(uto, &Tagging::UserOperations::sigFinished, uto, &Tagging::UserOperations::deleteLater);

	uto->mergeArtists(mergedata.sourceIds(), mergedata.targetId());
}

void ArtistView::showAlbumArtistsChanged()
{
	m->albumArtistAction->setChecked(GetSetting(Set::Lib_ShowAlbumArtists));
	setupColumnNames();
}

ItemModel* ArtistView::itemModel() const { return m->model; }

PlayActionEventHandler::TrackSet ArtistView::trackSet() const { return PlayActionEventHandler::TrackSet::All; }

void ArtistView::refreshView() { m->library->refreshArtists(); }

