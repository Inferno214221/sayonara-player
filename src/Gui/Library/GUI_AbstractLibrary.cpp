/* GUI_AbstractLibrary.cpp */

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

#include "GUI_AbstractLibrary.h"
#include "TableView.h"

#include "Components/Library/AbstractLibrary.h"
#include "Gui/Library/Utils/LibrarySearchBar.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Message/Message.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Library/Filter.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"

#include "Gui/Utils/EventFilter.h"

#include <QLineEdit>
#include <QMenu>
#include <QShortcut>

using namespace Library;

struct GUI_AbstractLibrary::Private
{
	AbstractLibrary*	library = nullptr;
	SearchBar*			leSearch=nullptr;

	Private(AbstractLibrary* library) :
		library(library)
	{}
};

GUI_AbstractLibrary::GUI_AbstractLibrary(AbstractLibrary* library, QWidget* parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>(library);
}

GUI_AbstractLibrary::~GUI_AbstractLibrary() = default;

void GUI_AbstractLibrary::init()
{
	m->leSearch = leSearch();

	lvTracks()->init(m->library);
	lvAlbum()->init(m->library);
	lvArtist()->init(m->library);

	initSearchBar();
	initShortcuts();

	connect(m->library, &AbstractLibrary::sigDeleteAnswer, this, &GUI_AbstractLibrary::showDeleteAnswer);

	connect(lvArtist(), &ItemView::sigDeleteClicked, this, &GUI_AbstractLibrary::itemDeleteClicked);
	connect(lvAlbum(), &ItemView::sigDeleteClicked, this, &GUI_AbstractLibrary::itemDeleteClicked);
	connect(lvTracks(), &ItemView::sigDeleteClicked, this, &GUI_AbstractLibrary::tracksDeleteClicked);

	if(m->leSearch)
	{
		connect(m->leSearch, &SearchBar::sigCurrentModeChanged, this, &GUI_AbstractLibrary::queryLibrary);
	}

	ListenSetting(Set::Lib_LiveSearch, GUI_AbstractLibrary::liveSearchChanged);
}

void GUI_AbstractLibrary::initSearchBar()
{
	if(!m->leSearch){
		return;
	}

	m->leSearch->setModes(this->searchOptions());
	m->leSearch->setCurrentMode(Filter::Fulltext);

	connect(m->leSearch, &QLineEdit::returnPressed, this, &GUI_AbstractLibrary::searchTriggered);
}

void GUI_AbstractLibrary::languageChanged() {}

void GUI_AbstractLibrary::initShortcuts()
{
	auto* keyPressFilter = new Gui::KeyPressFilter(this);
	this->installEventFilter(keyPressFilter);
	connect(keyPressFilter, &Gui::KeyPressFilter::setKeyPressed, this, &GUI_AbstractLibrary::keyPressed);
}

void GUI_AbstractLibrary::queryLibrary()
{
	QString text;
	Filter::Mode current_mode = Filter::Mode::Fulltext;

	if(m->leSearch)
	{
		text = m->leSearch->text();
		current_mode = m->leSearch->currentMode();
	}

	Filter filter = m->library->filter();
	filter.setMode(current_mode);
	filter.setFiltertext(text, GetSetting(Set::Lib_SearchMode));
	filter.setInvalidGenre(m->leSearch->hasInvalidGenreMode());

	m->library->changeFilter(filter);
}

void GUI_AbstractLibrary::searchTriggered()
{
	queryLibrary();
}

void GUI_AbstractLibrary::searchEdited(const QString& search)
{
	Q_UNUSED(search)

	if(GetSetting(Set::Lib_LiveSearch) || search.isEmpty())
	{
		queryLibrary();
	}
}

bool GUI_AbstractLibrary::hasSelections() const
{
	return (m->library->selectedAlbums().count() > 0) ||
	(m->library->selectedArtists().count() > 0);
}

void GUI_AbstractLibrary::keyPressed(int key)
{
	using Library::Filter;

	if(key != Qt::Key_Escape)
	{
		return;
	}

	if(hasSelections())
	{
		clearSelections();
	}

	else if(m->leSearch)
	{
		if(!m->leSearch->text().isEmpty())
		{
			m->leSearch->clear();
		}

		else
		{
			m->leSearch->setCurrentMode(Filter::Mode::Fulltext);
			m->library->refetch();
		}
	}
}

void GUI_AbstractLibrary::clearSelections()
{
	lvAlbum()->clearSelection();
	lvArtist()->clearSelection();
	lvTracks()->clearSelection();
}

void GUI_AbstractLibrary::itemDeleteClicked()
{
	int n_tracks = m->library->tracks().count();

	TrackDeletionMode answer = showDeleteDialog(n_tracks);
	if(answer != TrackDeletionMode::None) {
		m->library->deleteFetchedTracks(answer);
	}
}

void GUI_AbstractLibrary::tracksDeleteClicked()
{
	int n_tracks = m->library->currentTracks().count();

	TrackDeletionMode answer = showDeleteDialog(n_tracks);
	if(answer != TrackDeletionMode::None) {
		m->library->deleteCurrentTracks(answer);
	}
}

void GUI_AbstractLibrary::showDeleteAnswer(QString answer)
{
	Message::info(answer, Lang::get(Lang::Library));
}

void GUI_AbstractLibrary::liveSearchChanged()
{
	if(GetSetting(Set::Lib_LiveSearch)) {
		connect(m->leSearch, &QLineEdit::textChanged, this, &GUI_AbstractLibrary::searchEdited);
	}

	else {
		disconnect(m->leSearch, &QLineEdit::textEdited, this, &GUI_AbstractLibrary::searchEdited);
	}
}
