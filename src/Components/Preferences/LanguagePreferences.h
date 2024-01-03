/* LanguagePreferences.h */
/*
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
#ifndef SAYONARA_PLAYER_LANGUAGEPREFERENCES_H
#define SAYONARA_PLAYER_LANGUAGEPREFERENCES_H

#include "Utils/Pimpl.h"
#include <QObject>
#include <QLocale>
#include <QList>
#include <utility>

class WebClient;

class LanguagePreferences :
	public QObject
{
	Q_OBJECT

	signals:
		void sigInfo(const QString& info);
		void sigWarning(const QString& warning);

	public:
		struct LanguageData
		{
			QString languageCode;
			QString languageName;
			QString iconPath;
		};

		explicit LanguagePreferences(QObject* parent);
		~LanguagePreferences() override;

		void checkForUpdate(const QString& languageCode);
		QString importLanguage(const QString& filename);

		static std::pair<QList<LanguageData>, int> getAllLanguages();

	private slots:
		void downloadFinished(WebClient* awa, const QString& languageCode);
		void updateCheckFinished(WebClient* awa, const QString& languageCode);

	private: // NOLINT(readability-redundant-access-specifiers)
		void downloadUpdate(const QString& languageCode);
};

#endif //SAYONARA_PLAYER_LANGUAGEPREFERENCES_H
