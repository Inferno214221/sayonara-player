/* GUI_Simpleplayer.h */

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

#ifndef GUI_SIMPLEPLAYER_H
#define GUI_SIMPLEPLAYER_H

#include "Gui/Utils/GuiClass.h"
#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Message/MessageReceiverInterface.h"
#include "Utils/Pimpl.h"

#include <QSystemTrayIcon>

class GUI_TrayIcon;
class GUI_Logger;
class QAction;
class QMessageBox;
class QTranslator;

UI_FWD(GUI_Player)

class CoverDataProvider;
class DynamicPlaybackChecker;
class LibraryInfoAccessor;

namespace PlayerPlugin
{
	class Base;
}

namespace Playlist
{
	class Handler;
}

class PlayManager;

class GUI_Player :
	public Gui::MainWindow,
	public MessageReceiverInterface
{
	Q_OBJECT
	PIMPL(GUI_Player)
	UI_CLASS_SHARED_PTR(GUI_Player)

	public:
		GUI_Player(PlayManager* playManager, Playlist::Handler* playlistHandler, CoverDataProvider* coverProvider,
		           DynamicPlaybackChecker* dynamicPlaybackChecker, LibraryInfoAccessor* libraryAccessor,
		           QWidget* parent);
		~GUI_Player() override;

		void registerPreferenceDialog(QAction* dialog_action);
		void shutdown() override;

	protected:
		void closeEvent(QCloseEvent* e) override;
		void resizeEvent(QResizeEvent* e) override;
		bool event(QEvent* e) override;

		// NOLINTNEXTLINE(google-default-arguments)
		Message::Answer errorReceived(const QString& error, const QString& senderName = QString()) override;
		// NOLINTNEXTLINE(google-default-arguments)
		Message::Answer warningReceived(const QString& error, const QString& senderName = QString()) override;
		// NOLINTNEXTLINE(google-default-arguments)
		Message::Answer infoReceived(const QString& error, const QString& senderName = QString()) override;
		// NOLINTNEXTLINE(google-default-arguments)
		Message::Answer questionReceived(const QString& info, const QString& senderName = QString(),
		                                 Message::QuestionType type = Message::QuestionType::YesNo) override;

		void languageChanged() override;

	private slots:
		void playError(const QString& message);

		void splitterMainMoved(int pos, int idx);
		void splitterControlsMoved(int pos, int idx);

		void currentLibraryChanged();

		void minimize();

		void trayIconActivated(QSystemTrayIcon::ActivationReason reason);

		void pluginAdded(PlayerPlugin::Base* plugin);
		void pluginActionTriggered(bool b);

	private: // NOLINT(readability-redundant-access-specifiers)
		void initLanguage();
		void initTrayActions();
		void initConnections();
		void initLibrary();
		void initControlSplitter();
		void initMainSplitter();
		void initFontChangeFix();
		void initGeometry();

		void checkControlSplitter();

		void fullscreenChanged();
		void initControls();
		void controlstyleChanged();

		void showLibraryChanged();
		void addCurrentLibrary();
		void removeCurrentLibrary();

};

#endif // GUI_SIMPLEPLAYER_H
