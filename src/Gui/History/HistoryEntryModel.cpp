#include "HistoryEntryModel.h"
#include "Components/Session/Session.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/Algorithm.h"
#include "Utils/CustomMimeData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

struct HistoryEntryModel::Private
{
	Session::Manager* session=nullptr;
	Session::EntryList history;
	Session::Entry invalidEntry;

	Session::Timecode timecode;

	Private(Session::Timecode timecode) :
		timecode(timecode)
	{
		session = Session::Manager::instance();
		invalidEntry.timecode = 0;

		calc_history();
	}

	void calc_history()
	{
		history.clear();

		const QDateTime dt = Util::intToDate(this->timecode);
		const Session::EntryListMap map = session->historyForDay(dt);
		const QList<Session::Timecode> keys = map.keys();

		for(Session::Timecode t : keys)
		{
			Session::EntryList lst = map[t];
			Util::Algorithm::remove_duplicates(lst);

			history << lst;
		}
	}
};

const Session::Entry& HistoryEntryModel::entry(int row) const
{
	int index = (m->history.size() - 1) - row;
	if(index < 0 || index >= m->history.size()){
		return m->invalidEntry;
	}

	return m->history[index];
}

HistoryEntryModel::HistoryEntryModel(Session::Timecode timecode, QObject* parent) :
	QAbstractTableModel(parent)
{
	m = Pimpl::make<Private>(timecode);

	connect(m->session, &Session::Manager::sigSessionChanged, this, &HistoryEntryModel::historyChanged);

	ListenSetting(Set::Player_Language, HistoryEntryModel::languageChanged);
}

HistoryEntryModel::~HistoryEntryModel() = default;

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
	if(role != Qt::DisplayRole){
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
			return e.md.title();
		case 2:
			return e.md.artist();
		case 3:
			return e.md.album();
		default:
			return QVariant();
	}
}

void HistoryEntryModel::languageChanged()
{
	emit headerDataChanged(Qt::Orientation::Horizontal, 0, columnCount());
}

void HistoryEntryModel::historyChanged(Session::Id id)
{
	Session::Timecode min_id = Session::dayBegin(id);
	Session::Timecode max_id = Session::dayEnd(id);

	if(id >= min_id && id <= max_id)
	{
		int old_rowcount = rowCount();

		m->calc_history();

		if(rowCount() > old_rowcount)
		{
			beginInsertRows(QModelIndex(), old_rowcount, rowCount() - 1);
			this->insertRows(old_rowcount, (rowCount() - old_rowcount));
			endInsertRows();

			emit sigRowsAdded();
		}

		emit dataChanged(index(old_rowcount, 0), index(rowCount(), columnCount()), {Qt::DisplayRole});
	}
}

QVariant HistoryEntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Orientation::Vertical){
		return QVariant();
	}

	if(role != Qt::DisplayRole){
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
	if(e.md.filepath().isEmpty())
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
	for(const QModelIndex& index : indexes)
	{
		const auto& e = entry(index.row());

		if(e.timecode > 0 && !rows.contains(index.row()))
		{
			tracks << e.md;
			rows << index.row();
		}
	}

	data->setMetadata(tracks);

	return data;
}
