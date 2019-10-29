#ifndef CONNECTORPROVIDER_H
#define CONNECTORPROVIDER_H

#include "Utils/Pimpl.h"

namespace DB
{
	class Connector;

	/**
	 * @brief This interface provides a method
	 * to fetch a connector. For tests, it's often
	 * important, that the DB::Connector::instance()
	 * is not called directly, since tests bring their
	 * own db. So a test should implement this class
	 * and offer its own DB::Connector by calling
	 * DB::Connector::instance_custom
	 * @ingroup Database
	 */
	class ConnectorProvider
	{
		public:
			virtual DB::Connector* get_connector() const=0;
	};

	/**
	 * @brief A class that should be tested but which
	 * uses the database should derive from this class
	 * and only access the database via the db_connector()
	 * method
	 * @ingroup Database
	 */
	class ConnectorConsumer
	{
		PIMPL(ConnectorConsumer)

		protected:
			DB::Connector* db_connector() const;

		public:
			ConnectorConsumer();
			virtual ~ConnectorConsumer();

			void register_db_connector_provider(ConnectorProvider* p);
			virtual void setup_databases();
	};
}

#endif // CONNECTORPROVIDER_H
