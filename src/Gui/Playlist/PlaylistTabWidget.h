/* PlaylistTabWidget.h */

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

#ifndef PLAYLISTTABWIDGET_H
#define PLAYLISTTABWIDGET_H

#include "PlaylistMenuEntry.h"
#include "Utils/Pimpl.h"

#include <QTabWidget>

class QPoint;
namespace Playlist
{
	class View;
	class TabWidget :
		public QTabWidget
	{
		Q_OBJECT
		PIMPL(TabWidget)

		signals:
			void sigOpenFile(int tabIndex, const QStringList& files);
			void sigOpenDir(int tabIndex, const QString& dir);
			void sigTabReset(int tabIndex);
			void sigTabSave(int tabIndex);
			void sigTabSaveAs(int tabIndex, const QString& name);
			void sigTabSaveToFile(int tabIndex, const QString& filename, bool relativePaths);
			void sigTabRename(int tabIndex, const QString& name);
			void sigTabDelete(int tabIndex);
			void sigTabClear(int tabIndex);
			void sigAddTabClicked();
			void sigMetadataDropped(int tabIndex, const MetaDataList& tracks);
			void sigFilesDropped(int tabIndex, const QStringList& files);
			void sigContextMenuRequested(int tabIndex, const QPoint& point);
			void sigLockTriggered(int tabIndex, bool lock);

		public:
			explicit TabWidget(QWidget* parent = nullptr);
			~TabWidget() override;

			void checkTabButtons();

			[[nodiscard]] bool wasDragFromPlaylist() const;
			[[nodiscard]] int getDragOriginTab() const;

			void
			checkTabText(int tabIndex, int activeIndex, const QString& playlistName, bool hasChanges, bool isLocked);
			void showMenu(const QPoint& position, bool isTemporary, bool hasChanges, bool isLocked, int trackCount);

		private:
			void showMenuItems(MenuEntries actions, const QPoint& position);

	};
}

#endif // PLAYLISTTABWIDGET_H
