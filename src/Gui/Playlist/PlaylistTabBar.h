/* PlaylistTabBar.h */

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

#ifndef PLAYLISTTABBAR_H
#define PLAYLISTTABBAR_H

#include "PlaylistMenuEntry.h"
#include "Utils/Pimpl.h"

#include <QTabBar>

namespace Playlist
{
	/**
	 * @brief The PlaylistTabBar class
	 * @ingroup GuiPlaylists
	 */
	class TabBar :
			public QTabBar
	{
		Q_OBJECT
		PIMPL(TabBar)

		signals:
			void sigOpenFile(int tab_idx);
			void sigOpenDir(int tab_idx);

			void sigTabReset(int tab_idx);
			void sigTabSave(int tab_idx);
			void sigTabSaveAs(int tab_idx, const QString& name);
			void sigTabSaveToFile(int tab_idx, const QString& filename);
			void sigTabRename(int tab_idx, const QString& name);
			void sigTabClear(int tab_idx);

			void sigTabDelete(int tab_idx);
			void sigCurrentIndexChanged(int tab_idx);
			void sigAddTabClicked();
			void sigMetadataDropped(int tab_idx, const MetaDataList& v_md);
			void sigFilesDropped(int tab_idx, const QStringList& files);


		public:
			explicit TabBar(QWidget* parent=nullptr);
			~TabBar() override;

			void showMenuItems(MenuEntries entries);
			void setTabsClosable(bool b);

			bool wasDragFromPlaylist() const;
			int getDragOriginTab() const;

		private:
			void initShortcuts();

		private slots:
			void openFilePressed();
			void openDirPressed();
			void resetPressed();
			void savePressed();
			void saveAsPressed();
			void saveToFilePressed();
			void clearPressed();
			void deletePressed();
			void closePressed();
			void closeOthersPressed();
			void renamePressed();

		protected:
			void mousePressEvent(QMouseEvent* e) override;
			void wheelEvent(QWheelEvent* e) override;
			void dragEnterEvent(QDragEnterEvent* e) override;
			void dragMoveEvent(QDragMoveEvent* e) override;
			void dragLeaveEvent(QDragLeaveEvent* e) override;
			void dropEvent(QDropEvent* e) override;
	};
}

#endif // PLAYLISTTABBAR_H
