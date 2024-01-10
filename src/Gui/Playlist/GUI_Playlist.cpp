/* GUI_Playlist.cpp */

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

/*
 *  Created on: Apr 6, 2011
 */

#include "GUI_Playlist.h"
#include "PlaylistTabWidget.h"
#include "PlaylistView.h"
#include "PlaylistActionMenu.h"

#include "Components/DynamicPlayback/DynamicPlaybackChecker.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/PlaylistLibraryInteractor.h"
#include "Components/Playlist/PlaylistModifiers.h"
#include "Gui/Library/Utils/GUI_DeleteDialog.h"
#include "Gui/Playlist/ui_GUI_Playlist.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/M3UParser.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/globals.h"

#include <QTabBar>
#include <QShortcut>

using Playlist::Handler;
using Playlist::LibraryInteractor;
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

			case Util::SaveAsAnswer::InvalidName:
				return Message::warning(parent->tr("The specified name is invalid."),
				                        parent->tr("Save playlist"));

			default:
				return Message::Answer::Undefined;
		}
	}

	void calcTotalTimeLabel(PlaylistPtr playlist, QLabel* labTotalTime)
	{
		const auto durationMs = (playlist) ? ::Playlist::runningTime(*playlist) : MilliSeconds {0};
		const auto rows = (playlist) ? ::Playlist::count(*playlist) : 0;

		auto playlistString = (rows > 0)
		                      ? Lang::getWithNumber(Lang::NrTracks, rows)
		                      : labTotalTime->tr("Playlist empty");

		if(durationMs > 0)
		{
			playlistString += " - " + Util::msToString(durationMs, "$He $M:$S");
		}

		labTotalTime->setText(playlistString);
		labTotalTime->setContentsMargins(0, 2, 0, 2);
	}

	void checkPlaylistMenu(PlaylistPtr playlist, const QPoint& position, TabWidget* tabWidget)
	{
		if(!playlist)
		{
			return;
		}

		using Playlist::MenuEntry;

		const auto saveEnabled = (!playlist->isTemporary());
		const auto saveAsEnabled = true;
		const auto count = ::Playlist::count(*playlist);
		const auto saveToFileEnabled = (count > 0);
		const auto deleteEnabled = (!playlist->isTemporary());
		const auto resetEnabled = (!playlist->isTemporary() && playlist->wasChanged());
		const auto closeEnabled = (tabWidget->count() > 2);
		const auto clearEnabled = (count > 0);
		const auto isLocked = (playlist->isLocked());

		auto entries = Playlist::MenuEntries {MenuEntry::None};

		entries |= (saveEnabled) ? MenuEntry::Save : 0;
		entries |= (saveAsEnabled) ? MenuEntry::SaveAs : 0;
		entries |= (saveToFileEnabled) ? MenuEntry::SaveToFile : 0;
		entries |= (deleteEnabled) ? MenuEntry::Delete : 0;
		entries |= (resetEnabled) ? MenuEntry::Reset : 0;
		entries |= (closeEnabled) ? MenuEntry::Close : 0;
		entries |= (closeEnabled) ? MenuEntry::CloseOthers : 0;
		entries |= (clearEnabled) ? MenuEntry::Clear : 0;
		entries |= MenuEntry::OpenFile;
		entries |= MenuEntry::OpenDir;
		entries |= MenuEntry::Rename;
		entries |= (isLocked) ? MenuEntry::Unlock : MenuEntry::Lock;

		tabWidget->showMenuItems(entries, position);
	}

	void checkPlaylistName(PlaylistPtr playlist, TabWidget* tabWidget)
	{
		static const auto fontMetrics = tabWidget->fontMetrics();
		static const auto tabWidth = Gui::Util::textWidth(fontMetrics, QStringLiteral("This is enough [X]"));

		if(playlist)
		{
			const auto name = (!playlist->isTemporary() && playlist->wasChanged())
			                  ? QString("*%1").arg(playlist->name())
			                  : playlist->name();

			const auto elidedName = fontMetrics.elidedText(name, Qt::ElideRight, tabWidth);
			tabWidget->setTabText(playlist->index(), elidedName);
			tabWidget->setTabToolTip(playlist->index(), name);
		}
	}
} // namespace end

struct GUI_Playlist::Private
{
	Handler* playlistHandler;
	PlayManager* playManager;
	DynamicPlaybackChecker* dynamicPlaybackChecker;
	Library::InfoAccessor* libraryAccessor;

	Private(Handler* playlistHandler, PlayManager* playManager, DynamicPlaybackChecker* dynamicPlaybackChecker,
	        Library::InfoAccessor* libraryAccessor) :
		playlistHandler(playlistHandler),
		playManager(playManager),
		dynamicPlaybackChecker(dynamicPlaybackChecker),
		libraryAccessor(libraryAccessor) {}
};

GUI_Playlist::GUI_Playlist(QWidget* parent) :
	Widget(parent) {}

void
GUI_Playlist::init(Handler* playlistHandler, PlayManager* playManager, DynamicPlaybackChecker* dynamicPlaybackChecker,
                   Shutdown* shutdown, Library::InfoAccessor* libraryAccessor)
{
	m = Pimpl::make<Private>(playlistHandler, playManager, dynamicPlaybackChecker, libraryAccessor);

	ui = std::make_shared<Ui::PlaylistWindow>();
	ui->setupUi(this);
	ui->bottomBar->init(dynamicPlaybackChecker, shutdown);

	setAcceptDrops(true);

	for(auto i = 0; i < m->playlistHandler->count(); i++)
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
	connect(ui->twPlaylists, &TabWidget::tabCloseRequested, this, &GUI_Playlist::playlistCloseRequested);
	connect(ui->twPlaylists, &TabWidget::currentChanged, m->playlistHandler, &Handler::setCurrentIndex);
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
	connect(ui->twPlaylists, &TabWidget::sigContextMenuRequested, this, &GUI_Playlist::contextMenuRequested);
	connect(ui->twPlaylists, &TabWidget::sigLockTriggered, this, &GUI_Playlist::lockTriggered);

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
}

void GUI_Playlist::initToolButton()
{
	auto* menu = new Playlist::ActionMenu(m->dynamicPlaybackChecker, this);
	const auto actions = menu->actions();
	for(auto* action: actions)
	{
		ui->toolButton->registerAction(action);
	}

	ui->toolButton->registerPreferenceAction(new Gui::PlaylistPreferenceAction(this));
}

void GUI_Playlist::clearButtonPressed(int playlistIndex)
{
	if(auto playlist = m->playlistHandler->playlist(playlistIndex); playlist)
	{
		Playlist::clear(*playlist);
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
		if(auto* playlistView = ui->twPlaylists->viewByIndex(originTab); playlistView)
		{
			playlistView->removeSelectedRows();
		}
	}

	if(auto playlist = m->playlistHandler->playlist(playlistIndex); playlist)
	{
		if(originTab == playlistIndex)
		{
			Playlist::insertTracks(*playlist, tracks, 0);
		}

		else
		{
			Playlist::appendTracks(*playlist, tracks);
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

void GUI_Playlist::lockTriggered(int playlistIndex, const bool b)
{
	const auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(b)
	{
		playlist->lock();
	}
	else
	{
		playlist->unlock();
	}

	ui->twPlaylists->checkTabButtons();
}

void GUI_Playlist::playlistNameChanged(int playlistIndex)
{
	if(auto playlist = m->playlistHandler->playlist(playlistIndex); playlist)
	{
		const auto playlistName = playlist->name();
		checkPlaylistName(playlist, ui->twPlaylists);

		for(auto i = ui->twPlaylists->count() - 2; i >= 0; i--)
		{
			const auto tabText = ui->twPlaylists->tabText(i);
			if((i != playlistIndex) && (tabText == playlistName))
			{
				ui->twPlaylists->removeTab(i);
			}
		}

		ui->twPlaylists->checkTabButtons();
	}
}

void GUI_Playlist::playlistChanged(int playlistIndex)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(playlist && (playlistIndex != ui->twPlaylists->count() - 1))
	{
		checkPlaylistName(playlist, ui->twPlaylists);
		calcTotalTimeLabel(playlist, ui->labTotalTime);
	}
}

void GUI_Playlist::playlistIdxChanged(int playlistIndex)
{
	if(Util::between(playlistIndex, ui->twPlaylists->count() - 1))
	{
		ui->twPlaylists->setCurrentIndex(playlistIndex);

		this->setFocusProxy(ui->twPlaylists->currentWidget());

		auto playlist = m->playlistHandler->playlist(playlistIndex);
		calcTotalTimeLabel(playlist, ui->labTotalTime);
	}
}

void GUI_Playlist::playlistAdded(int playlistIndex)
{
	if(auto playlist = m->playlistHandler->playlist(playlistIndex); playlist)
	{
		const auto playlistName = playlist->name();
		auto* view = new View(playlist, m->dynamicPlaybackChecker, m->libraryAccessor, ui->twPlaylists);

		const auto tabIndex = ui->twPlaylists->insertTab(playlistIndex, view, playlistName);

		connect(playlist.get(), &Playlist::Playlist::sigItemsChanged, this, &GUI_Playlist::playlistChanged);

		ui->twPlaylists->setCurrentIndex(tabIndex);
		ui->twPlaylists->checkTabButtons();
	}
}

void GUI_Playlist::playlistCloseRequested(const int playlistIndex)
{
	const auto playlist = m->playlistHandler->playlist(playlistIndex);
	const auto isLocked = playlist->isLocked();
	const auto isSaved = playlist->isTemporary() || !playlist->wasChanged();
	if(GetSetting(Set::PL_ShowConfirmationOnClose) || isLocked || !isSaved)
	{
		const auto answer = Message::question_yn(QString("Do you really want to close %1").arg(playlist->name()));
		if(answer != Message::Answer::Yes)
		{
			return;
		}
	}

	m->playlistHandler->closePlaylist(playlistIndex);
}

void GUI_Playlist::playlistClosed(int playlistIndex)
{
	auto* playlistWidget = ui->twPlaylists->widget(playlistIndex);

	ui->twPlaylists->removeTab(playlistIndex);
	ui->twPlaylists->checkTabButtons();

	if(auto* playlistView = ui->twPlaylists->currentView(); playlistView)
	{
		playlistView->setFocus();
	}

	playlistWidget->deleteLater();
}

void GUI_Playlist::contextMenuRequested(int playlistIndex, const QPoint& position)
{
	if(auto playlist = m->playlistHandler->playlist(playlistIndex); playlist)
	{
		checkPlaylistMenu(playlist, position, ui->twPlaylists);
	}
}

void GUI_Playlist::tabSavePlaylistClicked(int playlistIndex)
{
	if(auto playlist = m->playlistHandler->playlist(playlistIndex); playlist)
	{
		const auto success = playlist->save();
		checkPlaylistName(playlist, ui->twPlaylists);
		showSaveMessageBox(this, success);
	}
}

void GUI_Playlist::tabSavePlaylistAsClicked(int playlistIndex, const QString& newName)
{
	auto playlist = m->playlistHandler->playlist(playlistIndex);
	if(!playlist || newName.isEmpty())
	{
		return;
	}

	const auto success = playlist->saveAs(newName);
	showSaveMessageBox(this, success);
}

void GUI_Playlist::tabSavePlaylistToFileClicked(int playlistIndex, const QString& filename, bool relativePaths)
{
	if(const auto playlist = m->playlistHandler->playlist(playlistIndex); playlist)
	{
		M3UParser::saveM3UPlaylist(filename, playlist->tracks(), relativePaths);
	}
}

void GUI_Playlist::tabRenameClicked(int playlistIndex, const QString& newName)
{
	if(auto playlist = m->playlistHandler->playlist(playlistIndex); playlist)
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
	if(auto playlist = m->playlistHandler->playlist(playlistIndex); playlist)
	{
		Playlist::reloadFromDatabase(*playlist);
	}
}

void GUI_Playlist::tabDeletePlaylistClicked(int playlistIndex)
{
	if(auto playlist = m->playlistHandler->playlist(playlistIndex); playlist)
	{
		playlist->setTemporary(true);
		playlist->save();
	}
}

void GUI_Playlist::checkTabIcon()
{
	const auto playState = m->playManager->playstate();
	const auto activeTab = (playState != PlayState::Stopped)
	                       ? m->playlistHandler->activeIndex()
	                       : -1;

	ui->twPlaylists->setActiveTab(activeTab);
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
	if(auto* view = ui->twPlaylists->currentView(); view)
	{
		view->dropEventFromOutside(event);
	}
}
