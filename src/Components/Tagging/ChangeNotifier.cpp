/* MetaDataChangeNotifier.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "ChangeNotifier.h"
#include "Utils/MetaData/MetaDataList.h"

using namespace Tagging;

ChangeNotifier::ChangeNotifier(QObject *parent) :
	QObject(parent) {}

ChangeNotifier::~ChangeNotifier() {}

void ChangeNotifier::change_metadata(const MetaDataList& v_md_old, const MetaDataList& v_md_new)
{
	emit sig_metadata_changed(v_md_old, v_md_new);
}

void ChangeNotifier::delete_metadata(const MetaDataList& v_md_deleted)
{
	emit sig_metadata_deleted(v_md_deleted);
}
