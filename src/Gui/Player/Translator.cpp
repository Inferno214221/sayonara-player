/* Translator.cpp */

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

#include "Translator.h"

#include "Utils/Utils.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/Logger/Logger.h"

#include <QTranslator>
#include <QApplication>

struct Translator::Private
{
	QString current_language;
	QTranslator* translator=nullptr;
};

Translator::Translator()
{
	 m = Pimpl::make<Private>();
}

Translator::~Translator() = default;

bool Translator::switchTranslator(QObject* parent, const QString& fourLetter, const QString& dir)
{
	QString filename = Util::Language::getUsedLanguageFile(fourLetter);

	if(m->translator)
	{
		QApplication::removeTranslator(m->translator);
		m->translator->deleteLater();
		m->translator = nullptr;
	}

	m->translator = new QTranslator(parent);
	if(!m->translator) {
		return false;
	}

	bool loaded = m->translator->load(filename, dir);
	if(!loaded)
	{
		m->translator->deleteLater();
		m->translator = nullptr;

		spLog(Log::Warning, this) << "Translator " << dir << "/" << filename << " could not be loaded";
		return false;
	}

	if(m->translator->isEmpty())
	{
		m->translator->deleteLater();
		m->translator = nullptr;

		spLog(Log::Warning, this) << "Translator is empty";
		return false;
	}

	bool installed = QApplication::installTranslator(m->translator);
	if(!installed)
	{
		m->translator->deleteLater();
		m->translator = nullptr;

		spLog(Log::Warning, this) << "Translator " << dir << "/" << filename << " could not be installed";
		return false;
	}

	return true;
}

void Translator::changeLanguage(QObject* parent, const QString& language)
{
	m->current_language = language;

	switchTranslator(parent, m->current_language, Util::sharePath("translations/"));
}
