/* RatingLabel.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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
	 * @brief A simple label, not suitable for editing.
	 * For editing, use the RatingEditor class. RatingLabel
	 * is intended for the paint method in delegates
	 * @ingroup Widgets
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

		/**
		 * @brief Returns the rating regarding the current mouse position
		 * @param pos
		 * @return
		 */
		Rating rating_at(QPoint pos) const;

		/**
		 * @brief The y-offset where the stars should be painted
		 * @param offset
		 */
		void set_vertical_offset(int offset);

		/**
		 * @brief Called from outside.
		 * Mostly from delegates or from the RatingEditor class
		 * @param painter
		 * @param rect
		 */
		void paint(QPainter* painter, const QRect& rect);

		/**
		 * @brief about 20px in height and 5x20px in width
		 * @return
		 */
		QSize sizeHint() const override;

		/**
		 * @brief Same as sizeHint
		 * @return
		 */
		QSize minimumSizeHint() const override;
	};

	/**
	 * @brief This class is used for the actual editing of a RatingLabel
	 * While the RatingLabel class is used in paint() methods of delegates,
	 * this class is used in normal widgets or for createEditor() methods
	 * in delegates
	 * @ingroup Widgets
	 */
	class RatingEditor : public QWidget
	{
		Q_OBJECT
		PIMPL(RatingEditor)

		signals:
			void sig_finished(bool save);

		public:
			RatingEditor(QWidget* parent);
			RatingEditor(Rating rating, QWidget* parent);
			~RatingEditor() override;

			/**
			 * @brief Sets the actual rating
			 * @param rating
			 */
			void set_rating(Rating rating);

			/**
			 * @brief Returns the actual rating. This
			 * is not neccessarily the rating currently visible.
			 * Consider the case where you hover over the stars
			 * and lose focus because you are clicking into another
			 * widget. You don't want the currently shown value
			 * then, you want the old value back. This value is
			 * updated when a mouseReleaseEvent is fired when
			 * clicking on a star in the current widget
			 * @return
			 */
			Rating rating() const;

			/**
			 * @brief Set an offset where to begin drawing stars
			 * @param offset
			 */
			void set_vertical_offset(int offset);

			/**
			 * @brief Enable mouse move events. If disabled, there's
			 * no live update
			 * @param b
			 */
			void set_mousetrackable(bool b);

			/**
			 * @brief Same as RatingLabel::sizeHint
			 * @return
			 */
			QSize sizeHint() const override;

			/**
			 * @brief Same as RatingLabel::minimumSizeHint
			 * @return
			 */
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
