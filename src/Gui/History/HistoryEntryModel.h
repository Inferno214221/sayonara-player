#ifndef HISTORYENTRYMODEL_H
#define HISTORYENTRYMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

class HistoryEntryModel :
	public QAbstractTableModel
{
	Q_OBJECT
	PIMPL(HistoryEntryModel)

	signals:
		void sigRowsAdded();

	private:
		const Session::Entry& entry(int row) const;

	public:
		HistoryEntryModel(Session::Timecode timecode, QObject* parent=nullptr);
		~HistoryEntryModel() override;

		// QAbstractItemModel interface
	public:
		QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		int rowCount(const QModelIndex& parent) const override;
		int columnCount(const QModelIndex& parent) const override;
		QVariant data(const QModelIndex& index, int role) const override;

		Qt::ItemFlags flags(const QModelIndex& index) const override;
		QMimeData* mimeData(const QModelIndexList& indexes) const override;

	private slots:
		void historyChanged(Session::Id id);

	protected:
		void languageChanged();
};

#endif // HISTORYENTRYMODEL_H
