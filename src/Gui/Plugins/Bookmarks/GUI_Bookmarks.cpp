/* GUI_Bookmarks.cpp */

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


#include "GUI_Bookmarks.h"

#include "Gui/Plugins/ui_GUI_Bookmarks.h"

#include "Utils/Utils.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Message/Message.h"
#include "Utils/Language/Language.h"

#include "Components/Bookmarks/Bookmark.h"
#include "Components/Bookmarks/Bookmarks.h"

#define NoBookmarkText QStringLiteral("--:--")

using Gui::ContextMenu;
using Gui::ContextMenuEntries;

struct GUI_Bookmarks::Private
{
	Bookmarks* bookmarks;

	Private(Bookmarks* bookmarks) :
		bookmarks {bookmarks} {}
};

GUI_Bookmarks::GUI_Bookmarks(Bookmarks* bookmarks, QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>(bookmarks);
}

GUI_Bookmarks::~GUI_Bookmarks()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

QString GUI_Bookmarks::name() const
{
	return "Bookmarks";
}

QString GUI_Bookmarks::displayName() const
{
	return Lang::get(Lang::Bookmarks);
}

void GUI_Bookmarks::retranslate()
{
	ui->retranslateUi(this);

	if(m->bookmarks->count() == 0)
	{
		ui->cb_bookmarks->clear();
		ui->cb_bookmarks->addItem(tr("No bookmarks found"), -1);
	}
}

void GUI_Bookmarks::initUi()
{
	if(isUiInitialized())
	{
		return;
	}

	setupParent(this, &ui);

	connect(m->bookmarks, &Bookmarks::sigBookmarksChanged, this, &GUI_Bookmarks::bookmarksChanged);
	connect(m->bookmarks, &Bookmarks::sigNextChanged, this, &GUI_Bookmarks::nextChanged);
	connect(m->bookmarks, &Bookmarks::sigPreviousChanged, this, &GUI_Bookmarks::previousChanged);

	connect(ui->btn_tool, &Gui::MenuToolButton::sigNew, this, &GUI_Bookmarks::newClicked);
	connect(ui->btn_tool, &Gui::MenuToolButton::sigDelete, this, &GUI_Bookmarks::deleteClicked);
	connect(ui->btn_prev, &QPushButton::clicked, this, &GUI_Bookmarks::previousClicked);
	connect(ui->btn_next, &QPushButton::clicked, this, &GUI_Bookmarks::nextClicked);
	connect(ui->cb_loop, &QCheckBox::clicked, this, &GUI_Bookmarks::loopToggled);
	connect(ui->cb_bookmarks, combo_current_index_changed_int, this, &GUI_Bookmarks::currentIndexChanged);

	ui->btn_tool->showAction(Gui::ContextMenu::EntryNew, false);
	ui->btn_tool->showAction(Gui::ContextMenu::EntryDelete, false);

	disablePrevious();
	disableNext();

	bookmarksChanged();
}

void GUI_Bookmarks::bookmarksChanged()
{
	if(!isUiInitialized())
	{
		return;
	}

	const auto& bookmarks = m->bookmarks->bookmarks();

	disconnect(ui->cb_bookmarks, combo_current_index_changed_int, this, &GUI_Bookmarks::currentIndexChanged);

	ui->cb_bookmarks->clear();
	for(const auto& bookmark: bookmarks)
	{
		ui->cb_bookmarks->addItem(bookmark.name(), static_cast<int>(bookmark.timestamp()));
	}

	if(bookmarks.isEmpty())
	{
		ui->cb_bookmarks->addItem(tr("No bookmarks found"), -1);
	}

	const auto& track = m->bookmarks->currentTrack();

	ui->btn_tool->showAction(ContextMenu::EntryNew, (track.id() >= 0));
	ui->btn_tool->showAction(ContextMenu::EntryDelete, !bookmarks.isEmpty());

	const auto visible = (track.id() >= 0 && bookmarks.size() > 0);
	ui->controls->setVisible(visible);

	connect(ui->cb_bookmarks, combo_current_index_changed_int, this, &GUI_Bookmarks::currentIndexChanged);
}

void GUI_Bookmarks::disablePrevious()
{
	if(isUiInitialized())
	{
		ui->btn_prev->setEnabled(false);
		ui->btn_prev->setText(NoBookmarkText);
	}
}

void GUI_Bookmarks::disableNext()
{
	if(isUiInitialized())
	{
		ui->btn_next->setEnabled(false);
		ui->btn_next->setText(NoBookmarkText);
	}
}

void GUI_Bookmarks::previousChanged(const Bookmark& bookmark)
{
	if(!isUiInitialized())
	{
		return;
	}

	ui->btn_prev->setEnabled(bookmark.isValid());
	ui->cb_loop->setEnabled(ui->btn_next->isEnabled());

	if(!bookmark.isValid())
	{
		disablePrevious();
		return;
	}

	ui->btn_prev->setText(Util::msToString(bookmark.timestamp() * 1000, "$M:$S"));
}

void GUI_Bookmarks::nextChanged(const Bookmark& bookmark)
{
	if(!isUiInitialized())
	{
		return;
	}

	ui->btn_next->setEnabled(bookmark.isValid());
	ui->cb_loop->setEnabled(ui->btn_next->isEnabled());

	if(!bookmark.isValid())
	{
		disableNext();
		return;
	}

	ui->btn_next->setText(Util::msToString(bookmark.timestamp() * 1000, "$M:$S"));
}

void GUI_Bookmarks::currentIndexChanged(int currentIndex)
{
	ui->btn_tool->showAction(ContextMenu::EntryDelete, (currentIndex >= 0));

	const auto data = ui->cb_bookmarks->itemData(currentIndex).toInt();
	if(data >= 0 && currentIndex >= 0)
	{
		m->bookmarks->jumpTo(currentIndex);
	}
}

void GUI_Bookmarks::nextClicked()
{
	m->bookmarks->jumpNext();
}

void GUI_Bookmarks::previousClicked()
{
	m->bookmarks->jumpPrevious();
}

void GUI_Bookmarks::loopToggled(bool b)
{
	const auto success = m->bookmarks->setLoop(b);
	if(!success)
	{
		ui->cb_loop->setChecked(success);
	}
}

void GUI_Bookmarks::newClicked()
{
	const auto status = m->bookmarks->create();
	if(status == BookmarkStorage::CreationStatus::NoDBTrack)
	{
		Message::warning(tr("Sorry, bookmarks can only be set for library tracks at the moment."),
		                 Lang::get(Lang::Bookmarks));
	}
}

void GUI_Bookmarks::deleteClicked()
{
	const auto index = ui->cb_bookmarks->currentIndex();
	m->bookmarks->remove(index);
}
