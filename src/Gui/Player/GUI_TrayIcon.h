/* GUI_TrayIcon.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)  gleugner
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

#ifndef GUI_TRAYICON_H
#define GUI_TRAYICON_H

#include "Interfaces/Notification/NotificationHandler.h"
#include "Components/PlayManager/PlayState.h"

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

#include <QSystemTrayIcon>
#include <QMenu>

class GUI_TrayIcon;
class QTimer;

class TrayIconContextMenu :
		public Gui::WidgetTemplate<QMenu>
{
	friend class GUI_TrayIcon;

	Q_OBJECT
	PIMPL(TrayIconContextMenu)

	signals:
		void sigShowClicked();
		void sigCloseClicked();

	private:
		// all here called by GUI_TrayIcon
		explicit TrayIconContextMenu(QWidget* parent=nullptr);
		~TrayIconContextMenu() override;

		void setForwardEnabled(bool b);
		void setDisplayNames();

	private slots:
		void playstateChanged(PlayState state);
		void muteChanged(bool muted);

		void muteClicked();
		void currentSongClicked();

	protected:
		void languageChanged() override;
		void skinChanged() override;
};


/**
  * Small class to be used as tray icon
  */
class GUI_TrayIcon :
		public QSystemTrayIcon,
		public NotificationInterface
{
	Q_OBJECT
	PIMPL(GUI_TrayIcon)

	signals:
		/**
		  * this event is fired, if we have a mouse wheel event
		  * @param delta bigger then 0 when mouse wheel has moved forward smaller when moved backwards
		  */
		void sigWheelChanged(int delta);
		void sigHideClicked();
		void sigCloseClicked();
		void sigShowClicked();

	public:
		explicit GUI_TrayIcon(QObject* parent=nullptr);
		~GUI_TrayIcon() override;

		bool event(QEvent* e) override;
		[[maybe_unused]] void setForwardEnabled(bool b);

		void notify(const MetaData& md) override;
		void notify(const QString& title, const QString& message, const QString& image_path) override;

		QString name() const override;
		QString displayName() const override;

	private:
		void initContextMenu();

	private slots:
		void playstateChanged(PlayState state);
		void showTrayIconChanged();

	protected:
		void languageChanged();
};

#endif
