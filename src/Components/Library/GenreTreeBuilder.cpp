/* GenreTreeBuilder.cpp, (Created on 01.01.2024) */

/* Copyright (C) 2011-2024 Michael Lugmair
 *
 * This file is part of Sayonara Player
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

#include "GenreTreeBuilder.h"
#include "Utils/Set.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Algorithm.h"

#include <QString>
#include <QMap>

namespace
{
	using GenreTreeBuilder::GenreNode;
	using StringSet = Util::Set<QString>;

	bool isChildNodeOf(const QString& genre, const QString& otherGenre)
	{
		if(otherGenre.isEmpty() || (genre.toLower() == otherGenre.toLower()))
		{
			return false;
		}

		return genre.toLower().contains(otherGenre.toLower());
	}

	bool isChildNodeOfSibling(const StringSet& siblings, const QString& name)
	{
		return Util::Algorithm::contains(siblings, [&](const auto& sibling) {
			return isChildNodeOf(name, sibling);
		});
	}

	bool isRootGenre(const Genre& rootGenre, const Util::Set<Genre>& genres)
	{
		return std::none_of(genres.begin(), genres.end(), [&](const auto& genre) {
			return isChildNodeOf(rootGenre.name(), genre.name()); // NOLINT(*-suspicious-call-argument)
		});
	}

	void buildGenreNode(GenreNode* node, const QMap<QString, StringSet>& childMap) // NOLINT(*-no-recursion)
	{
		const auto currentGenre = node->data;
		const auto& children = childMap[currentGenre];

		const auto isLeaf = children.isEmpty();
		if(isLeaf)
		{
			return;
		}

		for(const auto& childName: children)
		{
			auto* newChild = new GenreNode(childName);
			newChild->parent = node;

			const auto isSiblingMoreSpecific = isChildNodeOfSibling(children, childName);
			if(!isSiblingMoreSpecific)
			{
				node->addChild(newChild);
				buildGenreNode(newChild, childMap);
			}
		}

		node->sort(true);
	}

	StringSet getChildrenOfGenre(const Genre& genre, const Util::Set<Genre>& genres) // O(n)
	{
		auto result = StringSet {};
		for(const auto& childGenre: genres)
		{
			const auto childName = childGenre.name();
			if(childName.isEmpty() || (childName == genre.name()))
			{
				continue;
			}

			if(isChildNodeOf(childGenre.name(), genre.name())) // NOLINT(*-suspicious-call-argument)
			{
				result.insert(childGenre.name());
			}
		}

		return result;
	}

	QMap<QString, StringSet> getChildrenMap(const Util::Set<Genre>& genres) // O(n^2)
	{
		auto childGenreMap = QMap<QString, StringSet> {};
		for(const auto& genre: genres) // O(n)
		{
			const auto children = getChildrenOfGenre(genre, genres); // O(n)
			childGenreMap.insert(genre.name(), children);
		}

		return childGenreMap;
	}
}

GenreNode* GenreTreeBuilder::buildGenreDataTree(const Util::Set<Genre>& genres, const bool createTree)
{
	auto* result = new GenreNode(QString {});

	const auto rootGenres = StringSet {};
	const auto childGenreMap = getChildrenMap(genres); // O(n^2)
	for(const auto& genre: genres) // O(n)
	{
		const auto name = genre.name();
		if(isRootGenre(genre, genres) || !createTree) // O(n)
		{
			result->addChild(genre.name());
		}
	}

	for(auto* baseGenre: result->children)
	{
		buildGenreNode(baseGenre, childGenreMap);
	}

	result->sort(true);

	return result;
}