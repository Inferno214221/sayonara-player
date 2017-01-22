#include "AlbumCoverModel.h"
#include "Components/Covers/CoverLocation.h"
#include "Helper/MetaData/Album.h"
#include "Helper/Logger/Logger.h"

#include <QPixmap>
#include <QVector>


struct AlbumCoverModel::Private
{
	QList<CoverLocation> cover_locations;
	AlbumList albums;
	QHash<QString, QPixmap> pixmaps;

	int size;
	int columns;

	Private()
	{
		size = 100;
		columns = 10;
	}
};

static QString get_hash(const Album& album)
{
	return album.name + "-" + album.id;
}

AlbumCoverModel::AlbumCoverModel(QObject* parent) :
	LibraryItemModel()
{
	_m = Pimpl::make<Private>();
}

AlbumCoverModel::~AlbumCoverModel() {}


int AlbumCoverModel::rowCount(const QModelIndex& parent) const
{
	return (_m->albums.size() + columnCount() - 1 )/ columnCount();
}

int AlbumCoverModel::columnCount(const QModelIndex& parent) const
{
	return _m->columns;
}

void AlbumCoverModel::set_max_columns(int columns)
{
	_m->columns = columns;
}

QVariant AlbumCoverModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid()) {
		return QVariant();
	}

	int row = index.row();
	int col = index.column();

	int n_columns = columnCount();

	int idx = (row * n_columns) + col;
	if(idx >= _m->cover_locations.size()){
		return QVariant();
	}

	switch(role)
	{
		case Qt::DisplayRole:
			return _m->albums[idx].name;

		case Qt::DecorationRole:
			{
				QPixmap p;
				QString hash = get_hash(_m->albums[idx]);

				if( !_m->pixmaps.contains(hash) )
				{
					const CoverLocation& cl = _m->cover_locations[idx];
					QString preferred = cl.preferred_path();
					p = QPixmap(preferred).scaled(_m->size, _m->size, Qt::KeepAspectRatio);
					if(cl.valid() && !CoverLocation::isInvalidLocation(cl.preferred_path())){
						_m->pixmaps[hash] = p;
					}
				}

				else {
					p = _m->pixmaps[hash];
				}

				return p;
			}

		case Qt::SizeHintRole:
			return QSize(_m->size + 100, _m->size + 10);

		default:
			return QVariant();
	}

	return QVariant();
}


void AlbumCoverModel::set_data(const AlbumList& albums, const QList<CoverLocation>& cover_locations)
{
	beginRemoveRows(QModelIndex(), 0, rowCount());
	endRemoveRows();

	beginRemoveColumns(QModelIndex(), 0, columnCount());
	endRemoveColumns();

	_m->cover_locations = cover_locations;
	_m->albums = albums;

	beginInsertRows(QModelIndex(), 0, rowCount());
	endInsertRows();

	beginInsertColumns(QModelIndex(), 0, columnCount());
	endInsertColumns();

	emit dataChanged(index(0, 0),
					 index(rowCount(), columnCount())
	);
}


QModelIndex AlbumCoverModel::getFirstRowIndexOf(const QString& substr)
{
	return QModelIndex();
}

QModelIndex AlbumCoverModel::getNextRowIndexOf(const QString& substr, int cur_row, const QModelIndex& parent)
{
	return QModelIndex();
}

QModelIndex AlbumCoverModel::getPrevRowIndexOf(const QString& substr, int cur_row, const QModelIndex& parent)
{
	return QModelIndex();
}

QMap<QChar, QString> AlbumCoverModel::getExtraTriggers()
{
	return QMap<QChar, QString>();
}


int AlbumCoverModel::get_searchable_column() const
{
	return 0;
}

QString AlbumCoverModel::get_string(int row) const
{
	return QString();
}

int AlbumCoverModel::get_id_by_row(int row)
{
	return -1;
}

CoverLocation AlbumCoverModel::get_cover(const SP::Set<int>& indexes) const
{
	return CoverLocation::getInvalidLocation();
}