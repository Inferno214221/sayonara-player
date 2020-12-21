/* DatabaseAccess.cpp */
/*
 * Copyright (C) 2011-2020 Michael Lugmair
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

#include "CoverPersistence.h"
#include "CoverLocation.h"
#include "Database/Connector.h"
#include "Database/CoverConnector.h"
#include "Utils/Settings/Settings.h"

#include <QDir>
#include <QPixmap>
#include <QString>

void Cover::writeCoverIntoDatabase(const Cover::Location& cl, const QPixmap& pm)
{
	writeCoverIntoDatabase(cl, pm, DB::Connector::instance());
}

void Cover::writeCoverIntoDatabase(const Cover::Location& cl, const QPixmap& pm, DB::Connector* db)
{
	auto* dbc = db->coverConnector();
	dbc->setCover(cl.hash(), pm);
}

void Cover::writeCoverToLibrary(const Cover::Location& cl, const QPixmap& pm)
{
	const auto localDir = cl.localPathDir();
	if(localDir.isEmpty())
	{
		return;
	}

	auto coverTemplate = GetSetting(Set::Cover_TemplatePath);
	coverTemplate.replace("<h>", cl.hash());

	const auto extension = pm.hasAlphaChannel() ? "png" : "jpg";
	const auto filepath = QDir(localDir).absoluteFilePath(coverTemplate) + "." + extension;

	pm.save(filepath);
}
