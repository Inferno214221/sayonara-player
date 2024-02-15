/* Translator.cpp */

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

#include "Translator.h"

#include "Utils/Utils.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/StandardPaths.h"

#include <QTranslator>
#include <QApplication>
#include <QList>
#include <QDir>

namespace
{
	QStringList getTranslationPaths(const QString& fourLetter)
	{
		const auto languageFile = Util::Language::getUsedLanguageFile(fourLetter);
		const auto languageDir = Util::translationsSharePath();

		QStringList filenames;
		filenames << QDir(languageDir).absoluteFilePath(languageFile)
		          << Util::Language::getCurrentQtTranslationPaths();

		return filenames;
	}

	bool initTranslator(QTranslator* translator, const QString& filename)
	{
		constexpr const auto InstanceName = "Translator";
		if(const auto loaded = translator->load(filename); !loaded)
		{
			translator->deleteLater();
			spLog(Log::Debug, InstanceName) << filename << " could not be loaded";
			return false;
		}

		if(translator->isEmpty())
		{
			translator->deleteLater();
			spLog(Log::Debug, InstanceName) << "Translator is empty";
			return false;
		}

		if(const auto installed = QApplication::installTranslator(translator); !installed)
		{
			translator->deleteLater();
			spLog(Log::Debug, InstanceName) << filename << " could not be installed";
			return false;
		}

		return true;
	}
}

struct Translator::Private
{
	QList<QTranslator*> translators;
};

Translator::Translator() :
	m {Pimpl::make<Private>()} {}

Translator::~Translator() = default;

void Translator::changeLanguage(QObject* parent, const QString& fourLetter)
{
	if(!m->translators.isEmpty())
	{
		for(auto* translator: m->translators)
		{
			QApplication::removeTranslator(translator);
			translator->deleteLater();
		}

		m->translators.clear();
	}

	const auto translationPaths = getTranslationPaths(fourLetter);
	for(const auto& filename: translationPaths)
	{
		auto* translator = new QTranslator(parent);
		if(!initTranslator(translator, filename))
		{
			translator->deleteLater();
			continue;
		}

		m->translators << translator;
	}
}
