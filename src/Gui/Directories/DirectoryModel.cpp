#include "DirectoryModel.h"
#include "DirectoryIconProvider.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QFileSystemModel>
#include <QTimer>

using Directory::Model;

struct Model::Private
{
	QFileSystemModel* model=nullptr;
	QTimer*	filterTimer=nullptr;
	QString	filter;

	LibraryId libraryId;

	Private() :
		libraryId(-1)
	{}
};

Model::Model(QObject* parent) :
	QSortFilterProxyModel(parent)
{
	m = Pimpl::make<Private>();

	m->model = new QFileSystemModel(parent);
	m->model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
	m->model->setIconProvider(new IconProvider());

	this->setSourceModel(m->model);
}

Model::~Model() = default;

QModelIndex Model::setDataSource(LibraryId libraryId)
{
	Library::Info info = Library::Manager::instance()->libraryInfo(libraryId);
	QModelIndex index = setDataSource(info.path());

	m->libraryId = libraryId;

	return index;
}

QModelIndex Model::setDataSource(const QString& path)
{
	m->libraryId = -1;
	m->model->setRootPath(path);

	QModelIndex sourceIndex = m->model->index(path);
	return this->mapFromSource(sourceIndex);
}

LibraryId Model::libraryDataSource() const
{
	return m->libraryId;
}

QString Model::filePath(const QModelIndex& index)
{
	return m->model->filePath( mapToSource(index) );
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
	if(this->filterRegExp().pattern().isEmpty()){
		return true;
	}

	const QModelIndex index = m->model->index(sourceRow, 0, sourceParent);
	const QString path = m->model->filePath(index);
	if(Util::File::isSubdir(m->model->rootPath(), path) || Util::File::isSamePath(m->model->rootPath(), path))
	{
		return true;
	}

	if(m->libraryId >= 0)
	{
		LocalLibrary* ll = Library::Manager::instance()->libraryInstance(m->libraryId);
		const MetaDataList& tracks = ll->tracks();
		for(const MetaData& md : tracks)
		{
			if(Util::File::isSubdir(md.filepath(), path)) {
				return true;
			}
		}

		return false;
	}

	return true;
}


int Model::columnCount(const QModelIndex& /*parent*/) const
{
	return 1;
}
