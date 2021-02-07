/* GUI_Playlist.cpp */

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

/*
 *  Created on: Apr 6, 2011
 */

#include "GUI_Playlist.h"
#include "PlaylistTabWidget.h"
#include "PlaylistView.h"
#include "PlaylistActionMenu.h"

#include "Gui/Playlist/ui_GUI_Playlist.h"
#include "Gui/Library/Utils/GUI_DeleteDialog.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"

#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Language/Language.h"
#include "Utils/globals.h"
#include "Utils/Message/Message.h"
#include "Utils/Parser/PlaylistParser.h"

#include "Interfaces/PlayManager.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"

#include <QTabBar>
#include <QShortcut>

using Playlist::Handler;
using Playlist::View;
using Playlist::TabWidget;

namespace
{
	Message::Answer showSaveMessageBox(QWidget* parent, Util::SaveAsAnswer answer)
	{
		switch(answer)
		{
			case Util::SaveAsAnswer::OtherError:
				return Message::warning(parent->tr("Cannot save playlist."), Lang::get(Lang::SaveAs));

			case Util::SaveAsAnswer::NameAlreadyThere:
			{
				const auto question = QString("%1.\n%2")
					.arg(parent->tr("Playlist exists"))
					.arg(Lang::get(Lang::Overwrite).question());
				return Message::question_yn(question, Lang::get(Lang::SaveAs));
			}

			case Util::SaveAsAnswer::NotStorable:
				return Message::warning(parent->tr("Playlists are currently only supported for library tracks."),
				                        parent->tr("Save playlist"));

			default:
				return Message::Answer::Undefined;
		}
	}

	void calcTotalTimeLabel(PlaylistPtr playlist, QLabel* labTotalTime)
	{
		const auto durationMs = (playlist) ? playlist->runningTime() : MilliSeconds {0};
		const auto rows = (playlist) ? playlist->count() : 0;
		auto playlistString = Lang::getWithNumber(Lang::NrTracks, rows);
		if(rows == 0)
		{
			playlistString = labTotalTime->tr("Playlist empty");
		}

		if(durationMs > 0)
		{
			playlistString += " - " + Util::msToString(durationMs, "$He $M:$S");
		}

		labTotalTime->setText(playlistString);
		labTotalTime->setContentsMargins(0, 2, 0, 2);
	}

	void checkPlaylistMenu(PlaylistPtr playlist, TabWidget* tabWidget)
	{
		using Playlist::MenuEntry;

		auto entries = Playlist::MenuEntries {MenuEntry::None};

		const auto temporary = (playlist) ? playlist->isTemporary() : true;
		const auto wasChanged = (playlist) ? playlist->wasChanged() : false;
		const auto isEmpty = (playlist) ? (playlist->count() == 0) : true;

		const auto saveEnabled = (!temporary);
		const auto saveToFileEnabled = (!isEmpty);
		const auto deleteEnabled = (!temporary);
		const auto resetEnabled = (!temporary && wasChanged);
		const auto closeEnabled = (tabWidget->count() > 2);
		const auto clearEnabled = (!isEmpty);

		entries |= MenuEntry::OpenFile;
		entries |= MenuEntry::OpenDir;
		entries |= MenuEntry::SaveAs;
		entries |= MenuEntry::Rename;

		if(saveEnabled)
		{
			entries |= MenuEntry::Save;
		}
		if(saveToFileEnabled)
		{
			entries |= MenuEntry::SaveToFile;
		}
		if(deleteEnabled)
		{
			entries |= MenuEntry::Delete;
		}
		if(resetEnabled)
		{
			entries |= MenuEntry::Reset;
		}
		if(closeEnabled)
		{
			entries |= MenuEntry::Close;
			entries |= MenuEntry::CloseOthers;
		}
		if(clearEnabled)
		{
			entries |= MenuEntry::Clear;
		}

		tabWidget->showMenuItems(entries);
	}

	void checkPlaylistName(PlaylistPtr playlist, TabWidget* tabWidget)
	{
		if(playlist)
		{
			auto name = playlist->name();
			if(!playlist->isTemporary() && playlist->wasChanged())
			{
				name.prepend("*");
			}

			tabWidget->setTabText(playlist->index(), name);
		}
	}

	View* viewByIndex(TabWidget* tabWidget, int index)
	{
		return Util::between(index, tabWidget->count() - 1)
		       ? static_cast<View*>(tabWidget->widget(index))
		       : nullptr;
	}

	View* currentView(TabWidget* tabWidget)
	{
		return viewByIndex(tabWidget, tabWidget->currentIndex());
	}
} // namespace end

struct GUI_Playlist::Private
{
	Playlist::Handler* playlistHandler;
	PlayManager* playManager;

	Private(Playlist::Handler* playlistHandler, PlayManager* playManager) :
		playlistHandler(playlistHandler),
		playManager(playManager)
	{}
};

GUI_Playlist::GUI_Playlist(QWidget* parent) :
	Widget(parent)
{}

void GUI_Playlist::init(Playlist::Handler* playlistHandler, PlayManager* playManager)
{
	m = Pimpl::make<Private>(playlistHandler, playManager);

	ui = new Ui::PlaylistWindow();
	ui->setupUi(this);

	setAcceptDrops(true);

	for(int i=0; i<m->playlistHandler->count(); i++)
	{
		playlistAdded(i);
	}

	ui->twPlaylists->setCurrentIndex(m->playlistHandler->currentIndex());

	connect(m->playlistHandler, &Handler::sigPlaylistNameChanged, this, &GUI_Playlist::playlistNameChanged);
	connect(m->playlistHandler, &Handler::sigNewPlaylistAdded, this, &GUI_Playlist::playlistAdded);
	connect(m->playlistHandler, &Handler::sigCurrentPlaylistChanged, this, &GUI_Playlist::playlistIdxChanged);
	connect(m->playlistHandler, &Handler::sigPlaylistClosed, this, &GUI_Playlist::playlistClosed);

	connect(m->playManager, &PlayManager::sigPlaylistFinished, this, &GUI_Playlist::checkTabIcon);
	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &GUI_Playlist::checkTabIcon);

	connect(ui->twPlaylists, &TabWidget::sigAddTabClicked,
	        this, [&]() { m->playlistHandler->createEmptyPlaylist(false); });
	connect(ui->twPlaylists, &TabWidget::tabCloseRequested, m->playlistHandler, &Playlist::Handler::closePlaylist);
	connect(ui->twPlaylists, &TabWidget::currentChanged, m->playlistHandler, &Playlist::Handler::setCurrentIndex);
	connect(ui->twPlaylists, &TabWidget::sigTabDelete, this, &GUI_Playlist::tabDeletePlaylistClicked);
	connect(ui->twPlaylists, &TabWidget::sigTabSave, this, &GUI_Playlist::tabSavePlaylistClicked);
	connect(ui->twPlaylists, &TabWidget::sigTabSaveAs, this, &GUI_Playlist::tabSavePlaylistAsClicked);
	connect(ui->twPlaylists, &TabWidget::sigTabSaveToFile, this, &GUI_Playlist::tabSavePlaylistToFileClicked);
	connect(ui->twPlaylists, &TabWidget::sigTabRename, this, &GUI_Playlist::tabRenameClicked);
	connect(ui->twPlaylists, &TabWidget::sigTabClear, this, &GUI_Playlist::clearButtonPressed);
	connect(ui->twPlaylists, &TabWidget::sigTabReset, this, &GUI_Playlist::tabResetClicked);
	connect(ui->twPlaylists, &TabWidget::sigMetadataDropped, this, &GUI_Playlist::tabMetadataDropped);
	connect(ui->twPlaylists, &TabWidget::sigFilesDropped, this, &GUI_Playlist::tabFilesDropped);
	connect(ui->twPlaylists, &TabWidget::sigOpenFile, this, &GUI_Playlist::openFileClicked);
	connect(ui->twPlaylists, &TabWidget::sigOpenDir, this, &GUI_Playlist::openDirClicked);

	connect(ui->btnClear, &QPushButton::clicked, this, &GUI_Playlist::clearButtonPressed);

	initToolButton();

	ListenSetting(Set::PL_ShowBottomBar, GUI_Playlist::showBottomBarChanged);
	ListenSetting(Set::PL_ShowClearButton, GUI_Playlist::showClearButtonChanged);
}

GUI_Playlist::~GUI_Playlist()
{
	while(ui->twPlaylists->count() > 1)
	{
		const auto lastTab = ui->twPlaylists->count() - 1;
		auto* lastWidget = ui->twPlaylists->widget(lastTab);
		ui->twPlaylists->removeTab(lastTab);

		if(lastWidget)
		{
			delete lastWidget;
		}
	}

	if(ui)
	{
		delete ui;
		ui = nullptr;
	}
}

void GUI_Playlist::initToolButton()
{
	auto* menu = new Playlist::ActionMenu(this);
	const auto actions = menu->actions();
	for(auto* action : actions)
	{
		ui->toolButton->registerAction(action);
	}

	ui->toolButton->registerPreferenceAction(new Gui::PlaylistPreferenceAction(this));
}

void GUI_Playlist::clearButtonPressed(int playlistIndex)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(playlist)
	{
		playlist->clear();
	}
}

void GUI_Playlist::bookmarkSelected(int trackIndex, Seconds timestamp)
{
	auto playlist = m->playlistHandler->playlist(ui->twPlaylists->currentIndex());
	if(playlist)
	{
		playlist->changeTrack(trackIndex, timestamp * 1000);
	}
}

void GUI_Playlist::tabMetadataDropped(int playlistIndex, const MetaDataList& tracks)
{
	const auto originTab = ui->twPlaylists->getDragOriginTab();
	if(playlistIndex == ui->twPlaylists->count() - 1)
	{
		playlistIndex = m->playlistHandler->createEmptyPlaylist(false);
	}

	if(ui->twPlaylists->wasDragFromPlaylist())
	{
		auto* playlistView = viewByIndex(ui->twPlaylists, originTab);
		if(playlistView)
		{
			playlistView->removeSelectedRows();
		}
	}

	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(playlist)
	{
		if(originTab == playlistIndex)
		{
			playlist->insertTracks(tracks, 0);
		}

		else
		{
			playlist->appendTracks(tracks);
		}
	}
}

void GUI_Playlist::tabFilesDropped(int playlistIndex, const QStringList& paths)
{
	if(ui->twPlaylists->wasDragFromPlaylist())
	{   // a playlist would have had metadata and no paths
		return;
	}

	if(playlistIndex == ui->twPlaylists->count() - 1)
	{
		const auto name = m->playlistHandler->requestNewPlaylistName();
		m->playlistHandler->createPlaylist(paths, name);
	}

	else
	{
		auto playlist = m->playlistHandler->playlist(playlistIndex);
		m->playlistHandler->createPlaylist(paths, playlist->name());
	}
}

void GUI_Playlist::doubleClicked(int row)
{
	auto playlist = m->playlistHandler->playlist(ui->twPlaylists->currentIndex());
	if(playlist)
	{
		playlist->changeTrack(row);
	}
}

void GUI_Playlist::openFileClicked(int playlistIndex, const QStringList& files)
{
	const auto playlist = m->playlistHandler->playlist(playlistIndex);
	const auto name = (playlist) ? playlist->name() : QString();
	m->playlistHandler->createPlaylist(files, name);
}

void GUI_Playlist::openDirClicked(int playlistIndex, const QString& dir)
{
	openFileClicked(playlistIndex, QStringList {dir});
}

void GUI_Playlist::deleteTracksClicked(const IndexSet& rows)
{
	auto dialog = GUI_DeleteDialog(rows.count(), this);
	dialog.exec();

	const auto deletionMode = dialog.answer();
	if(deletionMode == Library::TrackDeletionMode::None)
	{
		return;
	}

	m->playlistHandler->deleteTracks(ui->twPlaylists->currentIndex(), rows, deletionMode);
}

void GUI_Playlist::playlistNameChanged(int playlistIndex)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(!playlist)
	{
		return;
	}

	const auto name = playlist->name();
	checkPlaylistName(playlist, ui->twPlaylists);

	for(auto i = ui->twPlaylists->count() - 2; i >= 0; i--)
	{
		if((i != playlistIndex) &&
		   (ui->twPlaylists->tabText(i) == name))
		{
			ui->twPlaylists->removeTab(i);
		}
	}
}

void GUI_Playlist::playlistChanged(int playlistIndex)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	checkPlaylistName(playlist, ui->twPlaylists);

	if(playlistIndex != ui->twPlaylists->currentIndex())
	{
		return;
	}

	calcTotalTimeLabel(playlist, ui->labTotalTime);
	checkPlaylistMenu(playlist, ui->twPlaylists);
}

void GUI_Playlist::playlistIdxChanged(int playlistIndex)
{
	if(!Util::between(playlistIndex, ui->twPlaylists->count() - 1))
	{
		return;
	}

	ui->twPlaylists->setCurrentIndex(playlistIndex);

	this->setFocusProxy(ui->twPlaylists->currentWidget());

	auto playlist = m->playlistHandler->playlist(playlistIndex);
	calcTotalTimeLabel(playlist, ui->labTotalTime);
	checkPlaylistMenu(playlist, ui->twPlaylists);
}

void GUI_Playlist::playlistAdded(int playlistIndex)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(playlist)
	{
		const auto name = playlist->name();
		auto* view = new View(m->playlistHandler, playlist, ui->twPlaylists);

		ui->twPlaylists->insertTab(ui->twPlaylists->count() - 1, view, name);

		connect(view, &View::sigDoubleClicked, this, &GUI_Playlist::doubleClicked);
		connect(view, &View::sigDeleteTracks, this, &GUI_Playlist::deleteTracksClicked);
		connect(view, &View::sigBookmarkPressed, this, &GUI_Playlist::bookmarkSelected);
		connect(playlist.get(), &Playlist::Playlist::sigItemsChanged, this, &GUI_Playlist::playlistChanged);

		ui->twPlaylists->setCurrentIndex(playlistIndex);
	}
}

void GUI_Playlist::playlistClosed(int playlistIndex)
{
	auto* playlistWidget = ui->twPlaylists->widget(playlistIndex);
	ui->twPlaylists->removeTab(playlistIndex);

	auto* playlistView = currentView(ui->twPlaylists);
	if(playlistView)
	{
		playlistView->setFocus();
	}

	delete playlistWidget;
}

void GUI_Playlist::tabSavePlaylistClicked(int playlistIndex)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(!playlist)
	{
		return;
	}

	const auto success = playlist->save();
	if(success == Util::SaveAsAnswer::Success)
	{
		auto oldString = ui->twPlaylists->tabText(playlistIndex);
		if(oldString.startsWith("*"))
		{
			oldString.remove(0, 1);
		}

		ui->twPlaylists->setTabText(playlistIndex, oldString);
	}

	showSaveMessageBox(this, success);
}

void GUI_Playlist::tabSavePlaylistAsClicked(int playlistIndex, const QString& newName)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(!playlist || newName.isEmpty())
	{
		return;
	}

	auto success = playlist->saveAs(newName, false);
	if(success == Util::SaveAsAnswer::NameAlreadyThere)
	{
		const auto answer = showSaveMessageBox(this, success);
		if(answer == Message::Answer::No)
		{
			return;
		}

		success = playlist->saveAs(newName, true);
	}

	showSaveMessageBox(this, success);
}

void GUI_Playlist::tabSavePlaylistToFileClicked(int playlistIndex, const QString& filename)
{
	const auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(playlist)
	{
		PlaylistParser::saveM3UPlaylist(filename, playlist->tracks(), false);
	}
}

void GUI_Playlist::tabRenameClicked(int playlistIndex, const QString& newName)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(playlist)
	{
		const auto success = playlist->rename(newName);
		if(success == Util::SaveAsAnswer::NameAlreadyThere)
		{
			Message::error(tr("Playlist name already exists"));
		}

		else
		{
			showSaveMessageBox(this, success);
		}
	}
}

void GUI_Playlist::tabResetClicked(int playlistIndex)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(playlist)
	{
		playlist->reloadFromDatabase();
	}
}

void GUI_Playlist::tabDeletePlaylistClicked(int playlistIndex)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(playlist)
	{
		playlist->deletePlaylist();
	}
}

void GUI_Playlist::checkTabIcon()
{
	for(auto i = 0; i < ui->twPlaylists->count(); i++)
	{
		const auto height = this->fontMetrics().height();
		ui->twPlaylists->setIconSize(QSize(height, height));
		ui->twPlaylists->setTabIcon(i, QIcon());
	}

	const auto activeIndex = m->playlistHandler->activeIndex();
	auto* playlistView = viewByIndex(ui->twPlaylists, activeIndex);

	if(playlistView &&
	   (m->playManager->playstate() != PlayState::Stopped) &&
	   (playlistView->model()->rowCount() > 0))
	{
		const auto icon = Gui::Icons::icon(Gui::Icons::PlayBorder);
		ui->twPlaylists->tabBar()->setTabIcon(activeIndex, icon);
	}
}

void GUI_Playlist::showClearButtonChanged()
{
	ui->btnClear->setVisible(GetSetting(Set::PL_ShowClearButton));
}

void GUI_Playlist::showBottomBarChanged()
{
	const auto showBottomBar = GetSetting(Set::PL_ShowBottomBar);

	ui->toolButton->setVisible(!showBottomBar);
	ui->bottomBar->setVisible(showBottomBar);
}

void GUI_Playlist::languageChanged()
{
	ui->retranslateUi(this);
	calcTotalTimeLabel(m->playlistHandler->playlist(ui->twPlaylists->currentIndex()), ui->labTotalTime);
}

void GUI_Playlist::skinChanged()
{
	checkTabIcon();
	ui->btnClear->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
}

void GUI_Playlist::dragLeaveEvent(QDragLeaveEvent* event) { event->accept(); }

void GUI_Playlist::dragEnterEvent(QDragEnterEvent* event) { event->accept(); }

void GUI_Playlist::dragMoveEvent(QDragMoveEvent* event) { event->accept(); }

void GUI_Playlist::dropEvent(QDropEvent* event)
{
	auto* view = currentView(ui->twPlaylists);
	if(view)
	{
		view->dropEventFromOutside(event);
	}
}
