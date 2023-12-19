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

namespace Library
{
	using ContainerList = QList<LibraryContainer*>;

	namespace
	{
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
	}

	struct PluginHandler::Private
	{
		LibraryContainer* emptyLibrary = nullptr;
		LibraryContainer* currentLibrary = nullptr;
		ContainerList libraryContainers;
	};

	PluginHandler::PluginHandler() :
		QObject(nullptr),
		m {Pimpl::make<Private>()} {}

	PluginHandler::~PluginHandler() = default;

	void PluginHandler::init(const ContainerList& containers, LibraryContainer* fallbackLibrary)
	{
		m->emptyLibrary = fallbackLibrary;

		initLibraries(containers);

		if(auto* lastLibrary = findLibrary(m->libraryContainers, GetSetting(Set::Lib_CurPlugin)); lastLibrary)
		{
			setCurrentLibrary(lastLibrary);
			return;
		}

		const auto localLibraryIndex = Util::Algorithm::indexOf(m->libraryContainers, [&](auto* container) {
			return container->isLocal();
		});

		setCurrentLibrary(localLibraryIndex);

	}

	void PluginHandler::shutdown()
	{
		for(auto* container: m->libraryContainers)
		{
			delete container;
		}

		m->libraryContainers.clear();
	}

	void PluginHandler::initLibraries(const QList<Library::LibraryContainer*>& containers)
	{
		for(auto* container: containers)
		{
			if(container)
			{
				spLog(Log::Debug, this) << "Add library " << container->displayName();
				m->libraryContainers << container;
			}
		}
	}

	void PluginHandler::setCurrentLibrary(const QString& name)
	{
		setCurrentLibrary(findLibrary(m->libraryContainers, name));
	}

	void PluginHandler::setCurrentLibrary(int index)
	{
		auto* result = m->emptyLibrary;

		if(!m->libraryContainers.isEmpty() && (index >= 0))
		{
			index = std::min(index, m->libraryContainers.size() - 1);
			result = m->libraryContainers[index];
		}

		setCurrentLibrary(result);
	}

	void PluginHandler::setCurrentLibrary(LibraryContainer* currentLibrary)
	{
		if(!currentLibrary)
		{
			currentLibrary = m->libraryContainers.isEmpty()
			                 ? m->emptyLibrary
			                 : m->libraryContainers.first();
		}

		m->currentLibrary = currentLibrary;

		if(m->currentLibrary)
		{
			m->currentLibrary->init();
		}

		SetSetting(Set::Lib_CurPlugin, currentLibrary->name());

		emit sigCurrentLibraryChanged();
	}

	LibraryContainer* PluginHandler::currentLibrary() const { return m->currentLibrary; }

	QWidget* PluginHandler::currentLibraryWidget() const
	{
		return (m->currentLibrary)
		       ? m->currentLibrary->widget()
		       : nullptr;
	}

	void PluginHandler::addLocalLibrary(Library::LibraryContainer* container)
	{
		if(!container)
		{
			return;
		}

		auto index = Util::Algorithm::indexOf(m->libraryContainers, [empty = m->emptyLibrary](auto* container) {
			return !container->isLocal() &&
			       (container != empty);
		});

		index = std::max(index, 0);
		m->libraryContainers.insert(index, container);

		emit sigLibrariesChanged();

		setCurrentLibrary(index);
	}

	void PluginHandler::renameLocalLibrary(const QString& oldName, const QString& new_name)
	{
		auto* container = findLibrary(m->libraryContainers, convertDisplayName(oldName));
		if(container && container->isLocal())
		{
			container->rename(new_name);
			emit sigLibrariesChanged();
		}
	}

	void PluginHandler::removeLocalLibrary(const QString& name)
	{
		auto* container = findLibrary(m->libraryContainers, convertDisplayName(name));
		if(container && container->isLocal())
		{
			m->libraryContainers.removeAll(container);

			if(m->currentLibrary == container)
			{
				setCurrentLibrary(0);
			}

			emit sigLibrariesChanged();
		}
	}

	void PluginHandler::moveLocalLibrary(const int oldIndex, const int newIndex)
	{
		if(Util::between(oldIndex, m->libraryContainers) &&
		   Util::between(newIndex, m->libraryContainers))
		{
			m->libraryContainers.move(oldIndex, newIndex);
			emit sigLibrariesChanged();
		}
	}

	QList<Library::LibraryContainer*> PluginHandler::libraries(const bool alsoEmpty) const
	{
		auto containers = m->libraryContainers;
		if(alsoEmpty)
		{
			containers.push_front(m->emptyLibrary);
		}

		return containers;
	}
}