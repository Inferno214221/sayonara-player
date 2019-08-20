/* RatingLabel.h */

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

#ifndef RATINGLABEL_H
#define RATINGLABEL_H

#include "Utils/Pimpl.h"

#include <QLabel>
#include <QPoint>
#include <QSize>

namespace Gui
{
	/**
	 * @brief The RatingLabel class
	 * @ingroup Gui
	 */
	class RatingLabel :
		public QLabel
	{
		Q_OBJECT
		PIMPL(RatingLabel)

	public:
		RatingLabel(QWidget* parent, bool enabled=true);
		~RatingLabel() override;

		/**
		 * @brief Set a rating from one to 5
		 * @param rating
		 */
		void set_rating(Rating rating);
		Rating rating() const;

		Rating rating_at(QPoint pos) const;

		/**
		 * @brief The y-offset where the stars should be painted
		 * @param offset
		 */
		void set_vertical_offset(int offset);

		void paint(QPainter* painter, const QRect& rect);

		void restore();

		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;
	};


	class RatingEditor : public QWidget
	{
		Q_OBJECT
		PIMPL(RatingEditor)

		signals:
			void sig_finished(bool save);
			void sig_triggered();

		public:
			RatingEditor(QWidget* parent);
			RatingEditor(Rating rating, QWidget* parent);
			~RatingEditor() override;

			void set_rating(Rating rating);
			Rating rating() const;

			void set_vertical_offset(int offset);
			void set_mousetrackable(bool b);

			QSize sizeHint() const override;
			QSize minimumSizeHint() const override;

		protected:
			void paintEvent(QPaintEvent* e) override;

			void focusInEvent(QFocusEvent* e) override;
			void focusOutEvent(QFocusEvent* e) override;

			void mousePressEvent(QMouseEvent* e) override;
			void mouseMoveEvent(QMouseEvent* e) override;
			void mouseReleaseEvent(QMouseEvent* e) override;
	};
}

#endif // RATINGLABEL_H
