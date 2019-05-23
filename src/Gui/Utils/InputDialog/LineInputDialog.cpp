/* LineInputDialog.cpp */

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

#include "LineInputDialog.h"
#include <QInputDialog>
#include "Utils/Language.h"

LineInputDialog::LineInputDialog(const QString& title, const QString& label, QWidget* parent) :
	QInputDialog(parent)
{
	setWindowTitle(title);
	setLabelText(label);
	setModal(true);
	setCancelButtonText(Lang::get(Lang::Cancel));
	setOkButtonText(Lang::get(Lang::OK));
	setInputMode(QInputDialog::TextInput);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setMinimumWidth(300);
	this->resize(300, this->height());
}

LineInputDialog::LineInputDialog(const QString& title, const QString& label, const QString& initial_value, QWidget* parent) :
	LineInputDialog(title, label, parent)
{
	setTextValue(initial_value);
}

LineInputDialog::~LineInputDialog() {}

