#ifndef HISTORYCONTAINER_H
#define HISTORYCONTAINER_H

#include "Gui/Utils/Library/LibraryContainerImpl.h"
#include "Utils/Pimpl.h"

class HistoryContainer :
	public Library::ContainerImpl
{
	Q_OBJECT
	PIMPL(HistoryContainer)

public:
	HistoryContainer(QObject* parent=nullptr);
	~HistoryContainer() override;

	// Container interface
public:
	QString name() const override;
	QString display_name() const override;
	QWidget* widget() const override;
	QFrame* header() const override;
	QPixmap icon() const override;

	// ContainerImpl interface
protected:
	void init_ui() override;
};

#endif // HISTORYCONTAINER_H