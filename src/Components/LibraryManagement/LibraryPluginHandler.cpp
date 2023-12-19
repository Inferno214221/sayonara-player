/* LibraryPluginHandler.cpp */

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

#include "LibraryPluginHandler.h"
#include "Components/LibraryManagement/LibraryContainer.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QDir>
#include <QPluginLoader>
#include <QJsonObject>
#include <QVariantMap>
#include <QWidget>

namespace
{
	using namespace Library;
	using ContainerList = QList<LibraryContainer*>;

	QString convertDisplayName(const QString& displayName)
	{
		auto ret = displayName.toLower().trimmed();
		ret.replace(" ", "-");

		return ret;
	}

	LibraryContainer* findLibrary(const ContainerList& containers, const QString& name)
	{
		spLog(Log::Debug, "LibraryPluginHandler") << "Searching for Library " << name;

		const auto it = Util::Algorithm::find(containers, [&name](auto* container) {
			return (container->name() == name);
		});

		return (it != containers.end()) ? *it : nullptr;
	}

	class PluginHandlerImpl :
		public PluginHandler
	{
		public:
			PluginHandlerImpl() = default;

			~PluginHandlerImpl() override
			{
				for(auto* container: m_libraryContainers)
				{
					delete container;
				}

				m_libraryContainers.clear();
			}

			void init(const QList<LibraryContainer*>& containers, LibraryContainer* fallbackLibrary) override
			{
				m_emptyLibrary = fallbackLibrary;

				initLibraries(containers);

				if(auto* lastLibrary = findLibrary(m_libraryContainers, GetSetting(Set::Lib_CurPlugin)); lastLibrary)
				{
					setCurrentLibrary(lastLibrary);
					return;
				}

				const auto localLibraryIndex = Util::Algorithm::indexOf(m_libraryContainers, [&](auto* container) {
					return container->isLocal();
				});

				setCurrentLibrary(localLibraryIndex);

			}

			void setCurrentLibrary(const QString& name) override
			{
				setCurrentLibrary(findLibrary(m_libraryContainers, name));
			}

			void setCurrentLibrary(int index) override
			{
				auto* result = m_emptyLibrary;

				if(!m_libraryContainers.isEmpty() && (index >= 0))
				{
					index = std::min(index, m_libraryContainers.size() - 1);
					result = m_libraryContainers[index];
				}

				setCurrentLibrary(result);
			}

			void setCurrentLibrary(LibraryContainer* currentLibrary) override
			{
				if(!currentLibrary)
				{
					currentLibrary = m_libraryContainers.isEmpty()
					                 ? m_emptyLibrary
					                 : m_libraryContainers.first();
				}

				m_currentLibrary = currentLibrary;

				if(m_currentLibrary)
				{
					m_currentLibrary->init();
				}

				SetSetting(Set::Lib_CurPlugin, currentLibrary->name());

				emit sigCurrentLibraryChanged();
			}

			[[nodiscard]] LibraryContainer* currentLibrary() const override { return m_currentLibrary; }

			[[nodiscard]] QWidget* currentLibraryWidget() const override
			{
				return (m_currentLibrary)
				       ? m_currentLibrary->widget()
				       : nullptr;
			}

			void addLocalLibrary(Library::LibraryContainer* container) override
			{
				if(!container)
				{
					return;
				}

				auto index = Util::Algorithm::indexOf(m_libraryContainers, [empty = m_emptyLibrary](auto* container) {
					return !container->isLocal() &&
					       (container != empty);
				});

				index = std::max(index, 0);
				m_libraryContainers.insert(index, container);

				emit sigLibrariesChanged();

				setCurrentLibrary(index);
			}

			void renameLocalLibrary(const QString& oldName, const QString& newName) override
			{
				auto* container = findLibrary(m_libraryContainers, convertDisplayName(oldName));
				if(container && container->isLocal())
				{
					container->rename(newName);
					emit sigLibrariesChanged();
				}
			}

			void removeLocalLibrary(const QString& name) override
			{
				auto* container = findLibrary(m_libraryContainers, convertDisplayName(name));
				if(container && container->isLocal())
				{
					m_libraryContainers.removeAll(container);

					if(m_currentLibrary == container)
					{
						setCurrentLibrary(0);
					}

					emit sigLibrariesChanged();
				}
			}

			void moveLocalLibrary(const int oldIndex, const int newIndex) override
			{
				if(Util::between(oldIndex, m_libraryContainers) &&
				   Util::between(newIndex, m_libraryContainers))
				{
					m_libraryContainers.move(oldIndex, newIndex);
					emit sigLibrariesChanged();
				}
			}

			[[nodiscard]] QList<Library::LibraryContainer*> libraries(const bool alsoEmpty) const override
			{
				auto containers = m_libraryContainers;
				if(alsoEmpty)
				{
					containers.push_front(m_emptyLibrary);
				}

				return containers;
			}

		private:
			void initLibraries(const QList<Library::LibraryContainer*>& containers)
			{
				for(auto* container: containers)
				{
					if(container)
					{
						spLog(Log::Debug, this) << "Add library " << container->displayName();
						m_libraryContainers << container;
					}
				}
			}

			LibraryContainer* m_emptyLibrary {nullptr};
			LibraryContainer* m_currentLibrary {nullptr};
			ContainerList m_libraryContainers;
	};
}

namespace Library
{
	PluginHandler::~PluginHandler() = default;

	PluginHandler* PluginHandler::create()
	{
		return new PluginHandlerImpl();
	}
}

