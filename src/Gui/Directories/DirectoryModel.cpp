#include "DirectoryModel.h"
#include "DirectoryIconProvider.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QFileSystemModel>

struct DirectoryModel::Private
{
	QFileSystemModel* model=nullptr;
	LibraryId libraryId;

	Private() :
		libraryId(-1)
	{}
};

DirectoryModel::DirectoryModel(QObject* parent) :
	QSortFilterProxyModel(parent)
{
	m = Pimpl::make<Private>();

	m->model = new QFileSystemModel(parent);
	m->model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
	m->model->setIconProvider(new IconProvider());

	this->setSourceModel(m->model);
}

DirectoryModel::~DirectoryModel() = default;

QModelIndex DirectoryModel::setDataSource(LibraryId libraryId)
{
	Library::Info info = Library::Manager::instance()->libraryInfo(libraryId);
	QModelIndex index = setDataSource(info.path());

	m->libraryId = libraryId;

	return index;
}

QModelIndex DirectoryModel::setDataSource(const QString& path)
{
	m->libraryId = -1;
	m->model->setRootPath(path);

	QModelIndex sourceIndex = m->model->index(path);
	return this->mapFromSource(sourceIndex);
}

LibraryId DirectoryModel::libraryDataSource() const
{
	return m->libraryId;
}

QString DirectoryModel::filePath(const QModelIndex& index)
{
	return m->model->filePath( mapToSource(index) );
}

QModelIndex DirectoryModel::indexOfPath(const QString& path) const
{
	return mapFromSource(m->model->index(path));
}

bool DirectoryModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	if(this->filterRegExp().pattern().isEmpty()){
		return true;
	}

	QModelIndex index = m->model->index(sourceRow, 0, sourceParent);
	QString path = m->model->filePath(index);
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
