#include "DirectorySelectionHandler.h"
#include "MetaDataScanner.h"
#include "FileOperations.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Components/Playlist/PlaylistHandler.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
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
	LocalLibrary*			genericLibrary=nullptr;

	int						currentLibraryIndex;
	QList<Library::Info>	libraries;

	Private()
	{
		libraries = Library::Manager::instance()->allLibraries();
		currentLibraryIndex = (libraries.count() > 0) ? 0 : -1;
	}
};

DirectorySelectionHandler::DirectorySelectionHandler(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	auto* libraryManager = Library::Manager::instance();
	connect(libraryManager, &Library::Manager::sigAdded, this, [this](auto ignore)
	{
		Q_UNUSED(ignore)
		librariesChanged();
	});

	connect(libraryManager, &Library::Manager::sigRemoved, this, [this](auto ignore)
	{
		Q_UNUSED(ignore)
		librariesChanged();
	});

	connect(libraryManager, &Library::Manager::sigMoved, this, [this](auto i1, auto i2, auto i3)
	{
		Q_UNUSED(i1)
		Q_UNUSED(i2)
		Q_UNUSED(i3)
		librariesChanged();
	});

	connect(libraryManager, &Library::Manager::sigRenamed, this, [this](auto ignore)
	{
		Q_UNUSED(ignore)
		librariesChanged();
	});
}

DirectorySelectionHandler::~DirectorySelectionHandler() = default;

void DirectorySelectionHandler::createPlaylist(const QStringList& paths, bool createNewPlaylist)
{
	auto* plh = Playlist::Handler::instance();

	if(createNewPlaylist) {
		plh->createPlaylist(paths, plh->requestNewPlaylistName());
	}

	else {
		plh->createPlaylist(paths);
	}
}

void DirectorySelectionHandler::playNext(const QStringList& paths)
{
	auto* plh = Playlist::Handler::instance();
	plh->playNext(paths);
}

void DirectorySelectionHandler::appendTracks(const QStringList& paths)
{
	auto* plh = Playlist::Handler::instance();
	plh->appendTracks(paths, plh->current_index());
}

void DirectorySelectionHandler::prepareTracksForPlaylist(const QStringList& paths, bool createNewPlaylist)
{
	this->libraryInstance()->prepareTracksForPlaylist(paths, createNewPlaylist);
}

void DirectorySelectionHandler::requestImport(LibraryId libraryId, const QStringList& paths, const QString& targetDirectory)
{
	if(libraryId != this->libraryId() || libraryId < 0){
		return;
	}

	LocalLibrary* library = this->libraryInstance();
	connect(library, &LocalLibrary::sigImportDialogRequested,
			this, &DirectorySelectionHandler::sigImportDialogRequested);

	// prepare import
	library->importFilesTo(paths, targetDirectory);
}

FileOperations* DirectorySelectionHandler::create_file_operation()
{
	auto* fo = new FileOperations(this);

	connect(fo, &FileOperations::sigStarted, this, &DirectorySelectionHandler::sigFileOperationStarted);
	connect(fo, &FileOperations::sigFinished, this, &DirectorySelectionHandler::sigFileOperationFinished);
	connect(fo, &FileOperations::sigFinished, fo, &QObject::deleteLater);

	return fo;
}

void DirectorySelectionHandler::copyPaths(const QStringList& paths, const QString& targetDirectory)
{
	create_file_operation()->copyPaths(paths, targetDirectory);
}

void DirectorySelectionHandler::movePaths(const QStringList& paths, const QString& targetDirectory)
{
	create_file_operation()->movePaths(paths, targetDirectory);
}

void DirectorySelectionHandler::renamePath(const QString& path, const QString& new_name)
{
	create_file_operation()->renamePath(path, new_name);
}

void DirectorySelectionHandler::renameByExpression(const QString& path, const QString& expression)
{
	create_file_operation()->renameByExpression(path, expression);
}

void DirectorySelectionHandler::deletePaths(const QStringList& paths)
{
	create_file_operation()->deletePaths(paths);
}

void DirectorySelectionHandler::librariesChanged()
{
	LibraryId id = libraryId();
	m->libraries = Library::Manager::instance()->allLibraries();

	int index = Util::Algorithm::indexOf(m->libraries, [&id](const Library::Info& info){
		return (info.id() == id);
	});

	m->currentLibraryIndex = index;

	emit sigLibrariesChanged();
}

void DirectorySelectionHandler::setLibraryId(LibraryId lib_id)
{
	m->currentLibraryIndex = Util::Algorithm::indexOf(m->libraries, [&lib_id](const Library::Info& info){
		return (info.id() == lib_id);
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
	if(!Util::between(m->currentLibraryIndex, m->libraries))
	{
		return Library::Info();
	}

	return m->libraries[m->currentLibraryIndex];
}

LocalLibrary* DirectorySelectionHandler::libraryInstance() const
{
	auto* manager = Library::Manager::instance();
	LibraryId lib_id = libraryInfo().id();
	auto* library = manager->libraryInstance(lib_id);

	if(library == nullptr)
	{
		if(!m->genericLibrary){
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

