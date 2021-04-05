/* BottomBarButton.cpp */

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

#include "PlaylistBottomBarButton.h"
#include <QPixmap>
#include <QPainter>
#include <QPaintEvent>

using Playlist::BottomBarButton;

struct BottomBarButton::Private
{
	QIcon icon;

	Private(const QIcon& icon) : icon(icon) {}
};

BottomBarButton::BottomBarButton(const QIcon& icon, QWidget* parent) :
	QPushButton(parent)
{
	m = Pimpl::make<Private>(icon);
}

BottomBarButton::~BottomBarButton() = default;

void BottomBarButton::setIcon(const QIcon& icon)
{
	m->icon = icon;
}

void BottomBarButton::paintEvent(QPaintEvent* e)
{
	if(!this->isChecked())
	{
		QPushButton::paintEvent(e);
	}

    if (!m->icon.isNull())
    {
		const int w = this->width();
		const int h = this->height();

		int pm_w = (w * 800) / 1000;
		int pm_h = (h * 800) / 1000;

		const int x = (w - pm_w) / 2;
		const int y = (h - pm_h) / 2;

		if((w - pm_w) % 2 == 1){
			pm_w++;
		}

		if((h - pm_h) % 2 == 1){
			pm_h++;
		}

		QPixmap pm = m->icon.pixmap(pm_w, pm_h);;

        QPainter painter(this);
        if(this->isChecked())
        {
			QRect r = e->rect();
			painter.setPen(palette().window().color());
			painter.drawRect(r);

			QColor c = palette().highlight().color();
			painter.setOpacity(0.3);
			painter.setPen(c);
			painter.setBrush(c);
			painter.drawRect(r);
			painter.fillRect(r, c);
        }

		painter.setOpacity(1.0);
        painter.drawPixmap
        (
			QRect(x, y, pm_w, pm_h),
			pm
        );
    }
}
