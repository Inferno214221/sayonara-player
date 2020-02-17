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
	TabBar* tab_bar=nullptr;

	Private(QWidget* parent) :
		tab_bar(new TabBar(parent))
	{}
};

TabWidget::TabWidget(QWidget* parent) :
	QTabWidget(parent)
{
	m = Pimpl::make<Private>(this);
	this->setTabBar(m->tab_bar);

	connect(m->tab_bar, &TabBar::sigOpenFile, this, &TabWidget::sigOpenFile);
	connect(m->tab_bar, &TabBar::sigOpenDir, this, &TabWidget::sigOpenDir);
	connect(m->tab_bar, &TabBar::sigTabReset, this, &TabWidget::sigTabReset);
	connect(m->tab_bar, &TabBar::sigTabSave, this, &TabWidget::sigTabSave);
	connect(m->tab_bar, &TabBar::sigTabSaveAs, this, &TabWidget::sigTabSaveAs);
	connect(m->tab_bar, &TabBar::sigTabSaveToFile, this, &TabWidget::sigTabSaveToFile);

	connect(m->tab_bar, &TabBar::sigTabRename, this, &TabWidget::sigTabRename);
	connect(m->tab_bar, &TabBar::sigTabDelete, this, &TabWidget::sigTabDelete);
	connect(m->tab_bar, &TabBar::sigTabClear, this, &TabWidget::sigTabClear);
	connect(m->tab_bar, &TabBar::sigCurrentIndexChanged, this, &TabWidget::currentChanged);
	connect(m->tab_bar, &TabBar::sigAddTabClicked, this, &TabWidget::sigAddTabClicked);
	connect(m->tab_bar, &TabBar::sigMetadataDropped, this, &TabWidget::sigMetadataDropped);
	connect(m->tab_bar, &TabBar::sigFilesDropped, this, &TabWidget::sigFilesDropped);
}

TabWidget::~TabWidget() = default;

void TabWidget::showMenuItems(Playlist::MenuEntries entries)
{
	m->tab_bar->showMenuItems(entries);
}

void TabWidget::checkLastTab()
{
	int cur_idx, num_tabs;

	cur_idx = currentIndex();
	num_tabs = count();

	m->tab_bar->setTabsClosable(num_tabs > 2);

	QWidget* close_button = m->tab_bar->tabButton(num_tabs - 1, QTabBar::RightSide);
	if(close_button){
		close_button->setMaximumWidth(0);
		close_button->hide();
	}

	m->tab_bar->setTabIcon(num_tabs - 1, QIcon());

	if(cur_idx == num_tabs - 1 && num_tabs >= 2){
		this->setCurrentIndex(num_tabs - 2);
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
	this->setCurrentIndex(index);
	checkLastTab();
}

void TabWidget::insertTab(int index, QWidget *widget, const QIcon &icon, const QString& label)
{
	QTabWidget::insertTab(index, widget, icon, label);
	this->setCurrentIndex(index);
	checkLastTab();
}

bool TabWidget::wasDragFromPlaylist() const
{
	return m->tab_bar->wasDragFromPlaylist();
}

int TabWidget::getDragOriginTab() const
{
	return m->tab_bar->getDragOriginTab();
}


