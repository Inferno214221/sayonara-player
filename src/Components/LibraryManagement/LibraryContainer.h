#ifndef SAYONARA_PLAYER_COMPONENTS_LIBRARY_CONTAINER_H
#define SAYONARA_PLAYER_COMPONENTS_LIBRARY_CONTAINER_H

class QWidget;
class QFrame;
class QIcon;
class QMenu;
class QAction;

#include <QObject>

namespace Library
{
	class LibraryContainer
	{
		public:
			virtual ~LibraryContainer() = default;
			[[nodiscard]] virtual QString name() const = 0;

			virtual void rename(const QString& newName) = 0;

			[[nodiscard]] virtual QString displayName() const = 0;

			[[nodiscard]] virtual QWidget* widget() const = 0;

			[[nodiscard]] virtual QFrame* header() const = 0;

			[[nodiscard]] virtual QMenu* menu() = 0;

			[[nodiscard]] virtual QIcon icon() const = 0;

			virtual void init() = 0;

			[[nodiscard]] virtual bool isLocal() const = 0;
	};
}

using LibraryContainerInterface = Library::LibraryContainer;

Q_DECLARE_INTERFACE(LibraryContainerInterface, "com.sayonara-player.library")

#endif // SAYONARA_PLAYER_COMPONENTS_LIBRARY_CONTAINER_H
