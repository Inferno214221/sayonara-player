/* Dialog.cpp */

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

#include "Dialog.h"
#include "Gui/Utils/GuiUtils.h"

#include <QSize>
#include <QScreen>
#include <QDialog>
#include <QCloseEvent>
#include <QMainWindow>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

#include <cmath>

using Gui::Dialog;
using Gui::WidgetTemplate;

Dialog::Dialog(QWidget* parent) :
	WidgetTemplate<QDialog>(parent)
{
    this->setAttribute(Qt::WA_StyledBackground);
    setSizeGripEnabled(true);
}

Dialog::~Dialog() = default;

bool Dialog::isAccepted() const
{
	return (result() == QDialog::Accepted);
}

void Dialog::resizeRelative(QWidget* widget, double percent, const QSize& maxSize)
{
    QSize size;
    size.setWidth(std::max<int>(widget->width() * percent, maxSize.width()));
    size.setHeight(std::max<int>(widget->height() * percent, maxSize.height()));

    this->resize(size);
}

void Dialog::closeEvent(QCloseEvent* e)
{
	QDialog::closeEvent(e);
	emit sigClosed();
}

void Dialog::paintEvent(QPaintEvent *e)
{
    QStyleOption option;
    option.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &option, &p, this);

    QDialog::paintEvent(e);
}
