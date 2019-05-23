/* TagTextInput.h */

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

#ifndef TAGTEXTINPUT_H
#define TAGTEXTINPUT_H

#include <QLineEdit>
#include "Gui/Utils/Widgets/WidgetTemplate.h"

class QAction;
class QMenu;
class QContextMenuEvent;

/**
 * @brief GUI class for String fields meant to convert content to first upper case by a context menu
 * @ingroup Tagging
 **/
class TagTextInput :
    public Gui::WidgetTemplate<QLineEdit>
{
	Q_OBJECT

public:
	TagTextInput(QWidget* parent=nullptr);
	~TagTextInput();

private slots:
	void cvt_to_first_upper();
	void cvt_to_very_first_upper();

protected:

	QMenu*			_context_menu=nullptr;
	QAction*		_action_cvt_to_first_upper=nullptr;
	QAction*		_action_cvt_to_very_first_upper=nullptr;

protected:

	void contextMenuEvent(QContextMenuEvent* event) override;
	void init_context_menu();
    void language_changed() override;
};

#endif // TAGTEXTINPUT_H
