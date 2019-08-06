/* LibrarySearchBar.cpp */

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

#include "LibrarySearchBar.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Utils/EventFilter.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/GuiUtils.h"

#include <QList>
#include <QMenu>
#include <QVariant>
#include <QShortcut>

using Library::SearchBar;
using Library::Filter;
namespace Algorithm=Util::Algorithm;

struct SearchBar::Private
{
	QAction*			action_live_search=nullptr;
	QAction*			preference_action=nullptr;

	QMenu*				context_menu=nullptr;
	QList<Filter::Mode>	modes;
	int					cur_idx;
	bool				search_icon_initialized;
	bool				invald_genre_mode;

	Private() :
		cur_idx(-1),
		search_icon_initialized(false),
		invald_genre_mode(false)
	{}
};

SearchBar::SearchBar(QWidget* parent) :
	Parent(parent)
{
	m = Pimpl::make<Private>();

	this->setFocusPolicy(Qt::ClickFocus);
	this->setContextMenuPolicy(Qt::CustomContextMenu);
	this->setClearButtonEnabled(true);

	this->setShortcutEnabled(QKeySequence::Find, true);

	new QShortcut(QKeySequence::Find, this, SLOT(search_shortcut_pressed()), nullptr, Qt::WindowShortcut);
	new QShortcut(QKeySequence("F3"), this, SLOT(search_shortcut_pressed()), nullptr, Qt::WindowShortcut);

	connect(this, &QLineEdit::textChanged, this, &SearchBar::text_changed);
}

SearchBar::~SearchBar() = default;

void SearchBar::set_invalid_genre_mode(bool b)
{
	m->invald_genre_mode = b;
}

bool SearchBar::has_invalid_genre_mode() const
{
	return m->invald_genre_mode;
}

void SearchBar::text_changed(const QString& text)
{
	if(!m->search_icon_initialized)
	{
		QAction* a = this->findChild<QAction*>("_q_qlineeditclearaction");

		if(a){
			a->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
		}

		m->search_icon_initialized = true;
	}

	if(text.startsWith("f:", Qt::CaseInsensitive))
	{
		this->clear();
		this->set_current_mode(Filter::Fulltext);
	}

	else if(text.startsWith("g:", Qt::CaseInsensitive))
	{
		this->clear();
		this->set_current_mode(Filter::Genre);
	}

	else if(text.startsWith("p:", Qt::CaseInsensitive))
	{
		this->clear();
		this->set_current_mode(Filter::Filename);
	}

	else if(text.startsWith("t:", Qt::CaseInsensitive))
	{
		this->clear();
		this->set_current_mode(Filter::Track);
	}
}

void SearchBar::search_shortcut_pressed()
{
	if(!this->hasFocus()){
		this->setFocus();
	}

	else {
		this->set_next_mode();
	}
}

void SearchBar::set_modes(const QList<Filter::Mode>& modes)
{
	m->modes = modes;
	m->cur_idx = (m->modes.size() > 0) ? 0 : -1;

	init_context_menu();
}

QList<Filter::Mode> SearchBar::modes() const
{
	return m->modes;
}

void SearchBar::set_current_mode(Filter::Mode mode)
{
	m->cur_idx = Algorithm::indexOf(m->modes, [&mode](const Filter::Mode& f){
		return (f == mode);
	});

	language_changed();
}

void SearchBar::set_previous_mode()
{
	if(m->modes.isEmpty()){
		return;
	}

	else
	{
		m->cur_idx --;
		if(m->cur_idx < 0)
		{
			m->cur_idx = m->modes.size() -  1;
		}
	}

	set_current_mode(m->modes[m->cur_idx]);
}

void SearchBar::set_next_mode()
{
	if(m->modes.isEmpty()){
		return;
	}

	if(m->cur_idx < 0){
		m->cur_idx = 0;
	}

	else{
		m->cur_idx = (m->cur_idx + 1) % m->modes.size();
	}

	set_current_mode(m->modes[m->cur_idx]);
}

Filter::Mode SearchBar::current_mode() const
{
	if(m->cur_idx < 0 || m->cur_idx >= m->modes.count()){
		return Filter::Invalid;
	}

	return m->modes[m->cur_idx];
}


void SearchBar::init_context_menu()
{
	if(m->context_menu){
		return;
	}

	m->context_menu = new QMenu(this);

	{
		m->action_live_search = new QAction(m->context_menu);
		m->action_live_search->setText(Lang::get(Lang::LiveSearch));
		m->action_live_search->setCheckable(true);
		m->action_live_search->setChecked(GetSetting(Set::Lib_LiveSearch));
		connect(m->action_live_search, &QAction::triggered, this, &SearchBar::livesearch_triggered);
		ListenSettingNoCall(Set::Lib_LiveSearch, SearchBar::livesearch_changed);
	}

	{
		m->preference_action = new Gui::SearchPreferenceAction(m->context_menu);
	}

	auto* cm_filter = new Gui::ContextMenuFilter(this);
	connect(cm_filter, &Gui::ContextMenuFilter::sig_context_menu, m->context_menu, &QMenu::popup);
	this->installEventFilter(cm_filter);

	QList<QAction*> actions;
	for(const Filter::Mode mode : m->modes)
	{
		QVariant data = QVariant((int) (mode));
		QAction* action = new QAction(Filter::get_text(mode), this);

		action->setCheckable(false);
		action->setData(data);

		actions << action;

		connect(action, &QAction::triggered, this, [=](){
			this->set_current_mode(mode);
			emit sig_current_mode_changed();
		});
	}

	actions << m->action_live_search;
	actions << m->context_menu->addSeparator();
	actions << m->preference_action;

	m->context_menu->addActions(actions);
}

void SearchBar::keyPressEvent(QKeyEvent* e)
{
	if(e->key() == Qt::Key_Escape)
	{
		this->clear();
		this->set_current_mode(Filter::Fulltext);
		emit sig_current_mode_changed();
	}

	else if(e->key() == Qt::Key_Backspace)
	{
		if(this->text().isEmpty())
		{
			this->set_current_mode(Filter::Fulltext);
		}
	}

	else if(e->key() == Qt::Key_Up)
	{
		this->clear();
		this->set_previous_mode();
	}

	else if(e->key() == Qt::Key_Down)
	{
		this->clear();
		this->set_next_mode();
	}

	Parent::keyPressEvent(e);
}


void SearchBar::language_changed()
{
	Parent::language_changed();

	QString text = Lang::get(Lang::SearchNoun) + ": " + Filter::get_text(current_mode());
	this->setPlaceholderText(text);

	if(!m->context_menu) {
		return;
	}

	QList<QAction*> actions = m->context_menu->actions();
	for(QAction* action : actions)
	{
		QVariant data = action->data();
		if(data.isNull()){
			continue;
		}

		Filter::Mode mode = static_cast<Filter::Mode>(action->data().toInt());
		action->setText(Filter::get_text(mode));
	}

	m->action_live_search->setText(Lang::get(Lang::LiveSearch));
}

void SearchBar::skin_changed()
{
	QAction* a = this->findChild<QAction*>("_q_qlineeditclearaction");

	if(a){
		a->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
	}

	m->search_icon_initialized = true;

	Parent::skin_changed();
}

void SearchBar::livesearch_changed()
{
	if(m->action_live_search){
		m->action_live_search->setChecked(GetSetting(Set::Lib_LiveSearch));
	}
}

void SearchBar::livesearch_triggered(bool b)
{
	SetSetting(Set::Lib_LiveSearch, b);
}
