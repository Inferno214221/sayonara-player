/* Translator.h */

/* Copyright (C) 2011-2019 Lucio Carreras
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



#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>

class Translator
{
	SINGLETON(Translator)
	PIMPL(Translator)

private:
	bool switch_translator(QObject* parent, const QString& four_letter, const QString& dir);

public:
	void change_language(QObject* parent, const QString& language);

};

#endif // TRANSLATOR_H