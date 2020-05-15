#include "Splitter.h"
#include "Gui/Utils/EventFilter.h"
#include <QMouseEvent>

struct Gui::Splitter::Private
{
	bool handleEnabled;

	Private() :
		handleEnabled(true)
	{}
};

Gui::Splitter::Splitter(QWidget* parent) :
	QSplitter(parent)
{
	m = Pimpl::make<Private>();
}

Gui::Splitter::~Splitter() = default;

void Gui::Splitter::setHandleEnabled(bool b)
{
	m->handleEnabled = b;
}

bool Gui::Splitter::isHandleEnabled() const
{
	return m->handleEnabled;
}

QSplitterHandle* Gui::Splitter::createHandle()
{
	return new Gui::SplitterHandle(this->orientation(), this);
}

void Gui::SplitterHandle::mouseMoveEvent(QMouseEvent* e)
{
	auto* splitter = dynamic_cast<Gui::Splitter*>(this->splitter());
	if(splitter && !splitter->isHandleEnabled())
	{
		return;
	}

	else
	{
		QSplitterHandle::mouseMoveEvent(e);
	}
}
