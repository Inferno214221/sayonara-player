/* PlaylistTabWidget.cpp */

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

#include "PlaylistTabWidget.h"
#include "PlaylistTabBar.h"

#include "Utils/MetaData/MetaDataList.h"

using Playlist::TabWidget;
using Playlist::TabBar;

struct TabWidget::Private
{
	TabBar* tabBar=nullptr;

	Private(QWidget* parent) :
		tabBar(new TabBar(parent))
	{}
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
}

TabWidget::~TabWidget() = default;

void TabWidget::showMenuItems(Playlist::MenuEntries entries)
{
	m->tabBar->showMenuItems(entries);
}

void TabWidget::checkLastTab()
{
	m->tabBar->setTabsClosable(count() > 2);

	auto* closeButton = m->tabBar->tabButton(count() - 1, QTabBar::RightSide);
	if(closeButton){
		closeButton->setMaximumWidth(0);
		closeButton->hide();
	}

	m->tabBar->setTabIcon(count() - 1, QIcon());

	if(currentIndex() == count() - 1 && count() >= 2){
		this->setCurrentIndex(count() - 2);
	}
}

void TabWidget::removeTab(int index)
{
	QTabWidget::removeTab(index);
	checkLastTab();
}

void TabWidget::addTab(QWidget *widget, const QIcon &icon, const QString& label)
{
	QTabWidget::addTab(widget, icon, label);
	checkLastTab();
}

void TabWidget::addTab(QWidget *widget, const QString& label)
{
	QTabWidget::addTab(widget, label);
	checkLastTab();
}

void TabWidget::insertTab(int index, QWidget *widget, const QString& label)
{
	QTabWidget::insertTab(index, widget, label);
	setCurrentIndex(index);
	checkLastTab();
}

void TabWidget::insertTab(int index, QWidget *widget, const QIcon &icon, const QString& label)
{
	QTabWidget::insertTab(index, widget, icon, label);
	setCurrentIndex(index);
	checkLastTab();
}

bool TabWidget::wasDragFromPlaylist() const
{
	return m->tabBar->wasDragFromPlaylist();
}

int TabWidget::getDragOriginTab() const
{
	return m->tabBar->getDragOriginTab();
}


