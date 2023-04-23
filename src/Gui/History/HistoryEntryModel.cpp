#include "HistoryEntryModel.h"
#include "Components/Session/Session.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MimeData/CustomMimeData.h"
#include "Utils/Set.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

namespace
{
	Session::EntryList calcHistory(const Session::Timecode timecode, Session::Manager* sessionManager)
	{
		auto result = Session::EntryList {};

		const auto dt = Util::intToDate(timecode);
		const auto entryListMap = sessionManager->historyForDay(dt);
		const auto timecodes = entryListMap.keys();

		for(const auto& key: timecodes)
		{
			auto entries = entryListMap[key];
			Util::Algorithm::remove_duplicates(entries);

			result << entries;
		}

		return result;
	}
}

struct HistoryEntryModel::Private
{
	Session::Manager* session;
	Session::Timecode timecode;
	Session::EntryList history;
	Session::Entry invalidEntry;

	Private(Session::Manager* sessionManager, const Session::Timecode timecode) :
		session {sessionManager},
		timecode {timecode},
		history {calcHistory(timecode, sessionManager)}
	{
		invalidEntry.timecode = 0;
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

const Session::Entry& HistoryEntryModel::entry(const int row) const
{
	const auto index = (m->history.size() - 1) - row;
	return (index >= 0) && (index < m->history.size())
	       ? m->history[index]
	       : m->invalidEntry;
}

int HistoryEntryModel::rowCount(const QModelIndex& /*parent*/) const { return m->history.count(); }

int HistoryEntryModel::columnCount(const QModelIndex& /*parent*/) const { return 4; }

QVariant HistoryEntryModel::data(const QModelIndex& index, const int role) const
{
	if(role != Qt::DisplayRole)
	{
		return {};
	}

	const auto row = index.row();
	switch(index.column())
	{
		case 0:
		{
			const auto dt = Util::intToDate(entry(row).timecode);
			return QString(" %1 ").arg(dt.time().toString());
		}

		case 1:
			return entry(row).track.title();
		case 2:
			return entry(row).track.artist();
		case 3:
			return entry(row).track.album();
		default:
			return {};
	}
}

void HistoryEntryModel::languageChanged()
{
	emit headerDataChanged(Qt::Orientation::Horizontal, 0, columnCount({}));
}

void HistoryEntryModel::historyChanged(const Session::Id id)
{
	const auto dayBegin = Session::dayBegin(id);
	const auto dayEnd = Session::dayEnd(id);

	if((id >= dayBegin) && (id <= dayEnd))
	{
		const int oldRowCount = rowCount({});
		m->history = calcHistory(m->timecode, m->session);
		const int newRowCount = rowCount({});

		if(newRowCount > oldRowCount)
		{
			beginInsertRows({}, oldRowCount, newRowCount - 1);
			insertRows(oldRowCount, (newRowCount - oldRowCount));
			endInsertRows();

			emit sigRowsAdded();
		}

		emit dataChanged(index(oldRowCount, 0), index(newRowCount, columnCount({})), {Qt::DisplayRole});
	}
}

QVariant HistoryEntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Orientation::Vertical)
	{
		return {};
	}

	if(role != Qt::DisplayRole)
	{
		return {};
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
			return {};
	}
}

Qt::ItemFlags HistoryEntryModel::flags(const QModelIndex& index) const
{
	auto f = QAbstractTableModel::flags(index);

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

	auto rows = Util::Set<int> {};
	auto tracks = MetaDataList {};
	for(const auto& index: indexes)
	{
		const auto& e = entry(index.row());

		if((e.timecode > 0) && !rows.contains(index.row()))
		{
			tracks << e.track;
			rows << index.row();
		}
	}

	data->setMetadata(tracks);

	return data;
}
