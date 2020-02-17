/* LibraryFileExtensionBar.cpp */

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

#include "LibraryFileExtensionBar.h"
#include "Components/Library/AbstractLibrary.h"

#include "Utils/Language/Language.h"
#include "Utils/ExtensionSet.h"
#include "Utils/Settings/Settings.h"

#include <QList>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

using Library::FileExtensionBar;

struct FileExtensionBar::Private
{
	QLayout*				btn_layout=nullptr;
	QLabel*					lab_filter=nullptr;
	QPushButton*			btn_close=nullptr;

	QMap<QString, QPushButton*> extension_button_map;

	AbstractLibrary*		library=nullptr;
};


FileExtensionBar::FileExtensionBar(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(10);
	this->setLayout(layout);

	m->lab_filter = new QLabel(this);

	QFont font = m->lab_filter->font();
	font.setBold(true);
	m->lab_filter->setFont(font);
	layout->addWidget(m->lab_filter);


	auto* btn_widget = new QWidget();

	m->btn_layout = new QHBoxLayout(btn_widget);
	m->btn_layout->setContentsMargins(0, 0, 0, 0);
	m->btn_layout->setSpacing(10);
	m->btn_layout->setSizeConstraint(QLayout::SetMinimumSize);

	btn_widget->setLayout(m->btn_layout);
	layout->addWidget(btn_widget);

	auto* spacer = new QSpacerItem(10, 10, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
	layout->addSpacerItem(spacer);

	m->btn_close = new QPushButton(this);
	layout->addWidget(m->btn_close);
	connect(m->btn_close, &QPushButton::clicked, this, &FileExtensionBar::closeClicked);
}

FileExtensionBar::~FileExtensionBar()
{
	for(QPushButton* btn : m->extension_button_map)
	{
		m->btn_layout->removeWidget(btn);
		btn->setParent(nullptr);
		btn->deleteLater();
	}
}

void FileExtensionBar::init(AbstractLibrary* library)
{
	m->library = library;
}

void FileExtensionBar::refresh()
{
	Gui::ExtensionSet extensions = m->library->extensions();
	const QStringList extension_strings = extensions.extensions();

	clear();

	bool has_multiple_extensions = (extension_strings.size() > 1);
	if(!has_multiple_extensions){
		return;
	}

	for(const QString& ext : extension_strings)
	{
		QPushButton* btn = nullptr;
		if(m->extension_button_map.contains(ext))
		{
			btn = m->extension_button_map[ext];
		}

		else
		{
			btn = new QPushButton();
			btn->setText(ext);
			btn->setCheckable(true);
			btn->setChecked(extensions.isEnabled(ext));

			connect(btn, &QPushButton::toggled, this, &FileExtensionBar::buttonToggled);

			m->btn_layout->addWidget(btn);
			m->extension_button_map[ext] = btn;
		}

		btn->setVisible(true);
	}
}

void FileExtensionBar::clear()
{
	for(QPushButton* btn : m->extension_button_map)
	{
		btn->setVisible(false);
	}
}

bool FileExtensionBar::hasExtensions() const
{
	return (m->library->extensions().extensions().size() > 1);
}


void FileExtensionBar::buttonToggled(bool b)
{
	auto* btn = static_cast<QPushButton*>(sender());

	Gui::ExtensionSet extensions = m->library->extensions();
	extensions.setEnabled(btn->text(), b);

	m->library->setExtensions(extensions);
}

void FileExtensionBar::closeClicked()
{
	SetSetting(Set::Lib_ShowFilterExtBar, false);
}

void FileExtensionBar::languageChanged()
{
	m->btn_close->setText(Lang::get(Lang::Hide));
	m->lab_filter->setText(Lang::get(Lang::Filetype));
}
