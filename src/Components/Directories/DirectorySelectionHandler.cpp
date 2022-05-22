#include "DirectorySelectionHandler.h"
#include "FileOperations.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/Filter.h"

#include <QThread>
#include <QList>

struct DirectorySelectionHandler::Private
{
	LocalLibrary* genericLibrary;
	Library::Manager* libraryManager;
	QList<Library::Info> libraries;
	int currentLibraryIndex;

	Private(Library::Manager* libraryManager) :
		genericLibrary {nullptr},
		libraryManager {libraryManager},
		libraries {libraryManager->allLibraries()},
		currentLibraryIndex {(libraries.count() > 0) ? 0 : -1} {}
};

DirectorySelectionHandler::DirectorySelectionHandler(Library::Manager* libraryManager, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(libraryManager);

	connect(m->libraryManager, &Library::Manager::sigAdded, this, [&](auto /* id */) {
		librariesChanged();
	});

	connect(m->libraryManager, &Library::Manager::sigRemoved, this, [&](auto /* id */) {
		librariesChanged();
	});

	connect(m->libraryManager, &Library::Manager::sigMoved, this, [&](auto /* id */, auto /* from */, auto /* to */) {
		librariesChanged();
	});

	connect(m->libraryManager, &Library::Manager::sigRenamed, this, [&](auto /* id */) {
		librariesChanged();
	});

	connect(m->libraryManager, &Library::Manager::sigPathChanged, this, [&](auto /* id */) {
		librariesChanged();
	});
}

DirectorySelectionHandler::~DirectorySelectionHandler() = default;

void DirectorySelectionHandler::createPlaylist(const QStringList& paths, bool createNewPlaylist)
{
	this->libraryInstance()->prepareTracksForPlaylist(paths, createNewPlaylist);
}

void DirectorySelectionHandler::playNext([[maybe_unused]] const QStringList& paths)
{
	this->libraryInstance()->playNextFetchedTracks();
}

void DirectorySelectionHandler::appendTracks([[maybe_unused]] const QStringList& paths)
{
	this->libraryInstance()->appendFetchedTracks();
}

void DirectorySelectionHandler::prepareTracksForPlaylist(const QStringList& paths, bool createNewPlaylist)
{
	this->libraryInstance()->prepareTracksForPlaylist(paths, createNewPlaylist);
}

void
DirectorySelectionHandler::requestImport(LibraryId libraryId, const QStringList& paths, const QString& targetDirectory)
{
	if(libraryId != this->libraryId() || libraryId < 0)
	{
		return;
	}

	auto* library = this->libraryInstance();
	connect(library, &LocalLibrary::sigImportDialogRequested,
	        this, &DirectorySelectionHandler::sigImportDialogRequested);

	// prepare import
	library->importFilesTo(paths, targetDirectory);
}

FileOperations* DirectorySelectionHandler::createFileOperation()
{
	auto* fo = new FileOperations(m->libraryManager, this);

	connect(fo, &FileOperations::sigStarted, this, &DirectorySelectionHandler::sigFileOperationStarted);
	connect(fo, &FileOperations::sigFinished, this, &DirectorySelectionHandler::sigFileOperationFinished);
	connect(fo, &FileOperations::sigFinished, fo, &QObject::deleteLater);

	return fo;
}

void DirectorySelectionHandler::copyPaths(const QStringList& paths, const QString& targetDirectory)
{
	createFileOperation()->copyPaths(paths, targetDirectory);
}

void DirectorySelectionHandler::movePaths(const QStringList& paths, const QString& targetDirectory)
{
	createFileOperation()->movePaths(paths, targetDirectory);
}

void DirectorySelectionHandler::renamePath(const QString& path, const QString& newName)
{
	createFileOperation()->renamePath(path, newName);
}

void DirectorySelectionHandler::renameByExpression(const QString& path, const QString& expression)
{
	createFileOperation()->renameByExpression(path, expression);
}

void DirectorySelectionHandler::deletePaths(const QStringList& paths)
{
	createFileOperation()->deletePaths(paths);
}

void DirectorySelectionHandler::librariesChanged()
{
	const auto id = libraryId();

	m->libraries = m->libraryManager->allLibraries();
	m->currentLibraryIndex = Util::Algorithm::indexOf(m->libraries, [&id](const Library::Info& info) {
		return (info.id() == id);
	});

	emit sigLibrariesChanged();
}

void DirectorySelectionHandler::setLibraryId(LibraryId libId)
{
	m->currentLibraryIndex = Util::Algorithm::indexOf(m->libraries, [&libId](const Library::Info& info) {
		return (info.id() == libId);
	});
}

LibraryId DirectorySelectionHandler::libraryId() const
{
	return libraryInfo().id();
}

void DirectorySelectionHandler::createNewLibrary(const QString& name, const QString& path)
{
	m->libraryManager->addLibrary(name, path);
}

Library::Info DirectorySelectionHandler::libraryInfo() const
{
	return (Util::between(m->currentLibraryIndex, m->libraries))
	       ? m->libraries[m->currentLibraryIndex]
	       : Library::Info();
}

LocalLibrary* DirectorySelectionHandler::libraryInstance() const
{
	const auto libraryId = libraryInfo().id();
	auto* library = m->libraryManager->libraryInstance(libraryId);

	if(library == nullptr)
	{
		if(!m->genericLibrary)
		{
			m->genericLibrary = m->libraryManager->libraryInstance(-1);
		}

		spLog(Log::Warning, this) << "Invalid library index";
		return m->genericLibrary;
	}

	return library;
}

void DirectorySelectionHandler::setSearchText(const QString& text)
{
	Library::Filter filter;
	filter.setFiltertext(text);
	filter.setMode(Library::Filter::Mode::Filename);

	libraryInstance()->changeFilter(filter);
}

