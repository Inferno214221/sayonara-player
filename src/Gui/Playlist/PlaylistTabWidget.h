/* PlaylistTabWidget.h */

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

#ifndef PLAYLISTTABWIDGET_H
#define PLAYLISTTABWIDGET_H

#include "PlaylistMenuEntry.h"
#include "Utils/Pimpl.h"

#include <QTabWidget>

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
			void sigTabSaveToFile(int tabIndex, const QString& filename);
			void sigTabRename(int tabIndex, const QString& name);
			void sigTabDelete(int tabIndex);
			void sigTabClear(int tabIndex);
			void sigAddTabClicked();
			void sigMetadataDropped(int tabIndex, const MetaDataList& tracks);
			void sigFilesDropped(int tabIndex, const QStringList& files);

		public:
			explicit TabWidget(QWidget* parent=nullptr);
			~TabWidget() override;

			void showMenuItems(MenuEntries actions);
			void checkTabButtons();

			bool wasDragFromPlaylist() const;
			int getDragOriginTab() const;

			View* viewByIndex(int index);
			View* currentView();

		public slots:
			void setActiveTab(int index);
	};
}

#endif // PLAYLISTTABWIDGET_H
