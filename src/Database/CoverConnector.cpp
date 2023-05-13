/* CoverConnector.cpp */

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

#include "CoverConnector.h"
#include "Query.h"
#include "Utils/Utils.h"
#include "Utils/Set.h"

using DB::Query;

DB::Covers::Covers(const QString& connection_name, DbId databaseId) :
	DB::Module(connection_name, databaseId)
{}

DB::Covers::~Covers() = default;

bool DB::Covers::exists(const QString& hash)
{
	auto q = runQuery(
		"SELECT hash FROM covers WHERE hash = :hash;",
		{
			{":hash", hash}
		},
		"Cannot check cover"
	);

	if(hasError(q))
	{
		return false;
	}

	return q.next();
}

bool DB::Covers::getCover(const QString& hash, QPixmap& pm)
{
	auto q = runQuery(
		"SELECT data FROM covers WHERE hash = :hash;",
		{
			{":hash", hash}
		},
		"Cannot fetch cover"
	);

	if(hasError(q))
	{
		return false;
	}

	if(q.next())
	{
		QByteArray data = q.value(0).toByteArray();
		pm = ::Util::convertByteArrayToPixmap(data);

		return true;
	}

	return false;
}

bool DB::Covers::setCover(const QString& hash, const QPixmap& pm)
{
	if(hash.isEmpty() || pm.isNull()){
		return false;
	}

	if(this->exists(hash))
	{
		return updateCover(hash, pm);
	}

	else
	{
		return insertCover(hash, pm);
	}
}

bool DB::Covers::updateCover(const QString& hash, const QPixmap& pm)
{
	const auto data = ::Util::convertPixmapToByteArray(pm);
	const auto q = update("covers",
	                      {{"data", data}},
	                      {"hash", hash},
	                      "Cannot update cover");

	return wasUpdateSuccessful(q);
}

bool DB::Covers::insertCover(const QString& hash, const QPixmap& pm)
{
	const auto data = ::Util::convertPixmapToByteArray(pm);
	const auto q = insert("covers",
	                      {
		                      {"data", data},
		                      {"hash", hash}
	                      }, "Cannot insert cover");

	return !hasError(q);
}

bool DB::Covers::removeCover(const QString& hash)
{
	const auto q = runQuery(
		"DELETE from covers WHERE hash=:hash;",
		{{":hash", hash}},
		"Cannot delete cover " + hash
	);

	return !hasError(q);
}

Util::Set<QString> DB::Covers::getAllHashes()
{
	auto q = runQuery("SELECT hash FROM covers;", "Cannot fetch all hashes");
	if(hasError(q))
	{
		return {};
	}

	Util::Set<QString> ret;
	while(q.next())
	{
		ret.insert(q.value(0).toString());
	}

	return ret;
}

bool DB::Covers::getAllCovers(QMap<QString, QPixmap>& covers)
{
	covers.clear();

	auto q = runQuery("SELECT hash, data FROM covers;", "Cannot fetch all covers");
	if(hasError(q))
	{
		return false;
	}

	while(q.next())
	{
		QString hash = q.value(0).toString();
		QByteArray data = q.value(1).toByteArray();

		covers[hash] = ::Util::convertByteArrayToPixmap(data);
	}

	return true;
}

void DB::Covers::clear()
{
	runQuery("DELETE FROM covers;", "Cannot drop all covers");
}
