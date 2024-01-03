/* ImageButton.h
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#ifndef IMAGEBUTTON_H
#define IMAGEBUTTON_H

#include <QPushButton>
#include "Utils/Pimpl.h"

class QPixmap;

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


	class ImageButton : public QPushButton
	{
		Q_OBJECT
		PIMPL(ImageButton)

		signals:
			void sigPixmapChanged();
			void sigTriggered();

		public:
			explicit ImageButton(QWidget* parent=nullptr);
			~ImageButton() override;

			QPixmap pixmap() const;
			int verticalPadding() const;
			void setFadingEnabled(bool b);

		public slots:
			void showDefaultPixmap();
			void setPixmap(const QPixmap& pm);
			void setPixmapPath(const QString& path);
			void setCoverData(const QByteArray& data, const QString& mimetype);

		private slots:
			void timerTimedOut();
			void byteconverterFinished();

		private:
			using QPushButton::setIcon;
			using QPushButton::icon;

		protected:
			void paintEvent(QPaintEvent* e) override;
			void resizeEvent(QResizeEvent* e) override;
			void mouseMoveEvent(QMouseEvent* e) override;
			void mouseReleaseEvent(QMouseEvent* event) override;
	};
}

#endif // IMAGEBUTTON_H
