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

#include "Components/Notification/NotificationHandler.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"

#include <QSystemTrayIcon>
#include <QMenu>

class GUI_TrayIcon;
class PlayManager;
class QTimer;

class TrayIconContextMenu :
	public Gui::WidgetTemplate<QMenu>
{
	Q_OBJECT
	PIMPL(TrayIconContextMenu)

	signals:
		void sigShowClicked();
		void sigCloseClicked();

	public:
		TrayIconContextMenu(PlayManager* playManager, GUI_TrayIcon* parent);
		~TrayIconContextMenu() override;

		void setForwardEnabled(bool b);
		void setDisplayNames();

	protected:
		void languageChanged() override;
		void skinChanged() override;

	private slots:
		void playstateChanged(PlayState state);
		void muteChanged(bool muted);

		void muteClicked();
		void currentSongClicked();
};

class GUI_TrayIcon :
	public QSystemTrayIcon,
	public NotificationInterface
{
	Q_OBJECT
	PIMPL(GUI_TrayIcon)

	signals:
		void sigWheelChanged(int delta);
		void sigCloseClicked();
		void sigShowClicked();

	public:
		explicit GUI_TrayIcon(PlayManager* playManager, QObject* parent = nullptr);
		~GUI_TrayIcon() override;

		bool event(QEvent* e) override;
		[[maybe_unused]] void setForwardEnabled(bool b);

		void notify(const MetaData& md) override;
		void notify(const QString& title, const QString& message, const QString& imagePath) override;

		[[nodiscard]] QString name() const override;
		[[nodiscard]] QString displayName() const override;

	private slots:
		void playstateChanged(PlayState state);
		void showTrayIconChanged();

	private: // NOLINT(readability-redundant-access-specifiers)
		void initContextMenu();
};

#endif
