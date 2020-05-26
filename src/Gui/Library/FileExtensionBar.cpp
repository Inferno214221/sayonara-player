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

#include "FileExtensionBar.h"
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
	QLayout* btnLayout = nullptr;
	QLabel* labFilter = nullptr;
	QPushButton* btnClose = nullptr;

	QMap<QString, QPushButton*> extensionButtonMap;

	AbstractLibrary* library = nullptr;
};

FileExtensionBar::FileExtensionBar(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(10);
	this->setLayout(layout);

	m->labFilter = new QLabel(this);

	QFont font = m->labFilter->font();
	font.setBold(true);
	m->labFilter->setFont(font);
	layout->addWidget(m->labFilter);

	auto* btn_widget = new QWidget();

	m->btnLayout = new QHBoxLayout(btn_widget);
	m->btnLayout->setContentsMargins(0, 0, 0, 0);
	m->btnLayout->setSpacing(10);
	m->btnLayout->setSizeConstraint(QLayout::SetMinimumSize);

	btn_widget->setLayout(m->btnLayout);
	layout->addWidget(btn_widget);

	auto* spacer = new QSpacerItem(10, 10, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
	layout->addSpacerItem(spacer);

	m->btnClose = new QPushButton(this);
	m->btnClose->setFocusPolicy(Qt::NoFocus);
	layout->addWidget(m->btnClose);
	connect(m->btnClose, &QPushButton::clicked, this, &FileExtensionBar::closeClicked);
}

FileExtensionBar::~FileExtensionBar()
{
	for(QPushButton* btn : m->extensionButtonMap)
	{
		m->btnLayout->removeWidget(btn);
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
	const QStringList extensionStrings = extensions.extensions();

	clear();

	bool hasMultipleExtensions = (extensionStrings.size() > 1);
	if(!hasMultipleExtensions)
	{
		return;
	}

	for(const QString& ext : extensionStrings)
	{
		QPushButton* btn;
		if(m->extensionButtonMap.contains(ext))
		{
			btn = m->extensionButtonMap[ext];
		}

		else
		{
			btn = new QPushButton();
			btn->setFocusPolicy(Qt::NoFocus);
			btn->setText(ext);
			btn->setCheckable(true);
			btn->setChecked(extensions.isEnabled(ext));

			connect(btn, &QPushButton::toggled, this, &FileExtensionBar::buttonToggled);

			m->btnLayout->addWidget(btn);
			m->extensionButtonMap[ext] = btn;
		}

		btn->setVisible(true);
	}
}

void FileExtensionBar::clear()
{
	for(QPushButton* btn : m->extensionButtonMap)
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
	auto* btn = dynamic_cast<QPushButton*>(sender());

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
	m->btnClose->setText(Lang::get(Lang::Hide));
	m->labFilter->setText(Lang::get(Lang::Filetype));
}
