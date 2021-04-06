/* TagTextInput.cpp */

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

#include "Utils/Utils.h"
#include "LineEdit.h"

#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QCursor>

using Gui::LineEdit;

struct LineEdit::Private
{
	QMenu* contextMenu = nullptr;
	QAction* removeSpecialChars = nullptr;

	QStringList items;

	QList<QAction*> actions;
	int currentIndex;
};

LineEdit::LineEdit(QWidget* parent) :
	WidgetTemplate<QLineEdit>(parent)
{
	m = Pimpl::make<Private>();

	initContextMenu();

	connect(this, &QLineEdit::textChanged, this, &LineEdit::itemTextChanged);
}

LineEdit::~LineEdit() = default;

void LineEdit::initContextMenu()
{
	m->contextMenu = createStandardContextMenu();
	m->contextMenu->addSeparator();

	m->removeSpecialChars = new QAction(m->contextMenu);
	connect(m->removeSpecialChars, &QAction::triggered, this, &LineEdit::removeSpecialCharsTriggered);

	m->contextMenu->addAction(m->removeSpecialChars);
	m->contextMenu->addSeparator();
}

void LineEdit::itemTextChanged(const QString& text)
{
	m->currentIndex = m->items.indexOf(text);
	if(m->currentIndex >= 0)
	{
		return;
	}

	m->items.clear();

	m->items << text.toLower();
	m->items << Util::stringToVeryFirstUpper(text);
	m->items << text;
	m->items << Util::stringToFirstUpper(text);
	m->items << text.toUpper();

	m->items.removeDuplicates();
	m->currentIndex = m->items.indexOf(text);

	for(auto* action : m->actions)
	{
		m->contextMenu->removeAction(action);
	}

	m->actions.clear();

	for(const auto& item : m->items)
	{
		auto* action = new QAction(item, m->contextMenu);
		connect(action, &QAction::triggered, this, &LineEdit::itemActionTriggered);

		m->actions << action;
	}

	m->contextMenu->addActions(m->actions);
}

void LineEdit::itemActionTriggered()
{
	auto* action = static_cast<QAction*>(sender());
	this->setText(action->text());
}

void LineEdit::removeSpecialCharsTriggered()
{
	static const auto specialChars = QStringLiteral("-_+<>=*+~#%.:;#");
	static const auto doubleSpace = QStringLiteral("  ");
	static const auto singleSpace = QStringLiteral(" ");

	QString newText;
	const auto text = this->text();

	for(const auto& c : text)
	{
		newText.append((!specialChars.contains(c)) ? c : ' ');
	}

	while(newText.contains(doubleSpace))
	{
		newText.replace(doubleSpace, singleSpace);
	}

	this->setText(newText);
}

void LineEdit::languageChanged()
{
	this->setToolTip(tr("Hint: Use up and down arrow keys for switching between upper and lower case letters"));
	m->removeSpecialChars->setText(tr("Remove special characters (letters)"));
}

void LineEdit::keyPressEvent(QKeyEvent* event)
{
	WidgetTemplate<QLineEdit>::keyPressEvent(event);

	if(event->key() == Qt::Key_Up)
	{
		m->currentIndex = (m->currentIndex + 1) % m->items.size();
		this->setText(m->items[m->currentIndex]);
	}

	else if(event->key() == Qt::Key_Down)
	{
		m->currentIndex = (m->currentIndex == 0)
			? m->items.size() - 1
			: m->currentIndex - 1;

		this->setText(m->items[m->currentIndex]);
	}
}

void LineEdit::contextMenuEvent([[maybe_unused]] QContextMenuEvent* event)
{
	m->contextMenu->exec(QCursor::pos());
}

