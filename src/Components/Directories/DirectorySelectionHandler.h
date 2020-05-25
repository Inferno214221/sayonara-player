#ifndef DIRECTORYSELECTIONHANDLER_H
#define DIRECTORYSELECTIONHANDLER_H

#include "Utils/Pimpl.h"
#include <QObject>

namespace Library
{
	class Info;
}

class FileOperations;
class LocalLibrary;
class QStringList;

class DirectorySelectionHandler :
	public QObject
{
	Q_OBJECT
	PIMPL(DirectorySelectionHandler)

	signals:
		void sigLibrariesChanged();
		void sigImportDialogRequested(const QString& targetPath);
		void sigFileOperationStarted();
		void sigFileOperationFinished();

	private:
		FileOperations* createFileOperation();

	public:
		DirectorySelectionHandler(QObject* parent = nullptr);
		~DirectorySelectionHandler();

		void playNext(const QStringList& paths);
		void createPlaylist(const QStringList& paths, bool createNewPlaylist);
		void appendTracks(const QStringList& paths);
		void prepareTracksForPlaylist(const QStringList& paths, bool createNewPlaylist);

		void requestImport(LibraryId libId, const QStringList& paths, const QString& targetDirectory);

		void setLibraryId(LibraryId libId);
		LibraryId libraryId() const;

		void createNewLibrary(const QString& name, const QString& path);

		Library::Info libraryInfo() const;
		LocalLibrary* libraryInstance() const;

		void setSearchText(const QString& text);

		void copyPaths(const QStringList& paths, const QString& target);
		void movePaths(const QStringList& paths, const QString& target);
		void renamePath(const QString& path, const QString& newName);
		void renameByExpression(const QString& path, const QString& expression);
		void deletePaths(const QStringList& paths);

	private slots:
		void librariesChanged();
};

#endif // DIRECTORYSELECTIONHANDLER_H
