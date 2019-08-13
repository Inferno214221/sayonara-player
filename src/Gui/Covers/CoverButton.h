/* CoverButton.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef COVER_BUTTON_H
#define COVER_BUTTON_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"

#include <QPushButton>

class QPixmap;
class QResizeEvent;

namespace Cover
{
	class Location;
}


namespace Gui
{
	/**
	 * @brief The CoverButton class
	 * @ingroup GuiCovers
	 */
	class CoverButton :
			public Gui::WidgetTemplate<QPushButton>
	{
		Q_OBJECT
		PIMPL(CoverButton)

		signals:
			void sig_cover_changed();
			void sig_rejected();

		public:
			explicit CoverButton(QWidget* parent=nullptr);
			virtual ~CoverButton();

			/**
			 * @brief Set an appropriate cover location.
			 * Afterwards a search is triggered to find the cover.
			 * @param cl
			 */
			void set_cover_location(const Cover::Location& cl);

			/**
			 * @brief Force a cover in order to override a searched cover.
			 * This is intended if the audio file contains a cover itself
			 * @param img
			 */
			void force_cover(const QImage& img);

			/**
			 * @brief Force a cover in order to override a searched cover.
			 * This is intended if the audio file contains a cover itself
			 * @param img
			 */
			void force_cover(const QPixmap& img);

			/**
			 * @brief silent results that the cover is not stored
			 * productively. The AlternativeCoverFetcher will
			 * save the cover to a temporary path which can be re-
			 * trieved by Cover::Location::alternative_path()
			 * @param silent
			 */
			void set_silent(bool silent);
			bool is_silent() const;

			QPixmap pixmap() const;

		private:
			using QPushButton::setIcon;
			QIcon current_icon() const;


		protected:
			void mouseMoveEvent(QMouseEvent* e) override;
			void mouseReleaseEvent(QMouseEvent* event) override;
			void paintEvent(QPaintEvent* event) override;
			void resizeEvent(QResizeEvent* e) override;

		private slots:
			void alternative_cover_fetched(const Cover::Location& cl);
			void cover_lookup_finished(bool success);
			void set_cover_image(const QString& path);
			void set_cover_image_pixmap(const QPixmap& pm);
			void covers_changed();
			void timer_timed_out();

		public slots:
			void refresh();
			void trigger();
	};
}

#endif
