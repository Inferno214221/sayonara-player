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
#include "Utils/StandardPaths.h"

#include <QTranslator>
#include <QApplication>
#include <QList>
#include <QDir>

struct Translator::Private
{
	QList<QTranslator*> translators;
};

Translator::Translator()
{
	m = Pimpl::make<Private>();
}

Translator::~Translator() = default;

bool Translator::switchTranslator(QObject* parent, const QString& fourLetter)
{
	if(!m->translators.isEmpty())
	{
		for(QTranslator* t : m->translators)
		{
			QApplication::removeTranslator(t);
			t->deleteLater();
		}

		m->translators.clear();
	}

	const QString languageFile = Util::Language::getUsedLanguageFile(fourLetter);
	const QString languageDir = Util::translationsSharePath();

	QStringList filenames;
	filenames << QDir(languageDir).absoluteFilePath(languageFile)
	          << Util::Language::getCurrentQtTranslationPaths();

	for(const QString& filename : filenames)
	{
		auto* translator = new QTranslator(parent);
		bool loaded = translator->load(filename);
		if(!loaded)
		{
			translator->deleteLater();
			spLog(Log::Debug, this) << "Translator " << filename << " could not be loaded";
			continue;
		}

		if(translator->isEmpty())
		{
			translator->deleteLater();
			spLog(Log::Debug, this) << "Translator is empty";
			continue;
		}

		bool installed = QApplication::installTranslator(translator);
		if(!installed)
		{
			translator->deleteLater();
			spLog(Log::Debug, this) << "Translator " << filename << " could not be installed";
			continue;
		}

		m->translators << translator;
	}

	return (!m->translators.isEmpty());
}

void Translator::changeLanguage(QObject* parent, const QString& fourLetter)
{
	switchTranslator(parent, fourLetter);
}
