/* GuiUtils.h */

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


/* GuiUtils.h */

#ifndef GuiUtils_H
#define GuiUtils_H

class QWidget;
class QPixmap;
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

namespace Gui
{
	namespace Util
	{

		/**
		 * @brief The IconTheme enum
		 * @ingroup Gui
		 */
		enum IconTheme
		{
			MintY,
			NoTheme
		};

		/**
		 * @brief fetch a icon from resources
		 * @ingroup Gui
		 * @param iconName if iconName ends with png the input string is not modified, else a .svg.png is appended
		 * @return icon
		 */
		QIcon icon(const QString& iconName, IconTheme themeName);


		/**
		 * @brief fetch a pixmap from resources
		 * @ingroup Gui
		 * @param iconName if iconName ends with png the input string is not modified, else a .svg.png is appended
		 * @param theme name use the MintY theme or the standard theme
		 * @param sz target size of pixmap
		 * @param keepAspect if true, aspect ratio is kept
		 * @return pixmap
		 */
		QPixmap pixmap(const QString& iconName, IconTheme themeName);
		QPixmap pixmap(const QString& iconName, IconTheme themeName, QSize sz, bool keepAspect=true);

		/**
		 * @brief see pixmap()
		 * @ingroup Gui
		 * @param iconName
		 * @param themeName
		 * @return
		 */
		QImage image(const QString& iconName, IconTheme themeName);
		QImage image(const QString& iconName, IconTheme themeName, QSize sz, bool keepAspect=true);


		/**
		 * @brief set the applications' main window
		 * @ingroup Gui
		 * @param window the new main window
		 */
		void setMainWindow(QMainWindow* window);

		/**
		 * @brief get the applications' main window
		 * @ingroup Gui
		 * @return main window of application
		 */
		QMainWindow* mainWindow();

		/**
		 * @brief return the screen with biggest screen
		 * @return nullptr on error
		 */
		QScreen* getBiggestScreen();

		/**
		 * @brief Place the widget in the center of the biggest screen
		 * @param widget
		 * @param rel_size_x a percentage value between 0 and 1 regarding width of screen
		 * @param rel_size_y a percentage value between 0 and 1 regarding height of screen
		 */
		void placeInScreenCenter(QWidget* widget, float relativeSizeX, float relativeSizeY);

		int textWidth(const QFontMetrics& fm, const QString& text);
		int textWidth(QWidget* widget, const QString& text);

		int viewRowHeight();
	}
}

#endif // GuiUtils_H
