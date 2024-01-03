/* ComboBox.cpp */

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

#include "ComboBox.h"
#include "Gui/Utils/Delegates/ComboBoxDelegate.h"
#include <QAbstractItemView>
#include <QLineEdit>

#include "Gui/Utils/Style.h"

using Gui::ComboBox;

ComboBox::ComboBox(QWidget* parent) :
	Gui::WidgetTemplate<QComboBox>(parent)
{
	this->setItemDelegate(new Gui::ComboBoxDelegate(this));
}

ComboBox::~ComboBox() = default;

void ComboBox::skinChanged()
{
	if(view() && view()->parentWidget())
	{
		const auto stylesheet = (Style::isDark())
		                        ? QStringLiteral("background: #505050; border: none;")
		                        : QString();

		view()->parentWidget()->setStyleSheet(stylesheet);
	}
}