#ifndef HISTORYENTRYMODEL_H
#define HISTORYENTRYMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

namespace Session
{
	class Manager;
}

class MetaDataList;

class HistoryEntryModel :
	public QAbstractTableModel
{
	Q_OBJECT
	PIMPL(HistoryEntryModel)

	signals:
		void sigRowsAdded();

	public:
		HistoryEntryModel(Session::Manager* sessionManager, Session::Timecode timecode, QObject* parent = nullptr);
		~HistoryEntryModel() override;

		[[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		[[nodiscard]] int rowCount(const QModelIndex& parent) const override;
		[[nodiscard]] int columnCount(const QModelIndex& parent) const override;
		[[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
		[[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
		[[nodiscard]] QMimeData* mimeData(const QModelIndexList& indexes) const override;
		[[nodiscard]] MetaDataList tracksByIndexes(const QModelIndexList& indexes) const;

	private:
		[[nodiscard]] const Session::Entry& entry(int row) const;

	private slots: // NOLINT(readability-redundant-access-specifiers)
		void historyChanged(Session::Id id);

	protected:
		void languageChanged();
};

#endif // HISTORYENTRYMODEL_H
