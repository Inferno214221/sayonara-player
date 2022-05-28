#include "HistoryEntryModel.h"
#include "Components/Session/Session.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include "Utils/MimeData/CustomMimeData.h"

struct HistoryEntryModel::Private
{
	Session::Manager* session = nullptr;
	Session::EntryList history;
	Session::Entry invalidEntry;

	Session::Timecode timecode;

	Private(Session::Manager* sessionManager, Session::Timecode timecode) :
		session(sessionManager),
		timecode(timecode)
	{
		invalidEntry.timecode = 0;

		calcHistory();
	}

	void calcHistory()
	{
		history.clear();

		const QDateTime dt = Util::intToDate(this->timecode);
		const Session::EntryListMap map = session->historyForDay(dt);
		const QList<Session::Timecode> keys = map.keys();

		for(Session::Timecode t: keys)
		{
			Session::EntryList lst = map[t];
			Util::Algorithm::remove_duplicates(lst);

			history << lst;
		}
	}
};

HistoryEntryModel::HistoryEntryModel(Session::Manager* sessionManager, Session::Timecode timecode, QObject* parent) :
	QAbstractTableModel(parent)
{
	m = Pimpl::make<Private>(sessionManager, timecode);

	connect(m->session, &Session::Manager::sigSessionChanged, this, &HistoryEntryModel::historyChanged);

	ListenSetting(Set::Player_Language, HistoryEntryModel::languageChanged);
}

HistoryEntryModel::~HistoryEntryModel() = default;

const Session::Entry& HistoryEntryModel::entry(int row) const
{
	int index = (m->history.size() - 1) - row;
	if(index < 0 || index >= m->history.size())
	{
		return m->invalidEntry;
	}

	return m->history[index];
}

int HistoryEntryModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return m->history.count();
}

int HistoryEntryModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return 4;
}

QVariant HistoryEntryModel::data(const QModelIndex& index, int role) const
{
	if(role != Qt::DisplayRole)
	{
		return QVariant();
	}

	int row = index.row();
	int col = index.column();

	const Session::Entry& e = entry(row);

	switch(col)
	{
		case 0:
		{
			QDateTime dt = Util::intToDate(e.timecode);
			return " " + dt.time().toString() + " ";
		}

		case 1:
			return e.track.title();
		case 2:
			return e.track.artist();
		case 3:
			return e.track.album();
		default:
			return QVariant();
	}
}

void HistoryEntryModel::languageChanged()
{
	int columnCount = this->columnCount(QModelIndex());
	emit headerDataChanged(Qt::Orientation::Horizontal, 0, columnCount);
}

void HistoryEntryModel::historyChanged(Session::Id id)
{
	const Session::Timecode dayBegin = Session::dayBegin(id);
	const Session::Timecode dayEnd = Session::dayEnd(id);

	if(id >= dayBegin && id <= dayEnd)
	{
		const int oldRowCount = rowCount(QModelIndex());
		m->calcHistory();
		const int newRowCount = rowCount(QModelIndex());

		if(newRowCount > oldRowCount)
		{
			beginInsertRows(QModelIndex(), oldRowCount, newRowCount - 1);
			this->insertRows(oldRowCount, (newRowCount - oldRowCount));
			endInsertRows();

			emit sigRowsAdded();
		}

		const int columnCount = this->columnCount(QModelIndex());
		emit dataChanged(index(oldRowCount, 0), index(newRowCount, columnCount), {Qt::DisplayRole});
	}
}

QVariant HistoryEntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Orientation::Vertical)
	{
		return QVariant();
	}

	if(role != Qt::DisplayRole)
	{
		return QVariant();
	}

	switch(section)
	{
		case 0:
			return Lang::get(Lang::Date);
		case 1:
			return Lang::get(Lang::Title);
		case 2:
			return Lang::get(Lang::Artist);
		case 3:
			return Lang::get(Lang::Album);
		default:
			return QVariant();
	}
}

Qt::ItemFlags HistoryEntryModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = QAbstractTableModel::flags(index);

	const auto& e = entry(index.row());
	if(e.track.filepath().isEmpty())
	{
		f &= ~(Qt::ItemIsEnabled);
		f &= ~(Qt::ItemIsDragEnabled);
	}

	else
	{
		f |= Qt::ItemIsDragEnabled;
	}

	return f;
}

QMimeData* HistoryEntryModel::mimeData(const QModelIndexList& indexes) const
{
	auto* data = new Gui::CustomMimeData(this);

	Util::Set<int> rows;
	MetaDataList tracks;
	for(const QModelIndex& index: indexes)
	{
		const auto& e = entry(index.row());

		if(e.timecode > 0 && !rows.contains(index.row()))
		{
			tracks << e.track;
			rows << index.row();
		}
	}

	data->setMetadata(tracks);

	return data;
}
