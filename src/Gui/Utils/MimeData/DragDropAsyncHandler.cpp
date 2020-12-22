#include "DragDropAsyncHandler.h"
#include "Utils/MetaData/MetaDataList.h"

using Gui::AsyncDropHandler;

struct Gui::AsyncDropHandler::Private
{
	MetaDataList tracks;
	int targetIndex;

	Private() :
		targetIndex(-1) {}
};

AsyncDropHandler::AsyncDropHandler(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

AsyncDropHandler::~AsyncDropHandler() = default;

void AsyncDropHandler::setTargetIndex(int index)
{
	m->targetIndex = index;
}

int AsyncDropHandler::targetIndex() const
{
	return m->targetIndex;
}

void Gui::AsyncDropHandler::setTracks(const MetaDataList& tracks)
{
	m->tracks = tracks;
	emit sigFinished();
}

MetaDataList Gui::AsyncDropHandler::tracks() const
{
	return m->tracks;
}
