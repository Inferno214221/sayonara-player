#ifndef LIBRARYWATCHER_H
#define LIBRARYWATCHER_H

#include "Utils/typedefs.h"
#include "Utils/Pimpl.h"

#include <QObject>
#include <QList>

namespace Library
{
	class Container;
	class LocalLibraryPluginHandler : public QObject
	{
		Q_OBJECT
		PIMPL(LocalLibraryPluginHandler)

		public:
			explicit LocalLibraryPluginHandler(QObject* parent=nullptr);
			~LocalLibraryPluginHandler();

			QList<Container*> get_local_library_containers() const;

		private slots:
			void library_added(LibraryId id);
			void library_moved(LibraryId id, int from, int to);
			void library_renamed(LibraryId id);
			void library_removed(LibraryId id);
	};
}

#endif // LIBRARYWATCHER_H
