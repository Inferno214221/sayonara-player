/* VersionChecker.h */

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



#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QObject>
#include "Utils/Pimpl.h"

class VersionChecker : public QObject
{
	Q_OBJECT

signals:
	void sig_finished();

public:
	explicit VersionChecker(QObject* parent);
	~VersionChecker();

private slots:
	void version_check_finished();
};

#endif // VERSIONCHECKER_H