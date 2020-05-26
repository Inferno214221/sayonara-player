/* GUI_Lyrics.h */

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

#ifndef GUI_LYRICS_H
#define GUI_LYRICS_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/Widget.h"

UI_FWD(GUI_Lyrics)

/**
 * @brief The GUI_Lyrics class
 * @ingroup InfoDialog
 */
class GUI_Lyrics :
	public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(GUI_Lyrics)
	PIMPL(GUI_Lyrics)

	public:
		explicit GUI_Lyrics(QWidget* parent = nullptr);
		~GUI_Lyrics() override;

		void setTrack(const MetaData& md);

	private:
		void init();

		void zoom(qreal font_size);
		void setupSources();
		void chooseSource();
		void showLyrics(const QString& lyrics, const QString& header, bool rich);
		void showLocalLyrics();
		void setSaveButtonText();

	private slots:
		void zoomIn();
		void zoomOut();

		void lyricsFetched();
		void lyricServerChanged(int idx);

		void switchPressed();
		void prepareLyrics();
		void saveLyricsClicked();

	protected:
		void languageChanged() override;
		void showEvent(QShowEvent* e) override;
		void wheelEvent(QWheelEvent* e) override;
};

#endif // GUI_LYRICS_H
