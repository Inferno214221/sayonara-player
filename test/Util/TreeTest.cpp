/* TreeTest.cpp
 *
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

#include "test/Common/SayonaraTest.h"

#include "Utils/Tree.h"

#include <QString>

// access working directory with Test::Base::tempPath("somefile.txt");

class TreeTest : 
    public Test::Base
{
    Q_OBJECT

    public:
        TreeTest() :
            Test::Base("TreeTest")
        {}

    private slots:
        void addTest();
        void removeTest();
};

void TreeTest::addTest()
{
	auto tree = Util::Tree<QString>("root");

	auto* node1 = tree.addChild("node1");
	auto* node2 = tree.addChild("node2");
	auto* node3 = tree.addChild("node3");

	QVERIFY(tree.children.size() == 3);

	QVERIFY(tree.children[0] == node1);
	QVERIFY(node1->data == "node1");
	QVERIFY(node1->parent->data == "root");
	QVERIFY(node1->children.size() == 0);

	QVERIFY(tree.children[1] == node2);
	QVERIFY(node2->data == "node2");
	QVERIFY(node2->parent->data == "root");
	QVERIFY(node2->children.size() == 0);

	QVERIFY(tree.children[2] == node3);
	QVERIFY(node3->data == "node3");
	QVERIFY(node3->parent->data == "root");
	QVERIFY(node3->children.size() == 0);

	node1->addChild("node11");
	node1->addChild("node12");

	QVERIFY(node1->children.size() == 2);
	QVERIFY(node1->children[0]->parent->data == "node1");
	QVERIFY(node1->children[1]->parent->data == "node1");
}

void TreeTest::removeTest()
{
	auto tree = Util::Tree<QString>("root");
	auto* node1 = tree.addChild("node1");
	auto* node2 = tree.addChild("node2");
	auto* node3 = tree.addChild("node3");

	auto* removedNode = tree.removeChild(node2);
	QVERIFY(removedNode == node2);
	QVERIFY(removedNode->parent == nullptr);

	QVERIFY(tree.children.size() == 2);
	QVERIFY(tree.children[0] == node1);
	QVERIFY(tree.children[1] == node3);

	removedNode = tree.removeChild(node2);
	QVERIFY(tree.children.size() == 2);
	QVERIFY(removedNode == nullptr);
}


QTEST_GUILESS_MAIN(TreeTest)
#include "TreeTest.moc"
