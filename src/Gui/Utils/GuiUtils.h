/* GuiUtils.h */

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


/* GuiUtils.h */

#ifndef GuiUtils_H
#define GuiUtils_H

class QWidget;
class QPixmap;
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

	/**
	 * @ingroup GUI
	 * @ingroup GUIHelper
	 */
	namespace Util
	{
		enum IconTheme
		{
			Breeze,
			MintY,
			NoTheme
		};

		/**
		 * @brief fetch a icon from resources
		 * @param icon_name if icon_name ends with png the input string is not modified, else a .svg.png is appended
		 * @return icon
		 */
		QIcon icon(const QString& icon_name, IconTheme theme_name);


		/**
		 * @brief fetch a pixmap from resources
		 * @param icon_name if icon_name ends with png the input string is not modified, else a .svg.png is appended
		 * @param sz target size of pixmap
		 * @param keep_aspect if true, aspect ratio is kept
		 * @return pixmap
		 */
		QPixmap pixmap(const QString& icon_name, IconTheme theme_name);
		QPixmap pixmap(const QString& icon_name, IconTheme theme_name, QSize sz, bool keep_aspect=true);

		QImage image(const QString& icon_name, IconTheme theme_name);
		QImage image(const QString& icon_name, IconTheme theme_name, QSize sz, bool keep_aspect=true);


		/**
		 * @brief set the applications' main window
		 * @param window the new main window
		 */
		void set_main_window(QMainWindow* window);

		/**
		 * @brief get the applications' main window
		 * @return main window of application
		 */
		QMainWindow* main_window();


		QString elide_text(const QString &text, QWidget *widget, int max_lines);
	}
}

#endif // GuiUtils_H
