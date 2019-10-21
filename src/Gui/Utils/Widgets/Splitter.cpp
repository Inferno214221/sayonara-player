#include "Splitter.h"
#include <QMouseEvent>

struct Gui::Splitter::Private
{
	bool move_allowed;

	Private() :
		move_allowed(true)
	{}
};

Gui::Splitter::Splitter(QWidget* parent) :
	QSplitter(parent)
{
	m = Pimpl::make<Private>();
}

Gui::Splitter::~Splitter() = default;

void Gui::Splitter::set_handle_enabled(bool b)
{
	m->move_allowed = b;
}

bool Gui::Splitter::is_handle_enabled() const
{
	return m->move_allowed;
}

QSplitterHandle* Gui::Splitter::createHandle()
{
	return new Gui::SplitterHandle(this->orientation(), this);
}

void Gui::SplitterHandle::mouseMoveEvent(QMouseEvent* e)
{
	auto* splitter = dynamic_cast<Gui::Splitter*>(this->splitter());
	if(splitter && !splitter->is_handle_enabled())
	{
		return;
	}

	else
	{
		QSplitterHandle::mouseMoveEvent(e);
	}
}
