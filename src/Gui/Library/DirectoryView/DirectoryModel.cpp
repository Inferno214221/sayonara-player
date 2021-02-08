#include "DirectoryModel.h"
#include "DirectoryIconProvider.h"

#include "Components/Covers/LocalCoverSearcher.h"
#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Gui/Utils/MimeData/CustomMimeData.h"
#include "Gui/Utils/MimeData/MimeDataUtils.h"

#include <QFileSystemModel>
#include <QTimer>
#include <QUrl>

using Directory::Model;

struct Model::Private
{
	Library::Manager* libraryManager;
	QFileSystemModel* model = nullptr;
	QTimer* filterTimer = nullptr;
	QString filter;

	LibraryId libraryId;

	Private(Library::Manager* libraryManager, QObject* parent) :
		libraryManager {libraryManager},
		model {new QFileSystemModel(parent)},
		libraryId(-1)
	{
		model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
		model->setIconProvider(new IconProvider());
	}
};

Model::Model(Library::Manager* libraryManager, QObject* parent) :
	QSortFilterProxyModel(parent)
{
	m = Pimpl::make<Private>(libraryManager, parent);

	this->setSourceModel(m->model);
}

Model::~Model() = default;

QModelIndex Model::setDataSource(LibraryId libraryId)
{
	const auto info = m->libraryManager->libraryInfo(libraryId);
	const auto index = setDataSource(info.path());

	m->libraryId = libraryId;

	return index;
}

QModelIndex Model::setDataSource(const QString& path)
{
	m->libraryId = -1;
	m->model->setRootPath(path);

	const auto sourceIndex = m->model->index(path);

	return Util::File::exists(path)
	       ? this->mapFromSource(sourceIndex)
	       : QModelIndex();
}

LibraryId Model::libraryDataSource() const
{
	return m->libraryId;
}

QString Model::filePath(const QModelIndex& index) const
{
	return m->model->filePath(mapToSource(index));
}

QModelIndex Model::indexOfPath(const QString& path) const
{
	return mapFromSource(m->model->index(path));
}

void Model::setFilter(const QString& filter)
{
	if(!m->filterTimer)
	{
		m->filterTimer = new QTimer();
		m->filterTimer->setInterval(333);
		m->filterTimer->setSingleShot(true);

		connect(m->filterTimer, &QTimer::timeout, this, &Model::filterTimerTimeout);
	}

	m->filter = filter;
	m->filterTimer->start();

	emit sigBusy(true);
}

void Model::filterTimerTimeout()
{
	this->setFilterFixedString(m->filter);
	emit sigBusy(false);
}

bool Model::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	if(this->filterRegExp().pattern().isEmpty())
	{
		return true;
	}

	const auto index = m->model->index(sourceRow, 0, sourceParent);
	const auto path = m->model->filePath(index);
	if(Util::File::isSubdir(m->model->rootPath(), path) ||
	   Util::File::isSamePath(m->model->rootPath(), path))
	{
		return true;
	}

	if(m->libraryId >= 0)
	{
		auto* localLibrary = m->libraryManager->libraryInstance(m->libraryId);
		const auto& tracks = localLibrary->tracks();

		const auto contains = Util::Algorithm::contains(tracks, [&](const auto& track) {
			return Util::File::isSubdir(track.filepath(), path);
		});

		return contains;
	}

	return true;
}

int Model::columnCount(const QModelIndex& /*parent*/) const
{
	return 1;
}

QMimeData* Directory::Model::mimeData(const QModelIndexList& indexes) const
{
	QStringList paths;
	QList<QUrl> urls;
	for(const QModelIndex& index : indexes)
	{
		const auto path = this->filePath(index);
		paths << path;
		urls << QUrl::fromLocalFile(path);
	}

	auto* cmd = new Gui::CustomMimeData(this);
	if(!paths.isEmpty())
	{
		const auto coverPaths = Cover::LocalSearcher::coverPathsFromPathHint(paths.first());
		if(!coverPaths.isEmpty())
		{
			cmd->setCoverUrl(coverPaths.first());
		}
	}

	cmd->setUrls(urls);
	return cmd;
}
