/* CoverButton.h */

/* Copyright (C) 2011-2017  Lucio Carreras
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

#include <QPushButton>

#include "Utils/Pimpl.h"
#include "GUI/Utils/Widgets/WidgetTemplate.h"

class QPixmap;
class QResizeEvent;

namespace Cover
{
	class Location;
}


/**
 * @brief The CoverButton class
 * @ingroup GUICovers
 */
class CoverButton :
		public Gui::WidgetTemplate<QPushButton>
{
	Q_OBJECT
	PIMPL(CoverButton)

signals:
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


private:
	QIcon current_icon() const;

protected:
	void paintEvent(QPaintEvent* event) override;
	void showEvent(QShowEvent* e) override;

private slots:
	void cover_button_clicked();
	void alternative_cover_fetched(const Cover::Location& cl);
	void cover_found(const Cover::Location& cl);
	void set_cover_image(const QString& cover_path);

public slots:
	void refresh();

};

#endif
