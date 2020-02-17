
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

#ifndef PLAYLISTBOTTOMBAR_H
#define PLAYLISTBOTTOMBAR_H

#include "Gui/Utils/Widgets/Widget.h"

#include "Components/Shutdown/Shutdown.h"
#include "Utils/Pimpl.h"
#include "Utils/Macros.h"

#ifdef SAYONARA_WITH_SHUTDOWN
	class GUI_Shutdown;
#endif

namespace Playlist
{
	class Mode;

	/**
	 * @brief The GUI_PlaylistBottomBar class
	 * @ingroup GuiPlaylists
	 */
	class BottomBar :
			public Gui::Widget
	{
		Q_OBJECT
		PIMPL(BottomBar)

		signals:
			void sigShowNumbersChanged(bool active);
			void sigPlaylistModeChanged(const ::Playlist::Mode& mode);

		public:
			explicit BottomBar(QWidget* parent=nullptr);
			~BottomBar() override;

			void checkDynamicPlayButton();

		private slots:
			void rep1Checked(bool checked);
			void repAllChecked(bool checked);
			void shuffleChecked(bool checked);
			void gaplessClicked();

			void changePlaylistMode();
			void playlistModeSettingChanged();

		#ifdef SAYONARA_WITH_SHUTDOWN
			void shutdown_clicked();
			void shutdown_started(MilliSeconds time2go);
			void shutdown_closed();
		#endif

		protected:
			void languageChanged() override;
			void skinChanged() override;
			void showEvent(QShowEvent* e) override;
			void resizeEvent(QResizeEvent* e) override;
	};
}

#endif // PLAYLISTBOTTOMBAR_H
