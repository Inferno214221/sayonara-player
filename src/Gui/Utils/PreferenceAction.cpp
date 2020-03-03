/* PreferenceAction.cpp */

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

#include "PreferenceAction.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Gui/Utils/Icons.h"
#include "Components/Preferences/PreferenceRegistry.h"

#include <QFont>
#include <QPushButton>

using namespace Gui;

struct PreferenceAction::Private
{
	QString identifier;

	Private(const QString& identifier) :
		identifier(identifier)
	{}
};

PreferenceAction::PreferenceAction(const QString& text, const QString& identifier, QWidget* parent) :
	QAction(QString(Lang::get(Lang::Preferences) + ": " + text), parent)
{
	m = Pimpl::make<Private>(identifier);
	this->setIcon(Gui::Icons::icon(Gui::Icons::Preferences));

	connect(this, &QAction::triggered, [=](){
		PreferenceRegistry::instance()->showPreference(this->identifier());
	});

	ListenSettingNoCall(Set::Player_Language, PreferenceAction::language_changed);
}

PreferenceAction::~PreferenceAction() = default;

QString PreferenceAction::label() const
{
	return Lang::get(Lang::Preferences) + ": " + displayName();
}

QPushButton* PreferenceAction::createButton(QWidget* parent)
{
	auto* btn = new QPushButton(parent);
	btn->setObjectName("PreferenceButton");
	btn->setText(this->label());
	btn->addAction(this);
	connect(btn, &QPushButton::clicked, this, &QAction::triggered);

	return btn;
}

QString PreferenceAction::identifier() const
{
	return m->identifier;
}

void PreferenceAction::language_changed()
{
	this->setText(this->label());
}

LibraryPreferenceAction::LibraryPreferenceAction(QWidget* parent) :
	PreferenceAction(Lang::get(Lang::Library), identifier(), parent)
{}

LibraryPreferenceAction::~LibraryPreferenceAction() = default;

QString LibraryPreferenceAction::displayName() const
{
	return Lang::get(Lang::Library);
}

QString LibraryPreferenceAction::identifier() const
{
	return "library";
}

PlaylistPreferenceAction::PlaylistPreferenceAction(QWidget* parent) :
	PreferenceAction(Lang::get(Lang::Playlist), identifier(), parent)
{}

PlaylistPreferenceAction::~PlaylistPreferenceAction() = default;


QString PlaylistPreferenceAction::displayName() const
{
	return Lang::get(Lang::Playlist);
}

QString PlaylistPreferenceAction::identifier() const
{
	return "playlist";
}

SearchPreferenceAction::SearchPreferenceAction(QWidget* parent) :
	PreferenceAction(Lang::get(Lang::SearchNoun), identifier(), parent)
{}

SearchPreferenceAction::~SearchPreferenceAction() = default;

QString SearchPreferenceAction::displayName() const
{
	return Lang::get(Lang::SearchNoun);
}

QString SearchPreferenceAction::identifier() const
{
	return "search";
}

CoverPreferenceAction::CoverPreferenceAction(QWidget* parent) :
	PreferenceAction(Lang::get(Lang::Covers), identifier(), parent)
{}

CoverPreferenceAction::~CoverPreferenceAction() = default;

QString CoverPreferenceAction::displayName() const
{
	return Lang::get(Lang::Covers);
}

QString CoverPreferenceAction::identifier() const
{
	return "covers";
}

PlayerPreferencesAction::PlayerPreferencesAction(QWidget* parent) :
	PreferenceAction(Lang::get(Lang::Application), identifier(), parent)
{}

PlayerPreferencesAction::~PlayerPreferencesAction() = default;

QString PlayerPreferencesAction::displayName() const
{
	return Lang::get(Lang::Application);
}

QString PlayerPreferencesAction::identifier() const
{
	return "application";
}

StreamRecorderPreferenceAction::StreamRecorderPreferenceAction(QWidget* parent) :
	PreferenceAction(tr("Stream Recorder"), identifier(), parent)
{}

StreamRecorderPreferenceAction::~StreamRecorderPreferenceAction() = default;

QString StreamRecorderPreferenceAction::displayName() const
{
	return tr("Stream Recorder");
}

QString StreamRecorderPreferenceAction::identifier() const
{
	return "streamrecorder";
}


ShortcutPreferenceAction::ShortcutPreferenceAction(QWidget* parent) :
	PreferenceAction(tr("Shortcuts"), identifier(), parent)
{}

ShortcutPreferenceAction::~ShortcutPreferenceAction() = default;

QString ShortcutPreferenceAction::identifier() const
{
	return "shortcuts";
}

QString ShortcutPreferenceAction::displayName() const
{
	return tr("Shortcuts");
}
