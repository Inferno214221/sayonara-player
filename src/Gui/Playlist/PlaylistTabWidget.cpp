/* PlaylistTabWidget.cpp */

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

#include "PlaylistTabWidget.h"
#include "PlaylistTabBar.h"
#include "PlaylistView.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Style.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/globals.h"

namespace
{
	void hideCloseButton(QTabBar* tabBar, int index)
	{
		auto* closeButton = tabBar->tabButton(index, QTabBar::RightSide);
		if(closeButton)
		{
			closeButton->hide();
			closeButton->resize(0, 0);
		}
	}

	void resizeCloseButton(QTabBar* tabBar, int index, int size)
	{
		auto* closeButton = tabBar->tabButton(index, QTabBar::RightSide);
		if(closeButton)
		{
			closeButton->resize(size, size);
			closeButton->show();
		}
	}

	void checkCloseButtons(QTabBar* tabBar, bool closeable)
	{
		tabBar->setTabsClosable(closeable);

		const auto size = tabBar->fontMetrics().height();
		for(int i = 0; i < tabBar->count() - 1; i++)
		{
			if(tabBar->tabsClosable())
			{
				resizeCloseButton(tabBar, i, size);
			}

			else
			{
				hideCloseButton(tabBar, i);
			}
		}
	}

	void checkLastTab(QTabBar* tabBar)
	{
		const auto lastTab = tabBar->count() - 1;
		tabBar->setTabIcon(lastTab, QIcon());
		hideCloseButton(tabBar, lastTab);
	}

	void checkTabs(QTabBar* tabBar)
	{
		const auto count = tabBar->count();

		if(Style::isDark())
		{
			checkCloseButtons(tabBar, count > 2);
		}

		const auto isLastTab = (tabBar->currentIndex() == (count - 1));
		if((count >= 2) && isLastTab)
		{
			tabBar->setCurrentIndex(count - 2);
		}

		checkLastTab(tabBar);
	}
}

namespace Playlist
{
	struct TabWidget::Private
	{
		TabBar* tabBar = nullptr;

		Private(QWidget* parent) :
			tabBar(new TabBar(parent)) {}
	};

	TabWidget::TabWidget(QWidget* parent) :
		QTabWidget(parent)
	{
		m = Pimpl::make<Private>(this);
		this->setTabBar(m->tabBar);

		connect(m->tabBar, &TabBar::sigOpenFile, this, &TabWidget::sigOpenFile);
		connect(m->tabBar, &TabBar::sigOpenDir, this, &TabWidget::sigOpenDir);
		connect(m->tabBar, &TabBar::sigTabReset, this, &TabWidget::sigTabReset);
		connect(m->tabBar, &TabBar::sigTabSave, this, &TabWidget::sigTabSave);
		connect(m->tabBar, &TabBar::sigTabSaveAs, this, &TabWidget::sigTabSaveAs);
		connect(m->tabBar, &TabBar::sigTabSaveToFile, this, &TabWidget::sigTabSaveToFile);

		connect(m->tabBar, &TabBar::sigTabRename, this, &TabWidget::sigTabRename);
		connect(m->tabBar, &TabBar::sigTabDelete, this, &TabWidget::sigTabDelete);
		connect(m->tabBar, &TabBar::sigTabClear, this, &TabWidget::sigTabClear);
		connect(m->tabBar, &TabBar::sigCurrentIndexChanged, this, &TabWidget::currentChanged);
		connect(m->tabBar, &TabBar::sigAddTabClicked, this, &TabWidget::sigAddTabClicked);
		connect(m->tabBar, &TabBar::sigMetadataDropped, this, &TabWidget::sigMetadataDropped);
		connect(m->tabBar, &TabBar::sigFilesDropped, this, &TabWidget::sigFilesDropped);
		connect(m->tabBar, &TabBar::sigLockTriggered, this, &TabWidget::sigLockTriggered);

		connect(m->tabBar, &TabBar::sigContextMenuRequested, this, &TabWidget::sigContextMenuRequested);
	}

	TabWidget::~TabWidget() = default;

	void TabWidget::showMenuItems(::Playlist::MenuEntries entries, const QPoint& position)
	{
		m->tabBar->showMenuItems(entries, position);
	}

	void TabWidget::checkTabButtons()
	{
		checkTabs(m->tabBar);
	}

	bool TabWidget::wasDragFromPlaylist() const
	{
		return m->tabBar->wasDragFromPlaylist();
	}

	int TabWidget::getDragOriginTab() const
	{
		return m->tabBar->getDragOriginTab();
	}

	View* TabWidget::viewByIndex(int index)
	{
		return Util::between(index, this->count() - 1)
		       ? dynamic_cast<View*>(this->widget(index))
		       : nullptr;
	}

	View* TabWidget::currentView()
	{
		return viewByIndex(currentIndex());
	}

	void TabWidget::setActiveTab(int index)
	{
		for(auto i = 0; i < count(); i++)
		{
			const auto height = this->fontMetrics().height();

			setIconSize(QSize(height, height));
			setTabIcon(i, QIcon());
		}

		auto* view = viewByIndex(index);
		if(view && (view->rowCount() > 0))
		{
			const auto icon = Gui::Icons::icon(Gui::Icons::PlayBorder);
			setTabIcon(index, icon);
		}
	}
}