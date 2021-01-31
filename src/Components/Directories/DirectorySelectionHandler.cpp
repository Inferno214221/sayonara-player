#include "DirectorySelectionHandler.h"
#include "FileOperations.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/ExternTracksPlaylistGenerator.h"

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
	public:
		LocalLibrary* genericLibrary;
		Playlist::Handler* playlistHandler;
		QList<Library::Info> libraries;
		int currentLibraryIndex;

		Private() :
			genericLibrary {nullptr},
			playlistHandler {Playlist::HandlerProvider::instance()->handler()},
			libraries {Library::Manager::instance()->allLibraries()},
			currentLibraryIndex {(libraries.count() > 0) ? 0 : -1} {}
};

DirectorySelectionHandler::DirectorySelectionHandler(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	auto* libraryManager = Library::Manager::instance();
	connect(libraryManager, &Library::Manager::sigAdded, this, [&](auto ignore) {
		Q_UNUSED(ignore)
		librariesChanged();
	});

	connect(libraryManager, &Library::Manager::sigRemoved, this, [&](auto ignore) {
		Q_UNUSED(ignore)
		librariesChanged();
	});

	connect(libraryManager, &Library::Manager::sigMoved, this, [&](auto i1, auto i2, auto i3) {
		Q_UNUSED(i1)
		Q_UNUSED(i2)
		Q_UNUSED(i3)
		librariesChanged();
	});

	connect(libraryManager, &Library::Manager::sigRenamed, this, [&](auto ignore) {
		Q_UNUSED(ignore)
		librariesChanged();
	});

	connect(libraryManager, &Library::Manager::sigPathChanged, this, [&](auto ignore) {
		Q_UNUSED(ignore)
		librariesChanged();
	});
}

DirectorySelectionHandler::~DirectorySelectionHandler() = default;

void DirectorySelectionHandler::createPlaylist(const QStringList& paths, bool createNewPlaylist)
{
	const auto newName = (createNewPlaylist)
	                     ? m->playlistHandler->requestNewPlaylistName()
	                     : QString();

	m->playlistHandler->createPlaylist(paths, newName);
}

void DirectorySelectionHandler::playNext(const QStringList& paths)
{
	auto playlist = m->playlistHandler->activePlaylist();

	auto* playlistGenerator = new ExternTracksPlaylistGenerator(m->playlistHandler, playlist);
	connect(playlistGenerator, &ExternTracksPlaylistGenerator::sigFinished, playlistGenerator, &QObject::deleteLater);
	playlistGenerator->insertPaths(paths, playlist->currentTrackIndex());
}

void DirectorySelectionHandler::appendTracks(const QStringList& paths)
{
	auto playlist = m->playlistHandler->activePlaylist();

	auto* playlistGenerator = new ExternTracksPlaylistGenerator(m->playlistHandler, playlist);
	connect(playlistGenerator, &ExternTracksPlaylistGenerator::sigFinished, playlistGenerator, &QObject::deleteLater);
	playlistGenerator->addPaths(paths);
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
	auto* fo = new FileOperations(this);

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

	m->libraries = Library::Manager::instance()->allLibraries();
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
	Library::Manager::instance()->addLibrary(name, path);
}

Library::Info DirectorySelectionHandler::libraryInfo() const
{
	return (Util::between(m->currentLibraryIndex, m->libraries))
	       ? m->libraries[m->currentLibraryIndex]
	       : Library::Info();
}

LocalLibrary* DirectorySelectionHandler::libraryInstance() const
{
	auto* manager = Library::Manager::instance();

	const auto libraryId = libraryInfo().id();
	auto* library = manager->libraryInstance(libraryId);

	if(library == nullptr)
	{
		if(!m->genericLibrary)
		{
			m->genericLibrary = Library::Manager::instance()->libraryInstance(-1);
		}

		spLog(Log::Warning, this) << "Invalid library index";
		return m->genericLibrary;
	}

	return library;
}

void DirectorySelectionHandler::setSearchText(const QString& text)
{
	Library::Filter filter;
	filter.setFiltertext(text, GetSetting(Set::Lib_SearchMode));
	filter.setMode(Library::Filter::Mode::Filename);

	libraryInstance()->changeFilter(filter);
}

