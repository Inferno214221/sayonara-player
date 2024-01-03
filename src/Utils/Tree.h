
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

#ifndef TREE_H
#define TREE_H

#include <QList>
#include <algorithm>

namespace Util
{
	template<typename T>
	/**
	 * @brief The Tree class
	 * @ingroup Helper
	 */
	class Tree
	{
		public:
			Tree* parent = nullptr;
			T data;
			QList<Tree*> children;

			Tree() :
				Tree(T {}) {}

			explicit Tree(const T& data) :
				parent(nullptr),
				data(data) {}

			~Tree()
			{
				for(auto* child: children)
				{
					delete child;
					child = nullptr;
				}

				children.clear();
				data = T();
			}

			/**
			 * @brief adds a child to the given node
			 * @param node the parent node
			 * @return pointer to inserted node
			 */
			Tree* addChild(Tree* node)
			{
				node->parent = this;

				this->children << node;
				this->sort(false);

				return node;
			}

			Tree* addChild(const T& data)
			{
				auto* node = new Tree(data);
				return addChild(node);
			}

			/**
			 * @brief remove a node from the current node
			 * @param deletedNode node to remove
			 * @return pointer to deletedNode
			 */
			Tree* removeChild(Tree* deletedNode)
			{
				auto it = std::find_if(children.begin(), children.end(), [&](const auto* node) {
					return (node == deletedNode);
				});

				if(it != children.end())
				{
					auto* node = *it;
					children.erase(it);
					node->parent = nullptr;
					return node;
				}

				return nullptr;
			}

			/**
			 * @brief sort children of all nodes in ascending way according to their data
			 * @param recursive if set to true, do it for all subnodes, too
			 */
			void sort(bool recursive)
			{
				if(children.isEmpty())
				{
					return;
				}

				auto lambda = [](auto* tree1, auto* tree2) {
					return (tree1->data < tree2->data);
				};

				std::sort(children.begin(), children.end(), lambda);

				if(recursive)
				{
					for(auto* child: children)
					{
						child->sort(recursive);
					}
				}
			}
	};
}

#endif // TREE_H
