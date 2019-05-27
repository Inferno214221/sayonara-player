/* GUI_AbstractLibrary.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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
#include "Utils/Language.h"

#include "Gui/Utils/EventFilter.h"
#include "Gui/Utils/GuiUtils.h"

#include <QLineEdit>
#include <QMenu>
#include <QShortcut>

using namespace Library;

struct GUI_AbstractLibrary::Private
{
	AbstractLibrary*	library = nullptr;
	SearchBar*			le_search=nullptr;

	Private(AbstractLibrary* library) :
		library(library)
	{}
};

GUI_AbstractLibrary::GUI_AbstractLibrary(AbstractLibrary* library, QWidget *parent) :
	Widget(parent)
{
	m = Pimpl::make<Private>(library);
}

GUI_AbstractLibrary::~GUI_AbstractLibrary() {}

void GUI_AbstractLibrary::init()
{
	m->le_search = le_search();

	lv_tracks()->init(m->library);
	lv_album()->init(m->library);
	lv_artist()->init(m->library);

	init_search_bar();
	init_shortcuts();

	connect(m->library, &AbstractLibrary::sig_delete_answer, this, &GUI_AbstractLibrary::show_delete_answer);

	connect(lv_artist(), &ItemView::sig_delete_clicked, this, &GUI_AbstractLibrary::item_delete_clicked);
	connect(lv_album(), &ItemView::sig_delete_clicked, this, &GUI_AbstractLibrary::item_delete_clicked);
	connect(lv_tracks(), &ItemView::sig_delete_clicked, this, &GUI_AbstractLibrary::tracks_delete_clicked);

	if(m->le_search)
	{
		connect(m->le_search, &SearchBar::sig_current_mode_changed, this, &GUI_AbstractLibrary::query_library);
	}

	ListenSetting(Set::Lib_LiveSearch, GUI_AbstractLibrary::live_search_changed);
}

void GUI_AbstractLibrary::init_search_bar()
{
	if(!m->le_search){
		return;
	}

	m->le_search->set_modes(this->search_options());
	m->le_search->set_current_mode(Filter::Fulltext);

	connect(m->le_search, &QLineEdit::returnPressed, this, &GUI_AbstractLibrary::search_return_pressed);
}


void GUI_AbstractLibrary::language_changed() {}

void GUI_AbstractLibrary::init_shortcuts()
{
	KeyPressFilter* kp_filter_lib = new KeyPressFilter(this);
	this->installEventFilter(kp_filter_lib);
	connect(kp_filter_lib, &KeyPressFilter::sig_key_pressed, this, &GUI_AbstractLibrary::key_pressed);
}


void GUI_AbstractLibrary::query_library()
{
	QString text;
	Filter::Mode current_mode = Filter::Mode::Fulltext;

	if(m->le_search)
	{
		text = m->le_search->text();
		current_mode = m->le_search->current_mode();
	}

	Filter filter = m->library->filter();
	filter.set_mode(current_mode);
	filter.set_filtertext(text, GetSetting(Set::Lib_SearchMode));
	filter.set_invalid_genre(m->le_search->has_invalid_genre_mode());

	m->library->change_filter(filter);
}

void GUI_AbstractLibrary::search_return_pressed()
{
	query_library();
}

void GUI_AbstractLibrary::search_edited(const QString& search)
{
	Q_UNUSED(search)

	if(GetSetting(Set::Lib_LiveSearch))
	{
		query_library();
	}
}

bool GUI_AbstractLibrary::has_selections() const
{
	return (m->library->selected_albums().count() > 0) ||
	(m->library->selected_artists().count() > 0) ||
	(m->library->selected_tracks().count() > 0);
}


void GUI_AbstractLibrary::key_pressed(int key)
{
	if(key == Qt::Key_Escape)
	{
		bool is_selected = has_selections();

		if(is_selected)
		{
			clear_selections();
		}

		else if(m->le_search)
		{
			if(m->le_search->text().size() > 0)
			{
				m->le_search->clear();
			}

			else
			{
				m->le_search->set_current_mode(Library::Filter::Mode::Fulltext);
			}
		}
	}
}

void GUI_AbstractLibrary::clear_selections()
{
	lv_album()->clearSelection();
	lv_artist()->clearSelection();
	lv_tracks()->clearSelection();
}

void GUI_AbstractLibrary::item_delete_clicked()
{
	int n_tracks = m->library->tracks().count();

	TrackDeletionMode answer = show_delete_dialog(n_tracks);
	if(answer != TrackDeletionMode::None) {
		m->library->delete_fetched_tracks(answer);
	}
}

void GUI_AbstractLibrary::tracks_delete_clicked()
{
	int n_tracks = m->library->current_tracks().count();

	TrackDeletionMode answer = show_delete_dialog(n_tracks);
	if(answer != TrackDeletionMode::None) {
		m->library->delete_current_tracks(answer);
	}
}

void GUI_AbstractLibrary::id3_tags_changed()
{
	m->library->refresh();
}

void GUI_AbstractLibrary::show_delete_answer(QString answer)
{
	Message::info(answer, Lang::get(Lang::Library));
}

void GUI_AbstractLibrary::live_search_changed()
{
	if(GetSetting(Set::Lib_LiveSearch)) {
		connect(m->le_search, &QLineEdit::textChanged, this, &GUI_AbstractLibrary::search_edited);
	}

	else {
		disconnect(m->le_search, &QLineEdit::textEdited, this, &GUI_AbstractLibrary::search_edited);
	}
}

