/* PluginCloseButton.h */

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


/* PluginCloseButton.h */

#ifndef PLUGINCLOSEBUTTON_H
#define PLUGINCLOSEBUTTON_H

#include <QPushButton>

#include "Gui/Utils/Widgets/WidgetTemplate.h"

/**
 * @brief The MenuButton class.\n
 * A button that sends a signal when clicked. \n
 * This Class is meant for inheritance like MenuToolButton does.
 * @ingroup PlayerPlugins
 */
class PluginCloseButton :
		public Gui::WidgetTemplate<QPushButton>
{
	Q_OBJECT

signals:
	void sig_triggered(QPoint);

protected:
	virtual void mouseReleaseEvent(QMouseEvent *e) override;
	virtual void enterEvent(QEvent* e) override;
	virtual void leaveEvent(QEvent* e) override;

	/**
	 * @brief sets the icon depending on the skin
	 */
	void set_std_icon();

public:
	explicit PluginCloseButton(QWidget* parent=nullptr);
	virtual ~PluginCloseButton();

protected:
	void skin_changed() override;
};

#endif // PLUGINCLOSEBUTTON_H
