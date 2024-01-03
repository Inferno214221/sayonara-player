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
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Components/LibraryManagement/LibraryContainer.h"

#include "Utils/Algorithm.h"

#include <QList>
#include <utility>

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	class LocalContainer :
		public Library::LibraryContainer
	{
		public:
			explicit LocalContainer(const int index) :
				m_name {QString("local-%1").arg(index)} {}

			~LocalContainer() override = default;

			[[nodiscard]] QFrame* header() const override { return nullptr; }

			[[nodiscard]] QIcon icon() const override { return {}; }

			[[nodiscard]] QMenu* menu() override { return nullptr; }

			[[nodiscard]] QString displayName() const override { return name().toUpper(); }

			[[nodiscard]] QString name() const override { return m_name; }

			[[nodiscard]] QWidget* widget() const override { return nullptr; }

			[[nodiscard]] bool isLocal() const override { return true; }

			void init() override {}

			void rename(const QString& newName) override { m_name = newName; }

		private:
			QString m_name;
			QString m_displayName;
	};

	class StandardContainer :
		public Library::LibraryContainer
	{
		public:
			explicit StandardContainer(QString name) :
				m_name {std::move(name)} {}

			~StandardContainer() override = default;

			[[nodiscard]] QFrame* header() const override { return nullptr; }

			[[nodiscard]] QIcon icon() const override { return {}; }

			[[nodiscard]] QMenu* menu() override { return nullptr; }

			[[nodiscard]] QString displayName() const override { return name(); }

			[[nodiscard]] QString name() const override { return m_name; }

			[[nodiscard]] QWidget* widget() const override { return nullptr; }

			[[nodiscard]] bool isLocal() const override { return false; }

			void init() override {}

			void rename(const QString& /*newName*/) override {}

		private:
			QString m_name;

	};

	QList<Library::LibraryContainer*> createLocalContainers(const int count)
	{
		auto result = QList<Library::LibraryContainer*> {};
		for(int i = 0; i < count; i++)
		{
			result << new LocalContainer(i);
		}

		return result;
	}

	QStringList names(const QList<Library::LibraryContainer*>& libraries)
	{
		QStringList result;
		Util::Algorithm::transform(libraries, result, [](auto* library) {
			return library->name();
		});

		return result;
	}
}

class LibraryPluginHandlerTest :
	public Test::Base
{
	Q_OBJECT

	public:
		LibraryPluginHandlerTest() :
			Test::Base("LibraryPluginHandlerTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testBehaviorIfNoLibFound()
		{
			auto* lph = Library::PluginHandler::create();
			QVERIFY(lph->currentLibrary() == nullptr);
			QVERIFY(lph->currentLibraryWidget() == nullptr);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testAfterInitialization()
		{
			constexpr const auto* FallbackName = "Fallback";
			constexpr const auto Count = 5;

			auto* lph = Library::PluginHandler::create();
			const auto containers = createLocalContainers(Count);
			lph->init(containers, new StandardContainer(FallbackName));

			QVERIFY(lph->libraries(false).count() == Count);
			QVERIFY(lph->libraries(true).count() == Count + 1);
			QVERIFY(lph->currentLibrary() == containers[0]);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testSetCurrentLibraryIndex()
		{
			constexpr const auto* FallbackName = "Fallback";
			struct TestCase
			{
				int count {0};
				int newIndex;
				QString expectedName;
			};

			const auto testCases = std::array {
				TestCase {0, 0, FallbackName},
				TestCase {0, 7, FallbackName},
				TestCase {3, 0, "local-0"},
				TestCase {3, 7, "local-2"},
				TestCase {3, 1, "local-1"},
			};

			for(const auto& testCase: testCases)
			{
				auto* lph = Library::PluginHandler::create();
				const auto containers = createLocalContainers(testCase.count);
				lph->init(containers, new StandardContainer(FallbackName));
				lph->setCurrentLibrary(testCase.newIndex);

				QVERIFY(lph->currentLibrary()->name() == testCase.expectedName);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testSetCurrentLibraryByName()
		{
			constexpr const auto* FallbackName = "Fallback";

			struct TestCase
			{
				int count {0};
				QString newName;
				QString expectedName;
			};

			const auto testCases = std::array {
				TestCase {0, FallbackName, FallbackName},
				TestCase {3, "local-0", "local-0"},
				TestCase {3, FallbackName, "local-0"},
				TestCase {3, "something", "local-0"},
				TestCase {2, "local-3", "local-0"}
			};

			for(const auto& testCase: testCases)
			{
				auto* lph = Library::PluginHandler::create();
				const auto containers = createLocalContainers(testCase.count);
				lph->init(containers, new StandardContainer(FallbackName));
				lph->setCurrentLibrary(testCase.newName);

				QVERIFY(lph->currentLibrary()->name() == testCase.expectedName);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testSetCurrentLibraryByLibrary()
		{
			constexpr const auto* FallbackName = "Fallback";
			constexpr const auto Count = 5;

			struct TestCase
			{
				Library::LibraryContainer* library;
				QString expectedName;
			};

			const auto containers = createLocalContainers(Count);
			auto unregisteredContainer = LocalContainer(100); // NOLINT(readability-magic-numbers)

			const auto testCases = std::array {
				TestCase {containers[0], "local-0"},
				TestCase {containers[1], "local-1"},
				TestCase {containers[4], "local-4"},
				TestCase {nullptr, "local-0"},
				TestCase {&unregisteredContainer, "local-100"},
			};

			for(const auto& testCase: testCases)
			{
				auto* lph = Library::PluginHandler::create();

				lph->init(containers, new StandardContainer(FallbackName));
				lph->setCurrentLibrary(testCase.library);

				QVERIFY(lph->currentLibrary()->name() == testCase.expectedName);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testAddLocalLibrary()
		{
			constexpr const auto* FallbackName = "Fallback";
			constexpr const auto Count = 5;

			auto* lph = Library::PluginHandler::create();
			const auto containers = createLocalContainers(Count);
			lph->init(containers, new StandardContainer(FallbackName));

			auto unregisteredContainer = LocalContainer(100); // NOLINT(readability-magic-numbers)
			lph->addLocalLibrary(&unregisteredContainer);

			QVERIFY(lph->currentLibrary()->name() == "local-100");
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testRenameLocalLibrary()
		{
			constexpr const auto* FallbackName = "Fallback";
			constexpr const auto* NewName = "NewName";
			constexpr const auto Count = 5;

			auto* lph = Library::PluginHandler::create();
			const auto containers = createLocalContainers(Count);
			lph->init(containers, new StandardContainer(FallbackName));

			lph->setCurrentLibrary(3);
			auto oldName = lph->currentLibrary()->name();

			lph->setCurrentLibrary(0);
			lph->renameLocalLibrary(oldName, NewName);

			lph->setCurrentLibrary(3);
			QVERIFY(lph->currentLibrary()->name() == NewName);

			lph->setCurrentLibrary(0);
			lph->setCurrentLibrary(NewName);
			QVERIFY(lph->currentLibrary()->name() == NewName);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testRemoveLocalLibrary()
		{
			constexpr const auto* FallbackName = "Fallback";
			constexpr const auto Count = 5;

			auto* lph = Library::PluginHandler::create();
			const auto containers = createLocalContainers(Count);
			lph->init(containers, new StandardContainer(FallbackName));

			lph->setCurrentLibrary(3);

			const auto name = lph->currentLibrary()->name();
			lph->removeLocalLibrary(name);

			QVERIFY(lph->libraries(false).count() == Count - 1);

			lph->setCurrentLibrary(name);
			QVERIFY(lph->currentLibrary()->name() == "local-0");
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testMoveLocalLibrary()
		{
			constexpr const auto* FallbackName = "Fallback";
			constexpr const auto Count = 3;

			struct TestCase
			{
				int from {0};
				int to {0};
				QStringList expectedNames;
			};

			const auto testCases = std::array {
				TestCase {1, 2, {"local-0", "local-2", "local-1"}},
				TestCase {2, 0, {"local-2", "local-0", "local-1"}},
				TestCase {1, 5, {"local-0", "local-1", "local-2"}},
				TestCase {5, 1, {"local-0", "local-1", "local-2"}}
			};

			for(const auto& testCase: testCases)
			{
				auto* lph = Library::PluginHandler::create();
				const auto containers = createLocalContainers(Count);
				lph->init(containers, new StandardContainer(FallbackName));
				lph->moveLocalLibrary(testCase.from, testCase.to);

				const auto libraryNames = names(lph->libraries(false));
				QVERIFY(libraryNames == testCase.expectedNames);
			}
		}
};

QTEST_GUILESS_MAIN(LibraryPluginHandlerTest)

#include "LibraryPluginHandlerTest.moc"
