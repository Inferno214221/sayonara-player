#include "Mergable.h"
#include "Utils/Library/MergeData.h"
#include "Utils/Set.h"

#include <QMenu>
#include <QStringList>
#include <QMap>

struct MergeMenu::Private
{
	QAction*			action=nullptr;
	QMenu*				parent=nullptr;

	QMap<Id, QString>	data;
	Id					target_id;

	explicit Private(QMenu* parent) :
		parent(parent),
		target_id(-1)
	{}
};

MergeMenu::MergeMenu(QMenu* parent) :
	Gui::WidgetTemplate<QMenu>(parent)
{
	m = Pimpl::make<Private>(parent);

	m->action = m->parent->addMenu(this);
	m->action->setVisible(false);
	m->action->setText(tr("Merge"));
}

MergeMenu::~MergeMenu() = default;

void MergeMenu::set_data(const QMap<Id, QString>& data)
{
	if(data.size() < 2){
		return;
	}

	this->clear();

	m->data = data;
	for(Id key : data.keys())
	{
		QString val	= data[key];

		auto* action = new QAction(this);
		action->setText(val);
		action->setData(key);
		this->addAction(action);

		connect(action, &QAction::triggered, this, [=](){
			m->target_id = key;
			emit sig_merge_triggered();
		});
	}
}

QAction* MergeMenu::action() const
{
	return m->action;
}

bool MergeMenu::is_data_valid() const
{
	return (m->data.size() > 1);
}

MergeData MergeMenu::mergedata() const
{
	Util::Set<Id> source_ids;
	for(Id key : m->data.keys()){
		source_ids << key;
	}

	return MergeData(source_ids, m->target_id, -1);
}

void MergeMenu::language_changed()
{
	m->action->setText(tr("Merge"));
}
