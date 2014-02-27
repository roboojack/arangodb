////////////////////////////////////////////////////////////////////////////////
/// @brief wrapper for Aql transactions
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2013 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Jan Steemann
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_UTILS_AHUACATL_TRANSACTION_H
#define TRIAGENS_UTILS_AHUACATL_TRANSACTION_H 1

#ifdef TRI_ENABLE_CLUSTER
#include "Cluster/ServerState.h"
#endif

#include "Utils/CollectionNameResolver.h"
#include "Utils/Transaction.h"
#include "VocBase/server.h"
#include "VocBase/transaction.h"

struct TRI_vocbase_s;

namespace triagens {
  namespace arango {

    template<typename T>
    class AhuacatlTransaction : public Transaction<T> {

// -----------------------------------------------------------------------------
// --SECTION--                                         class AhuacatlTransaction
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create the transaction and add all collections from the query
/// context
////////////////////////////////////////////////////////////////////////////////

        AhuacatlTransaction (struct TRI_vocbase_s* const vocbase,
                             triagens::arango::CollectionNameResolver const& resolver,
                             TRI_aql_context_t* const context) :
          Transaction<T>(vocbase, TRI_GetIdServer(), resolver, false) {

          this->addHint(TRI_TRANSACTION_HINT_LOCK_ENTIRELY);

          TRI_vector_pointer_t* collections = &context->_collections;

          const size_t n = collections->_length;

          for (size_t i = 0; i < n; ++i) {
            TRI_aql_collection_t* collection = (TRI_aql_collection_t*) TRI_AtVectorPointer(collections, i);

            processCollection(resolver, collection);
          }
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief end the transaction
////////////////////////////////////////////////////////////////////////////////

        ~AhuacatlTransaction () {
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief add a collection to the transaction
////////////////////////////////////////////////////////////////////////////////
        
        void processCollection (triagens::arango::CollectionNameResolver const& resolver,
                                TRI_aql_collection_t* collection) {
#ifdef TRI_ENABLE_CLUSTER
          if (ServerState::instance()->isCoordinator()) {
            processCollectionCoordinator(resolver, collection);
            return;
          }
#endif

          processCollectionNormal(resolver, collection);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief add a coordinator collection to the transaction
////////////////////////////////////////////////////////////////////////////////

#ifdef TRI_ENABLE_CLUSTER
        void processCollectionCoordinator (triagens::arango::CollectionNameResolver const& resolver,
                                           TRI_aql_collection_t* collection) {
          TRI_voc_cid_t cid = resolver.getCollectionIdCluster(collection->_name);
         
          this->addCollection(cid, collection->_name, TRI_TRANSACTION_READ);
        }
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief add a regular collection to the transaction
////////////////////////////////////////////////////////////////////////////////

        void processCollectionNormal (triagens::arango::CollectionNameResolver const& resolver,
                                      TRI_aql_collection_t* collection) {
          TRI_vocbase_col_t const* col = resolver.getCollectionStruct(collection->_name);
          TRI_voc_cid_t cid = 0;

          if (col != 0) {
            cid = col->_cid;
          }

          int res = this->addCollection(cid, collection->_name, TRI_TRANSACTION_READ);

          if (res == TRI_ERROR_NO_ERROR && col != 0) {
            collection->_collection = (TRI_vocbase_col_t*) col;
          }
        }

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

    };

  }
}

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
