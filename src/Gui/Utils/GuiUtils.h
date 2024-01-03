/* GuiUtils.h */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
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

#ifndef GUI_UTILS_H
#define GUI_UTILS_H

class QWidget;
class QPixmap;
class QColor;
class QFontMetrics;
class QImage;
class QString;
class QIcon;
class QPoint;
class QMainWindow;
class QScreen;
class QSize;

template<typename T>
class QList;

#include <QPalette>

namespace Gui
{
	namespace Util
	{
		enum IconTheme
		{
			MintY,
			NoTheme
		};

		QIcon icon(const QString& iconName, IconTheme iconTheme);
		QIcon systemThemeIcon(const QString& iconName);

		QImage image(const QString& iconName, IconTheme themeName);
		QImage image(const QString& iconName, IconTheme themeName, const QSize& size, bool keepAspectRatio = true);

		QPixmap pixmap(const QString& iconName, IconTheme themeName);
		QPixmap pixmap(const QString& iconName, IconTheme themeName, const QSize& size, bool keepAspectRatio = true);

		QScreen* getBiggestScreen();

		QColor color(QPalette::ColorGroup colorGroup, QPalette::ColorRole colorRole);

		void placeInScreenCenter(QWidget* widget, float relativeSizeX, float relativeSizeY);

		int textWidth(const QFontMetrics& fm, const QString& text);
		int textWidth(QWidget* widget, const QString& text);

		int viewRowHeight(const QFontMetrics& fontMetrics);
	}
}

#endif // GUI_UTILS_H
