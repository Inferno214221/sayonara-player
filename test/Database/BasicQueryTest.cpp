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

#include "Common/SayonaraTest.h"
#include "Database/Base.h"
#include "Database/Connector.h"
#include "Database/Fixes.h"
#include "Database/Query.h"

#include <QMap>
#include <QVariant>
#include <QSqlQuery>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	class FixesMock :
		public DB::Fixes
	{
		public:
			using DB::Fixes::Fixes;

			void applyFixes() override {}
	};

	QString createTableName()
	{
		static int counter = 0;
		auto result = QString("Table%1").arg(counter);
		counter++;

		return result;
	}

	QString createColumnName()
	{
		static int counter = 0;
		auto result = QString("Column%1").arg(counter);
		counter++;

		return result;
	}

	QString createTableStatement(const QString& tableName)
	{
		return QString(R"(
			CREATE TABLE %1
			(
				name VARCHAR(255) PRIMARY KEY
			);
		)").arg(tableName);
	}
}

class BasicQueryTest :
	public Test::Base
{
	Q_OBJECT

	public:
		BasicQueryTest() :
			Test::Base("BasicQueryTest") {}

	private slots:

		[[maybe_unused]] void testCreateTable() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto tableName = createTableName();

			auto* db = DB::Connector::instance();
			auto fixes = FixesMock(db->connectionName(), db->databaseId());
			const auto tableCreated = fixes.checkAndCreateTable(tableName, createTableStatement(tableName));

			QVERIFY(tableCreated);
		};

		[[maybe_unused]] void testInsertColumn() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto tableName = createTableName();
			const auto columnName = createColumnName();

			auto* db = DB::Connector::instance();
			auto fixes = FixesMock(db->connectionName(), db->databaseId());

			const auto insertedIntoNonExistingTable = fixes.checkAndInsertColumn(tableName, columnName, "INTEGER", "3");
			QVERIFY(!insertedIntoNonExistingTable);

			fixes.checkAndCreateTable(tableName, createTableStatement(tableName));

			const auto columnInserted = fixes.checkAndInsertColumn(tableName, columnName, "INTEGER", "3");
			QVERIFY(columnInserted);
		}

		[[maybe_unused]] void testFillData() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto tableName = createTableName();
			const auto columnName = createColumnName();

			auto* db = DB::Connector::instance();
			const auto queryInsertIntoMissingTable = db->insert(tableName, {
				{"name",     "Some name"},
				{columnName, 5}
			}, "Table not existent");

			QVERIFY(DB::hasError(queryInsertIntoMissingTable));

			auto fixes = FixesMock(db->connectionName(), db->databaseId());
			fixes.checkAndCreateTable(tableName, createTableStatement(tableName));

			const auto queryInsertIntoTableWithMissingColumn = db->insert(tableName, {
				{"name",     "Some name"},
				{columnName, 5}
			}, "Column not existent");
			QVERIFY(DB::hasError(queryInsertIntoTableWithMissingColumn));

			fixes.checkAndInsertColumn(tableName, columnName, "INTEGER", "3");
			const auto successfulQuery = db->insert(tableName, {
				{"name",     "Some name"},
				{columnName, 5}
			}, "Unpredicted error occured");
			QVERIFY(!DB::hasError(successfulQuery));
		}

		[[maybe_unused]] void testUpdateData() // NOLINT(readability-convert-member-functions-to-static)
		{
			const auto tableName = createTableName();
			const auto columnName = createColumnName();

			auto* db = DB::Connector::instance();
			const auto queryUpdateMissingTable = db->update(tableName, {
				{"name", "New name"}
			}, {}, "Table not existent");

			QVERIFY(DB::hasError(queryUpdateMissingTable));

			auto fixes = FixesMock(db->connectionName(), db->databaseId());
			fixes.checkAndCreateTable(tableName, createTableStatement(tableName));

			const auto queryUpdateMissingColumn = db->update(tableName, {
				{"invalidColumn", "Some value"},
			}, {}, "Column not existent");
			QVERIFY(DB::hasError(queryUpdateMissingColumn));

			const auto queryUpdateMissingRow =
				db->update(tableName,
				           {{"name", "New name"}},
				           {"name", "Old name"},
				           "Unpredicted error occured");
			QVERIFY(!DB::hasError(queryUpdateMissingRow));
			QVERIFY(!DB::wasUpdateSuccessful(queryUpdateMissingRow));

			const auto temporaryName = QStringLiteral("Some name");
			const auto newName = QStringLiteral("New Name");

			db->insert(tableName, {{"name", temporaryName}}, "Insert failed");

			const auto querySuccessfulUpdate =
				db->update(tableName,
				           {{"name", newName}},
				           {"name", temporaryName},
				           "Unpredicted error occured");
			QVERIFY(!DB::hasError(querySuccessfulUpdate));
			QVERIFY(DB::wasUpdateSuccessful(querySuccessfulUpdate));

			auto querySelectData = db->runQuery(
				QString("SELECT COUNT(name) FROM %1 WHERE name = :name;").arg(tableName),
				{":name", newName},
				"Unexpected Error");

			QVERIFY(!DB::hasError(querySelectData));
			QVERIFY(querySelectData.next());
			QVERIFY(querySelectData.value(0).toInt() == 1);
		}
};

QTEST_GUILESS_MAIN(BasicQueryTest)

#include "BasicQueryTest.moc"
