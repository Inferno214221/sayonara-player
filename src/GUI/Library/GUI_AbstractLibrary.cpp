/* GUI_AbstractLibrary.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include "Utils/Message/Message.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Library/Filter.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/Logger/Logger.h"

#include "GUI/Utils/EventFilter.h"
#include "GUI/Utils/PreferenceAction.h"
#include "GUI/Utils/GuiUtils.h"

#include <QLineEdit>
#include <QMenu>
#include <QShortcut>

using namespace Library;

struct GUI_AbstractLibrary::Private
{
	AbstractLibrary*	library = nullptr;
	QLineEdit*			le_search=nullptr;

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

	Set::listen<Set::Lib_LiveSearch>(this, &GUI_AbstractLibrary::_sl_live_search_changed);
}

void GUI_AbstractLibrary::init_search_bar()
{
	m->le_search->setFocusPolicy(Qt::ClickFocus);
	m->le_search->setContextMenuPolicy(Qt::CustomContextMenu);
	m->le_search->setClearButtonEnabled(true);

	QList<QAction*> actions;
	QList<Filter::Mode> filters = search_options();
	for(const Filter::Mode filter_mode : filters)
	{
		QVariant data = QVariant((int) (filter_mode));
		QAction* action = new QAction(Filter::get_text(filter_mode), m->le_search);

		action->setCheckable(false);
		action->setData(data);

		actions << action;

		connect(action, &QAction::triggered, this, [=](){
			search_mode_changed(filter_mode);
		});
	}

	QMenu* menu = new QMenu(m->le_search);
	actions << menu->addSeparator();
	actions << new SearchPreferenceAction(menu);

	menu->addActions(actions);

	ContextMenuFilter* cm_filter = new ContextMenuFilter(m->le_search);
	connect(cm_filter, &ContextMenuFilter::sig_context_menu, menu, &QMenu::popup);

	m->le_search->installEventFilter(cm_filter);
	connect(m->le_search, &QLineEdit::returnPressed, this, &GUI_AbstractLibrary::search_return_pressed);

	search_mode_changed(Filter::Fulltext);
}

void GUI_AbstractLibrary::language_changed()
{
	Filter::Mode mode = static_cast<Filter::Mode>(m->le_search->property("search_mode").toInt());
	QString text = Lang::get(Lang::SearchNoun) + ": " + Filter::get_text(mode);

	m->le_search->setPlaceholderText(text);
}


void GUI_AbstractLibrary::init_shortcuts()
{
	if(!m->le_search){
		return;
	}

	m->le_search->setShortcutEnabled(QKeySequence::Find, true);

	new QShortcut(QKeySequence::Find, m->le_search, SLOT(setFocus()), nullptr, Qt::WindowShortcut);
	new QShortcut(QKeySequence("F3"), m->le_search, SLOT(setFocus()), nullptr, Qt::WindowShortcut);

	KeyPressFilter* kp_filter_lib = new KeyPressFilter(this);
	this->installEventFilter(kp_filter_lib);
	connect(kp_filter_lib, &KeyPressFilter::sig_esc_pressed, this, &GUI_AbstractLibrary::esc_pressed);

	KeyPressFilter* kp_filter_search = new KeyPressFilter(m->le_search);
	m->le_search->installEventFilter(kp_filter_search);
	connect(kp_filter_search, &KeyPressFilter::sig_esc_pressed, this, &GUI_AbstractLibrary::search_esc_pressed);
}

void GUI_AbstractLibrary::query_library()
{
	Filter filter = m->library->filter();
	Filter::Mode current_mode = static_cast<Filter::Mode>(m->le_search->property("search_mode").toInt());

	filter.set_mode(current_mode);
	filter.set_filtertext(m->le_search->text(), _settings->get<Set::Lib_SearchMode>());

	m->library->change_filter(filter);
}

void GUI_AbstractLibrary::search_return_pressed()
{
	query_library();
}

void GUI_AbstractLibrary::search_edited(const QString& search)
{
	static bool search_icon_initialized=false;
	if(!search_icon_initialized)
	{
		QAction* a = m->le_search->findChild<QAction*>("_q_qlineeditclearaction");

		if(a){
			a->setIcon(Gui::Util::icon("broom.png"));
		}

		search_icon_initialized = true;
	}

	if(search.startsWith("f:", Qt::CaseInsensitive))
	{
		m->le_search->clear();
		search_mode_changed(::Library::Filter::Fulltext);
	}

	else if(search.startsWith("g:", Qt::CaseInsensitive))
	{
		m->le_search->clear();
		search_mode_changed(::Library::Filter::Genre);
	}

	else if(search.startsWith("p:", Qt::CaseInsensitive))
	{
		m->le_search->clear();
		search_mode_changed(::Library::Filter::Filename);
	}

	else if(_settings->get<Set::Lib_LiveSearch>())
	{
		query_library();
	}
}

void GUI_AbstractLibrary::search_esc_pressed()
{
	m->le_search->clear();

	search_mode_changed(Filter::Fulltext);
	query_library();
}

void GUI_AbstractLibrary::search_mode_changed(Filter::Mode mode)
{
	QString text = Lang::get(Lang::SearchNoun) + ": " + Filter::get_text(mode);

	m->le_search->setPlaceholderText(text);
	m->le_search->setProperty("search_mode", (int) mode);

	query_library();
}

void GUI_AbstractLibrary::esc_pressed()
{
	bool is_selected = (
		(lv_album()->selected_items().count() > 0) ||
		(lv_artist()->selected_items().count() > 0) ||
		(lv_tracks()->selected_items().count() > 0)
	);

	if(is_selected)
	{
		lv_album()->clearSelection();
		lv_artist()->clearSelection();
		lv_tracks()->clearSelection();
	}

	else
	{
		search_esc_pressed();
	}
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


void GUI_AbstractLibrary::_sl_live_search_changed()
{
	if(_settings->get<Set::Lib_LiveSearch>()) {
		connect(m->le_search, &QLineEdit::textChanged, this, &GUI_AbstractLibrary::search_edited);
	}

	else {
		disconnect(m->le_search, &QLineEdit::textEdited, this, &GUI_AbstractLibrary::search_edited);
	}
}
