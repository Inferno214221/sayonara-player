/* SearchBar.cpp */

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

#include "Searchbar.h"

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
namespace Algorithm = Util::Algorithm;

struct SearchBar::Private
{
	QAction* actionLiveSearch = nullptr;
	QAction* preferenceAction = nullptr;

	QMenu* contextMenu = nullptr;
	QList<Filter::Mode> modes;
	int currentIndex;
	bool searchIconInitialized;
	bool invaldGenreMode;

	Private() :
		currentIndex(-1),
		searchIconInitialized(false),
		invaldGenreMode(false) {}
};

SearchBar::SearchBar(QWidget* parent) :
	Parent(parent)
{
	m = Pimpl::make<Private>();

	this->setFocusPolicy(Qt::ClickFocus);
	this->setContextMenuPolicy(Qt::CustomContextMenu);
	this->setClearButtonEnabled(true);
	this->setShortcutEnabled(QKeySequence::Find, true);

	new QShortcut(QKeySequence::Find, this, SLOT(searchShortcutPressed()), nullptr, Qt::WindowShortcut);
	new QShortcut(QKeySequence("F3"), this, SLOT(searchShortcutPressed()), nullptr, Qt::WindowShortcut);

	connect(this, &QLineEdit::textChanged, this, &SearchBar::currentTextChanged);
}

SearchBar::~SearchBar() = default;

void SearchBar::setInvalidGenreMode(bool b)
{
	m->invaldGenreMode = b;
}

[[maybe_unused]] bool SearchBar::hasInvalidGenreMode() const
{
	return m->invaldGenreMode;
}

void SearchBar::currentTextChanged(const QString& text)
{
	if(!m->searchIconInitialized)
	{
		auto* a = this->findChild<QAction*>("_q_qlineeditclearaction");
		if(a)
		{
			a->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
		}

		m->searchIconInitialized = true;
	}

	if(text.startsWith("f:", Qt::CaseInsensitive))
	{
		this->clear();
		this->setCurrentMode(Filter::Fulltext);
	}

	else if(text.startsWith("g:", Qt::CaseInsensitive))
	{
		this->clear();
		this->setCurrentMode(Filter::Genre);
	}

	else if(text.startsWith("p:", Qt::CaseInsensitive))
	{
		this->clear();
		this->setCurrentMode(Filter::Filename);
	}
}

void SearchBar::searchShortcutPressed()
{
	if(!this->hasFocus())
	{
		this->setFocus();
	}

	else
	{
		this->setNextMode();
	}
}

[[maybe_unused]] void SearchBar::setModes(const QList<Filter::Mode>& modes)
{
	m->modes = modes;
	m->currentIndex = m->modes.empty() ? -1 : 0;

	initContextMenu();
}

[[maybe_unused]] QList<Filter::Mode> SearchBar::modes() const
{
	return m->modes;
}

void SearchBar::setCurrentMode(Filter::Mode mode)
{
	m->currentIndex = Algorithm::indexOf(m->modes, [&mode](const Filter::Mode& f) {
		return (f == mode);
	});

	languageChanged();
}

void SearchBar::setPreviousMode()
{
	if(m->modes.isEmpty())
	{
		return;
	}

	else
	{
		m->currentIndex--;
		if(m->currentIndex < 0)
		{
			m->currentIndex = m->modes.size() - 1;
		}
	}

	setCurrentMode(m->modes[m->currentIndex]);
}

void SearchBar::setNextMode()
{
	if(m->modes.isEmpty())
	{
		return;
	}

	if(m->currentIndex < 0)
	{
		m->currentIndex = 0;
	}

	else
	{
		m->currentIndex = (m->currentIndex + 1) % m->modes.size();
	}

	setCurrentMode(m->modes[m->currentIndex]);
}

Filter::Mode SearchBar::currentMode() const
{
	if(m->currentIndex < 0 || m->currentIndex >= m->modes.count())
	{
		return Filter::Invalid;
	}

	return m->modes[m->currentIndex];
}

void SearchBar::reset()
{
	this->clear();
	this->setCurrentMode(Filter::Mode::Fulltext);
}

void SearchBar::initContextMenu()
{
	if(m->contextMenu)
	{
		return;
	}

	m->contextMenu = new QMenu(this);

	{
		m->actionLiveSearch = new QAction(m->contextMenu);
		m->actionLiveSearch->setText(Lang::get(Lang::LiveSearch));
		m->actionLiveSearch->setCheckable(true);
		m->actionLiveSearch->setChecked(GetSetting(Set::Lib_LiveSearch));
		connect(m->actionLiveSearch, &QAction::triggered, this, &SearchBar::livesearchTriggered);
		ListenSettingNoCall(Set::Lib_LiveSearch, SearchBar::livesearchChanged);
	}

	{
		m->preferenceAction = new Gui::SearchPreferenceAction(m->contextMenu);
	}

	auto* cm_filter = new Gui::ContextMenuFilter(this);
	connect(cm_filter, &Gui::ContextMenuFilter::sigContextMenu, m->contextMenu, &QMenu::popup);
	this->installEventFilter(cm_filter);

	QList<QAction*> actions;
	for(const Filter::Mode mode : m->modes)
	{
		const QVariant data = QVariant(int(mode));
		auto* action = new QAction(Filter::text(mode), this);

		action->setCheckable(false);
		action->setData(data);

		actions << action;

		connect(action, &QAction::triggered, this, [=]() {
			this->setCurrentMode(mode);
			emit sigCurrentModeChanged();
		});
	}

	actions << m->actionLiveSearch;
	actions << m->contextMenu->addSeparator();
	actions << m->preferenceAction;

	m->contextMenu->addActions(actions);
}

bool SearchBar::event(QEvent* e)
{
	// Do not intercept "Space" key unless this edit box contains at least
	// one character. Otherwise the default Play/Pause shortcut wouldn't
	// work while this edit box has focus.
	if(e->type() == QEvent::ShortcutOverride)
	{
		const auto* ke = static_cast<QKeyEvent*>(e);
		if((ke->modifiers() == Qt::NoModifier) && (ke->key() == Qt::Key_Space) && text().isEmpty())
		{
			return true;
		}
	}
	return Parent::event(e);
}

void SearchBar::keyPressEvent(QKeyEvent* e)
{
	if(e->key() == Qt::Key_Escape)
	{
		this->clear();
		this->setCurrentMode(Filter::Fulltext);
		emit sigCurrentModeChanged();
	}

	else if(e->key() == Qt::Key_Backspace)
	{
		if(this->text().isEmpty())
		{
			this->setCurrentMode(Filter::Fulltext);
		}
	}

	else if(e->key() == Qt::Key_Up)
	{
		this->clear();
		this->setPreviousMode();
	}

	else if(e->key() == Qt::Key_Down)
	{
		this->clear();
		this->setNextMode();
	}

	Parent::keyPressEvent(e);
}

void SearchBar::languageChanged()
{
	Parent::languageChanged();

	QString text = Lang::get(Lang::SearchNoun) + ": " + Filter::text(currentMode());
	this->setPlaceholderText(text);

	if(!m->contextMenu)
	{
		return;
	}

	QList<QAction*> actions = m->contextMenu->actions();
	for(QAction* action : actions)
	{
		QVariant data = action->data();
		if(data.isNull())
		{
			continue;
		}

		Filter::Mode mode = Filter::Mode(action->data().toInt());
		action->setText(Filter::text(mode));
	}

	m->actionLiveSearch->setText(Lang::get(Lang::LiveSearch));
}

void SearchBar::skinChanged()
{
	auto* a = this->findChild<QAction*>("_q_qlineeditclearaction");
	if(a)
	{
		a->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
	}

	m->searchIconInitialized = true;

	Parent::skinChanged();
}

void SearchBar::livesearchChanged()
{
	if(m->actionLiveSearch)
	{
		m->actionLiveSearch->setChecked(GetSetting(Set::Lib_LiveSearch));
	}
}

void SearchBar::livesearchTriggered(bool b)
{
	SetSetting(Set::Lib_LiveSearch, b);
}
