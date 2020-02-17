/* CoverButton.h */

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

	class ByteArrayConverter :
		public QObject
	{
		Q_OBJECT
		PIMPL(ByteArrayConverter)

		signals:
			void sigFinished();

		public:
			ByteArrayConverter(const QByteArray& data, const QString& mime);
			~ByteArrayConverter();

			QPixmap pixmap() const;

		public slots:
			void start();
	};

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
			void sigCoverChanged();
			void sigRejected();

		public:
			explicit CoverButton(QWidget* parent=nullptr);
			~CoverButton() override;

			/**
			 * @brief Set an appropriate cover location.
			 * Afterwards a search is triggered to find the cover.
			 * @param cl
			 */
			void setCoverLocation(const Cover::Location& cl);

			/**
			 * @brief Sets the raw data parsed out of the audio file
			 * @param data raw data
			 * @param mimetype jpg, png or something similar
			 */
			void setCoverData(const QByteArray& data, const QString& mimetype);


			/**
			 * @brief silent results that the cover is not stored
			 * productively. The AlternativeCoverFetcher will
			 * save the cover to a temporary path which can be re-
			 * trieved by Cover::Location::alternative_path()
			 * @param silent
			 */
			void setSilent(bool silent);
			bool isSilent() const;

			QPixmap pixmap() const;
			int verticalPadding() const;

			void setAlternativeSearchEnabled(bool b);
			bool isAlternativeSearchEnabled() const;

		public slots:
			void trigger();

		private:
			using QPushButton::setIcon;
			using QPushButton::icon;


		protected:
			void mouseMoveEvent(QMouseEvent* e) override;
			void mouseReleaseEvent(QMouseEvent* event) override;
			void paintEvent(QPaintEvent* event) override;
			void resizeEvent(QResizeEvent* e) override;

		private slots:
			void alternativeCoverFetched(const Cover::Location& cl);
			void coverLookupFinished(bool success);
			void setCoverImage(const QString& path);
			void setCoverImagePixmap(const QPixmap& pm);
			void coversChanged();
			void timerTimedOut();
			void byteconverterFinished();
	};
}

#endif
