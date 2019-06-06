#ifndef LIBRARYWATCHER_H
#define LIBRARYWATCHER_H

#include "Utils/typedefs.h"
#include "Utils/Pimpl.h"

#include <QObject>
#include <QList>

namespace Library
{
	class Container;
	class LocalLibraryWatcher : public QObject
	{
		Q_OBJECT
		PIMPL(LocalLibraryWatcher)

		public:
			explicit LocalLibraryWatcher(QObject* parent=nullptr);
			~LocalLibraryWatcher();

			QList<Container*> get_local_library_containers() const;

		private slots:
			void library_added(LibraryId id);
			void library_moved(LibraryId id, int from, int to);
			void library_renamed(LibraryId id);
			void library_removed(LibraryId id);
	};
}

#endif // LIBRARYWATCHER_H
