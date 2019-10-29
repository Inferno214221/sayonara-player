#include "ConnectorProvider.h"
#include "Connector.h"

using namespace DB;

struct DB::ConnectorConsumer::Private
{
	DB::ConnectorProvider* p=nullptr;
};


DB::ConnectorConsumer::ConnectorConsumer()
{
	m = Pimpl::make<Private>();
}

DB::ConnectorConsumer::~ConnectorConsumer() = default;

void DB::ConnectorConsumer::register_db_connector_provider(ConnectorProvider* p)
{
	m->p = p;
}

void DB::ConnectorConsumer::setup_databases() {}

DB::Connector* DB::ConnectorConsumer::db_connector() const
{
	if(m->p){
		return m->p->get_connector();
	}

	return DB::Connector::instance();
}
