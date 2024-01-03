/* SearchBar.cpp */

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

#include "Searchbar.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

#include "Gui/Utils/EventFilter.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/Style.h"

#include <QList>
#include <QMenu>
#include <QShortcut>
#include <QVariant>

#include <array>
#include <utility>

using Library::SearchBar;
using Library::Filter;

namespace
{
	bool isValidMode(const Filter::Mode& mode)
	{
		return (mode != Filter::Mode::Invalid) && (mode != Filter::Mode::InvalidGenre);
	}

	class ModeManager
	{
		public:
			void setModes(const QList<Filter::Mode>& modes)
			{
				mModes = modes;
				mCurrentIndex = modes.isEmpty() ? -1 : 0;
			}

			QList<Filter::Mode> modes() const { return mModes; }

			void increaseIndex()
			{
				mCurrentIndex = (mCurrentIndex + 1) % mModes.size();
				if(!isValidMode(currentMode()))
				{
					increaseIndex();
				}
			}

			void decreaseIndex()
			{
				mCurrentIndex = (mCurrentIndex <= 0) ? (mModes.size() - 1) : (mCurrentIndex - 1);
				if(!isValidMode(currentMode()))
				{
					decreaseIndex();
				}
			}

			Filter::Mode currentMode()
			{
				return Util::between(mCurrentIndex, mModes)
				       ? mModes[mCurrentIndex]
				       : Filter::Mode::Invalid;
			}

			void setCurrentMode(Filter::Mode mode)
			{
				mCurrentIndex = mModes.indexOf(mode);
			}

		private:
			QList<Filter::Mode> mModes;
			int mCurrentIndex {-1};
	};

	QAction* createModeAction(Filter::Mode mode, QWidget* parent)
	{
		auto* action = new QAction(Filter::filterModeName(mode), parent);

		action->setCheckable(false);
		action->setData(mode);

		return action;
	}

	template<typename OnLiveSearchTriggered>
	QAction* initLiveSearchAction(QMenu* contextMenu, SearchBar* searchBar, OnLiveSearchTriggered callback)
	{
		auto* action = new QAction(contextMenu);

		action->setText(Lang::get(Lang::LiveSearch));
		action->setCheckable(true);
		action->setChecked(GetSetting(Set::Lib_LiveSearch));

		searchBar->connect(action, &QAction::triggered, searchBar, callback);

		return action;
	}

	void installContextMenuEventFilter(SearchBar* searchBar, QMenu* contextMenu)
	{
		auto* contextMenuFilter = new Gui::ContextMenuFilter(searchBar);
		searchBar->connect(contextMenuFilter, &Gui::ContextMenuFilter::sigContextMenu, contextMenu, &QMenu::popup);

		searchBar->installEventFilter(contextMenuFilter);
	}

	QString calcPlaceHolderText(Filter::Mode mode)
	{
		return Lang::get(Lang::SearchNoun) + ": " + Filter::filterModeName(mode);
	}

	void setClearButtonIcon(QLineEdit* lineEdit)
	{
		auto* clearAction = lineEdit->findChild<QAction*>("_q_qlineeditclearaction");
		if(clearAction)
		{
			clearAction->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
		}
	}

	void refreshModeActionText(QAction* action)
	{
		const auto data = action->data();
		if(data.canConvert<Filter::Mode>())
		{
			const auto filterMode = data.value<Filter::Mode>();
			action->setText(Filter::filterModeName(filterMode));
		}
	}

	void createFocusShortcut(QKeySequence ks, SearchBar* searchBar)
	{
		auto* shortcut = new QShortcut(ks, searchBar, nullptr, nullptr, Qt::WindowShortcut);
		searchBar->connect(shortcut, &QShortcut::activated, searchBar, [searchBar]() { searchBar->setFocus(); });
	}

	Filter::Mode getFilterModeFromSearchstring(const QString& searchString)
	{
		constexpr const auto filterMap = std::array
			{
				std::make_pair("f:", Filter::Fulltext),
				std::make_pair("g:", Filter::Genre),
				std::make_pair("p:", Filter::Filename)
			};

		const auto it = Util::Algorithm::find(filterMap, [&](const auto entry) {
			return (searchString.left(2).toLower() == entry.first);
		});

		return (it != filterMap.end()) ?
		       (it->second) :
		       Filter::Mode::Invalid;
	}

	void clearAndSetMode(SearchBar* searchBar, Filter::Mode newMode)
	{
		searchBar->clear();
		searchBar->setCurrentMode(newMode);
	}
}

struct SearchBar::Private
{
	ModeManager modeManager;

	QAction* actionLiveSearch {nullptr};
	QMenu* contextMenu {nullptr};
};

SearchBar::SearchBar(QWidget* parent) :
	Parent(parent)
{
	m = Pimpl::make<Private>();

	setFocusPolicy(Qt::ClickFocus);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setClearButtonEnabled(true);
	setShortcutEnabled(QKeySequence::Find, true);

	createFocusShortcut(QKeySequence::Find, this);
	createFocusShortcut(QKeySequence("F3"), this);

	connect(this, &QLineEdit::textChanged, this, &SearchBar::currentTextChanged);
}

SearchBar::~SearchBar() = default;

void SearchBar::setGenre(const QString& text, bool invalidGenreMode)
{
	setCurrentMode(invalidGenreMode ? Filter::Mode::InvalidGenre : Filter::Mode::Genre);
	setText(text);
}

void SearchBar::currentTextChanged(const QString& text)
{
	const auto mode = getFilterModeFromSearchstring(text);
	if(mode != Filter::Mode::Invalid)
	{
		clearAndSetMode(this, mode);
	}
}

void SearchBar::setModes(const QList<Filter::Mode>& modes)
{
	m->modeManager.setModes(modes);
	initContextMenu();
}

void SearchBar::setCurrentMode(const Filter::Mode mode)
{
	m->modeManager.setCurrentMode(mode);

	const auto placeholderText = calcPlaceHolderText(mode);
	setPlaceholderText(placeholderText);
}

Filter::Mode SearchBar::currentMode() const
{
	return m->modeManager.currentMode();
}

QList<QAction*> SearchBar::initModeActions(const QList<Filter::Mode>& modes)
{
	QList<QAction*> actions;

	for(const auto& mode: modes)
	{
		if(isValidMode(mode))
		{
			auto* action = createModeAction(mode, this);

			connect(action, &QAction::triggered, this, [=]() {
				setCurrentMode(mode);
				emit sigCurrentModeChanged();
			});

			actions << action;
		}
	}

	return actions;
}

void SearchBar::initContextMenu()
{
	if(!m->contextMenu)
	{
		m->contextMenu = new QMenu(this);
		m->actionLiveSearch = initLiveSearchAction(m->contextMenu, this, &SearchBar::livesearchTriggered);

		const auto actions = initModeActions(m->modeManager.modes())
			<< m->actionLiveSearch
			<< m->contextMenu->addSeparator()
			<< new Gui::SearchPreferenceAction(m->contextMenu);

		m->contextMenu->addActions(actions);

		installContextMenuEventFilter(this, m->contextMenu);

		ListenSettingNoCall(Set::Lib_LiveSearch, SearchBar::livesearchChanged);
	}
}

bool SearchBar::event(QEvent* event)
{
	// if space is pressed in an empty searchBar do not use it
	// in order to keep the play/pause shortcut working
	if(event->type() == QEvent::ShortcutOverride)
	{
		const auto* keyEvent = static_cast<QKeyEvent*>(event);
		const auto noModifierPressed = (keyEvent->modifiers() == Qt::NoModifier);
		const auto spacePressed = (keyEvent->key() == Qt::Key_Space);
		if(noModifierPressed && spacePressed && text().isEmpty())
		{
			return true;
		}
	}

	return Parent::event(event);
}

void SearchBar::keyPressEvent(QKeyEvent* keyEvent)
{
	if(keyEvent->key() == Qt::Key_Escape)
	{
		clearAndSetMode(this, Filter::Mode::Fulltext);
		emit sigCurrentModeChanged();
	}

	else if((keyEvent->key() == Qt::Key_Backspace) && text().isEmpty())
	{
		clearAndSetMode(this, Filter::Mode::Fulltext);
	}

	else if(keyEvent->key() == Qt::Key_Up)
	{
		m->modeManager.decreaseIndex();
		clearAndSetMode(this, m->modeManager.currentMode());
	}

	else if(keyEvent->key() == Qt::Key_Down)
	{
		m->modeManager.increaseIndex();
		clearAndSetMode(this, m->modeManager.currentMode());
	}

	Parent::keyPressEvent(keyEvent);
}

void SearchBar::languageChanged()
{
	Parent::languageChanged();

	const auto placeholderText = calcPlaceHolderText(m->modeManager.currentMode());
	setPlaceholderText(placeholderText);

	if(m->contextMenu)
	{
		const auto actions = m->contextMenu->actions();
		for(auto* action: actions)
		{
			refreshModeActionText(action);
		}

		m->actionLiveSearch->setText(Lang::get(Lang::LiveSearch));
	}
}

void SearchBar::skinChanged()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
	if(Style::isDark())
	{
		constexpr const auto value = 140;
		static const auto placeholderColor = QColor(value, value, value);

		auto palette = this->palette();
		palette.setColor(QPalette::Active, QPalette::PlaceholderText, placeholderColor);
		palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, placeholderColor);
		setPalette(palette);
	}
#endif

	setClearButtonIcon(this);
	Parent::skinChanged();
}

void SearchBar::livesearchChanged()
{
	m->actionLiveSearch->setChecked(GetSetting(Set::Lib_LiveSearch));
}

void SearchBar::livesearchTriggered(bool b)
{
	SetSetting(Set::Lib_LiveSearch, b);
}
