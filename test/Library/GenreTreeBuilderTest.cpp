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
#include "Utils/MetaData/Genre.h"
#include "Utils/Set.h"

#include "Components/Library/GenreTreeBuilder.h"

#include <QStringList>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	Util::Set<Genre> createGenreSet(const QStringList& genres)
	{
		auto result = Util::Set<Genre> {};
		for(const auto& name: genres)
		{
			result.insert(Genre {name});
		}

		return result;
	}

	bool verifyChildren(GenreTreeBuilder::GenreNode* node, const QStringList& expectedStrings)
	{
		if(node->children.count() != expectedStrings.count())
		{
			return false;
		}

		for(int i = 0; i < expectedStrings.count(); i++)
		{
			if(node->children[i]->data != expectedStrings[i])
			{
				return false;
			}
		}

		return true;
	}
}

class GenreTreeBuilderTest :
	public Test::Base
{
	Q_OBJECT

	public:
		GenreTreeBuilderTest() :
			Test::Base("GenreTreeBuilder") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void test()
		{
			const auto genres = createGenreSet(
				{"Metal",
				 "Groove Metal",
				 "Nu metal"
				});
			auto* genreNode = GenreTreeBuilder::buildGenreDataTree(genres, true);
			QVERIFY(genreNode->data.isEmpty());
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testEmptyGenreCannotBeParent()
		{
			const auto genres = createGenreSet(
				{"Rock",
				 "Metal",
				 "Jazz",
				 ""
				});

			auto* genreNode = GenreTreeBuilder::buildGenreDataTree(genres, true);
			QVERIFY(genreNode->data.isEmpty());
			QVERIFY(verifyChildren(genreNode, {"", "Jazz", "Metal", "Rock"}));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testZeroChildren()
		{
			const auto genres = createGenreSet(
				{"Rock",
				 "Metal",
				 "Jazz"
				});

			auto* genreNode = GenreTreeBuilder::buildGenreDataTree(genres, true);
			QVERIFY(genreNode->data.isEmpty());
			QVERIFY(verifyChildren(genreNode, {"Jazz", "Metal", "Rock"}));

			auto* jazzNode = genreNode->children[0];
			QVERIFY(verifyChildren(jazzNode, {}));

			auto* metalNode = genreNode->children[1];
			QVERIFY(verifyChildren(metalNode, {}));

			auto* rockNode = genreNode->children[2];
			QVERIFY(verifyChildren(rockNode, {}));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testOneChild()
		{
			const auto genres = createGenreSet(
				{"Rock",
				 "Metal",
				 "Heavy Metal",
				 "Krautrock",
				 "Thrash Metal",
				 "Psychodelic rock",
				 "Jazz"
				});

			auto* genreNode = GenreTreeBuilder::buildGenreDataTree(genres, true);
			QVERIFY(genreNode->data.isEmpty());
			QVERIFY(verifyChildren(genreNode, {"Jazz", "Metal", "Rock"}));

			auto* jazzNode = genreNode->children[0];
			QVERIFY(verifyChildren(jazzNode, {}));

			auto* metalNode = genreNode->children[1];
			QVERIFY(verifyChildren(metalNode, {"Heavy Metal", "Thrash Metal"}));

			auto* rockNode = genreNode->children[2];
			rockNode->sort(true);
			QVERIFY(verifyChildren(rockNode, {"Krautrock", "Psychodelic rock"}));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testMultiParents()
		{
			const auto genres = createGenreSet(
				{"Rock",
				 "Krautrock",
				 "Psychodelic",
				 "Psychodelic rock"
				});

			auto* genreNode = GenreTreeBuilder::buildGenreDataTree(genres, true);
			QVERIFY(verifyChildren(genreNode, {"Psychodelic", "Rock"}));

			auto* psyNode = genreNode->children[0];
			QVERIFY(verifyChildren(psyNode, {"Psychodelic rock"}));

			auto* rockNode = genreNode->children[1];
			QVERIFY(verifyChildren(rockNode, {"Krautrock", "Psychodelic rock"}));
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testDeep()
		{
			const auto genres = createGenreSet(
				{"Rock",
				 "Classic Rock",
				 "Southern classic Rock",
				 "Instrumental southern classic rock"
				});

			auto* genreNode = GenreTreeBuilder::buildGenreDataTree(genres, true);
			QVERIFY(genreNode->children[0]->data == "Rock");

			auto* rockNode = genreNode->children[0];
			QVERIFY(verifyChildren(rockNode, {"Classic Rock"}));

			auto* classicRockNode = rockNode->children[0];
			QVERIFY(verifyChildren(classicRockNode, {"Southern classic Rock"}));

			auto* southernNode = classicRockNode->children[0];
			QVERIFY(verifyChildren(southernNode, {"Instrumental southern classic rock"}));
		}
};

QTEST_GUILESS_MAIN(GenreTreeBuilderTest)

#include "GenreTreeBuilderTest.moc"
