/* GUI_FileExpressionDialog.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "GUI_FileExpressionDialog.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include "Components/Directories/FileOperations.h"

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QSpacerItem>

struct GUI_FileExpressionDialog::Private
{
	QMap<QString, Lang::Term> tagLanguageMapping;
	QLineEdit* leExpression=nullptr;

	QList<QPushButton*> buttons;
	QPushButton* btnCancel=nullptr;
	QPushButton* btnOk=nullptr;

	Private()
	{
		tagLanguageMapping = QMap<QString, Lang::Term>
		{
			{"<title>", Lang::Title},
			{"<album>", Lang::Album},
			{"<artist>", Lang::Artist},
			{"<year>", Lang::Year},
			{"<bitrate>", Lang::Bitrate},
			{"<tracknum>", Lang::TrackNo},
			{"<disc>", Lang::Disc}
		};
	}

	QPushButton* init_button(const QString& value, QWidget* parent)
	{
		auto* btn = new QPushButton(parent);

		if(!tagLanguageMapping.contains(value))
		{
			spLog(Log::Warning, this) << value << " is not allowed";
			return nullptr;
		}

		Lang::Term term = tagLanguageMapping[value];
		btn->setText(Lang::get(term));
		btn->setProperty("value", value);
		btn->setProperty("langterm", int(term));

		buttons << btn;

		return btn;
	}
};

static bool is_valid(const QString& expression)
{
	if(expression.trimmed().isEmpty())
	{
		return false;
	}

	QStringList invalid
	{
		"/", "\\", "?", "*", "{", "}", "[", "]", "#", "\"", "|"
	};

	for(const QString& s : invalid)
	{
		if(expression.contains(s))
		{
			return false;
		}
	}

	QString replaced(expression);
	const QStringList allowed_tags = FileOperations::supportedReplacementTags();
	for(const QString& tag : allowed_tags)
	{
		replaced.replace(tag, "Hallo");
	}

	if(replaced == expression) {
		return false;
	}

	if(replaced.contains("<") || replaced.contains(">")){
		return false;
	}

	return true;
}


GUI_FileExpressionDialog::GUI_FileExpressionDialog(QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>();

	if(this->layout()){
		this->setLayout(nullptr);
	}

	auto* vbox_layout = new QVBoxLayout(this);
	this->setLayout(vbox_layout);

	{ // line edit
		m->leExpression = new QLineEdit(this);
		vbox_layout->addWidget(m->leExpression);
		connect(m->leExpression, &QLineEdit::textChanged, this, &GUI_FileExpressionDialog::textChanged);
	}

	{ // init buttons
		auto* hbox_layout_buttons = new QHBoxLayout();

		const QStringList allowed_tags = FileOperations::supportedReplacementTags();
		for(const QString& tag : allowed_tags)
		{
			m->init_button(tag, this);
		}

		for(auto* btn : m->buttons)
		{
			hbox_layout_buttons->addWidget(btn);
			connect(btn, &QPushButton::clicked, this, &GUI_FileExpressionDialog::buttonClicked);
		}

		vbox_layout->addLayout(hbox_layout_buttons);
	}

	{ // ok cancel
		auto* hbox_layout_okcancel = new QHBoxLayout();
		m->btnOk = new QPushButton(Lang::get(Lang::OK), this);
		m->btnCancel = new QPushButton(Lang::get(Lang::Cancel), this);

		hbox_layout_okcancel->addSpacerItem(
			new QSpacerItem(100, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum)
		);

		hbox_layout_okcancel->addWidget(m->btnCancel);
		hbox_layout_okcancel->addWidget(m->btnOk);

		auto* line = new QFrame(this);
		line->setFrameShape(QFrame::Shape::HLine);

		vbox_layout->addWidget(line);
		vbox_layout->addLayout(hbox_layout_okcancel);

		connect(m->btnOk, &QPushButton::clicked, this, [=]()
		{
			SetSetting(Set::Dir_TagToFilenameExpression, m->leExpression->text());
			this->accept();
		});

		connect(m->btnCancel, &QPushButton::clicked, this, &Gui::Dialog::reject);
	}

	{ // taborder
		this->setTabOrder(m->leExpression, m->buttons.first());
		for(int i=0; i<m->buttons.size() - 1; i++)
		{
			this->setTabOrder(m->buttons[i], m->buttons[i+1]);
		}
		this->setTabOrder(m->buttons.last(), m->btnCancel);
		this->setTabOrder(m->btnCancel, m->btnOk);
		this->setTabOrder(m->btnOk, m->leExpression);
	}

	m->btnOk->setDefault(true);
}

GUI_FileExpressionDialog::~GUI_FileExpressionDialog() = default;

QString GUI_FileExpressionDialog::expression() const
{
	return m->leExpression->text();
}

void GUI_FileExpressionDialog::showEvent(QShowEvent* event)
{
	Gui::Dialog::showEvent(event);

	m->leExpression->setText(GetSetting(Set::Dir_TagToFilenameExpression));

	if(m->leExpression->text().isEmpty())
	{
		QString text = QString("<tracknum>. <title>");
		m->leExpression->setText(text);
	}
}

void GUI_FileExpressionDialog::languageChanged()
{
	for(auto* btn : m->buttons)
	{
		Lang::Term term = Lang::Term(btn->property("langterm").toInt());
		btn->setText(Lang::get(term));
	}
}

void GUI_FileExpressionDialog::buttonClicked()
{
	auto* button = static_cast<QPushButton*>(sender());

	QString text = m->leExpression->text();
	QString button_text = button->property("value").toString();
	int cursor = m->leExpression->cursorPosition();
	if(cursor < text.size() - 1 && cursor >= 0)
	{
		text.insert(cursor, button_text);
		cursor += button_text.size();
	}

	else
	{
		text.append(button_text);
		cursor = text.size();
	}

	m->leExpression->setText(text);
	m->leExpression->setCursorPosition(cursor);
	m->leExpression->setFocus();
}

void GUI_FileExpressionDialog::textChanged(const QString& text)
{
	bool valid = is_valid(text);

	if(!valid) {
		m->leExpression->setStyleSheet("color: red;");
	} else {
		m->leExpression->setStyleSheet("");
	}

	m->btnOk->setEnabled(valid);
}
