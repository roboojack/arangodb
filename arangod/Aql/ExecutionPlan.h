////////////////////////////////////////////////////////////////////////////////
/// @brief Aql, execution plan
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
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
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Steemann
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2012-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_AQL_EXECUTION_PLAN_H
#define ARANGODB_AQL_EXECUTION_PLAN_H 1

#include "Basics/Common.h"
#include "Aql/ExecutionNode.h"
#include "Aql/ModificationOptions.h"
#include "Aql/Query.h"
#include "Basics/json.h"

namespace triagens {
  namespace aql {

    class Ast;
    struct AstNode;
    class CalculationNode;
    class ExecutionNode;

// -----------------------------------------------------------------------------
// --SECTION--                                               class ExecutionPlan
// -----------------------------------------------------------------------------

    class ExecutionPlan {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief create the plan
////////////////////////////////////////////////////////////////////////////////

        ExecutionPlan (Ast*);

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the plan, frees all assigned nodes
////////////////////////////////////////////////////////////////////////////////

        ~ExecutionPlan ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan from an AST
////////////////////////////////////////////////////////////////////////////////

        static ExecutionPlan* instanciateFromAst (Ast*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan from JSON
////////////////////////////////////////////////////////////////////////////////

        static void getCollectionsFromJson(Ast *ast, 
                                           triagens::basics::Json const& Json);


        static ExecutionPlan* instanciateFromJson (Ast* ast,
                                                   triagens::basics::Json const& Json);

////////////////////////////////////////////////////////////////////////////////
/// @brief export to JSON, returns an AUTOFREE Json object
////////////////////////////////////////////////////////////////////////////////

        triagens::basics::Json toJson (Ast* ast,
                                       TRI_memory_zone_t* zone,
                                       bool verbose) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief note that an optimizer rule was applied
////////////////////////////////////////////////////////////////////////////////

        inline void addAppliedRule (int level) {
          _appliedRules.push_back(level);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get a list of all applied rules
////////////////////////////////////////////////////////////////////////////////

        std::vector<std::string> getAppliedRules () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief return the next value for a node id
////////////////////////////////////////////////////////////////////////////////
        
        inline size_t nextId () {
          return ++_nextId;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get a node by its id
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* getNodeById (size_t id) const;
     
////////////////////////////////////////////////////////////////////////////////
/// @brief get the root node
////////////////////////////////////////////////////////////////////////////////
        
        ExecutionNode* root () const {
          TRI_ASSERT(_root != nullptr);
          return _root;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief invalidate all cost estimations in the plan
////////////////////////////////////////////////////////////////////////////////

        void invalidateCost () {
          TRI_ASSERT(_root != nullptr);
          return _root->invalidateCost();
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get the estimated cost . . .
////////////////////////////////////////////////////////////////////////////////

        double getCost () {
          TRI_ASSERT(_root != nullptr);
          return _root->getCost();
        }
       
////////////////////////////////////////////////////////////////////////////////
/// @brief show an overview over the plan
////////////////////////////////////////////////////////////////////////////////

        void show ();

////////////////////////////////////////////////////////////////////////////////
/// @brief get the node where variable with id <id> is introduced . . .
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* getVarSetBy (VariableId id) const {
          auto it = _varSetBy.find(id);
          if( it == _varSetBy.end()){
            return nullptr;
          }
          return (*it).second;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief find nodes of a certain type
////////////////////////////////////////////////////////////////////////////////

        std::vector<ExecutionNode*> findNodesOfType (ExecutionNode::NodeType,
                                                     bool enterSubqueries);

////////////////////////////////////////////////////////////////////////////////
/// @brief check linkage
////////////////////////////////////////////////////////////////////////////////

#if 0
        void checkLinkage ();
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief determine and set _varsUsedLater and _valid and _varSetBy
////////////////////////////////////////////////////////////////////////////////

        void findVarUsage ();

////////////////////////////////////////////////////////////////////////////////
/// @brief determine if the above are already set
////////////////////////////////////////////////////////////////////////////////

        bool varUsageComputed ();

////////////////////////////////////////////////////////////////////////////////
/// @brief unlinkNodes, note that this does not delete the removed
/// nodes and that one cannot remove the root node of the plan.
////////////////////////////////////////////////////////////////////////////////

        void unlinkNodes (std::unordered_set<ExecutionNode*>& toUnlink);

////////////////////////////////////////////////////////////////////////////////
/// @brief unlinkNode, note that this does not delete the removed
/// node and that one cannot remove the root node of the plan.
////////////////////////////////////////////////////////////////////////////////

        void unlinkNode (ExecutionNode*);

////////////////////////////////////////////////////////////////////////////////
/// @brief add a node to the plan, will delete node if addition fails and
/// throw an exception, in addition, the pointer is set to nullptr such
/// that another delete does not hurt
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* registerNode (ExecutionNode*);

////////////////////////////////////////////////////////////////////////////////
/// @brief replaceNode, note that <newNode> must be registered with the plan
/// before this method is called, also this does not delete the old
/// node and that one cannot replace the root node of the plan.
////////////////////////////////////////////////////////////////////////////////

        void replaceNode (ExecutionNode* oldNode, 
                          ExecutionNode* newNode); 

////////////////////////////////////////////////////////////////////////////////
/// @brief insert <newNode> as a new (the first!) dependency of
/// <oldNode> and make the former first dependency of <oldNode> a
/// dependency of <newNode> (and no longer a direct dependency of
/// <oldNode>).
/// <newNode> must be registered with the plan before this method is called.
////////////////////////////////////////////////////////////////////////////////

        void insertDependency (ExecutionNode* oldNode, 
                               ExecutionNode* newNode);

////////////////////////////////////////////////////////////////////////////////
/// @brief clone the plan by recursively cloning starting from the root
////////////////////////////////////////////////////////////////////////////////

        ExecutionPlan* clone ();

////////////////////////////////////////////////////////////////////////////////
/// @brief get ast
////////////////////////////////////////////////////////////////////////////////

        Ast* getAst () {
          return _ast;
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief create modification options from an AST node
////////////////////////////////////////////////////////////////////////////////

        ModificationOptions createOptions (AstNode const*);


////////////////////////////////////////////////////////////////////////////////
/// @brief creates a calculation node for an arbitrary expression
////////////////////////////////////////////////////////////////////////////////

        CalculationNode* createTemporaryCalculation (AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief adds "previous" as dependency to "plan", returns "plan"
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* addDependency (ExecutionNode*,
                                      ExecutionNode*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST FOR node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeFor (ExecutionNode*,
                                    AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST FILTER node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeFilter (ExecutionNode*,
                                       AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST LET node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeLet (ExecutionNode*,
                                    AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST SORT node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeSort (ExecutionNode*,
                                     AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST COLLECT node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeCollect (ExecutionNode*,
                                        AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST LIMIT node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeLimit (ExecutionNode*,
                                      AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST RETURN node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeReturn (ExecutionNode*,
                                       AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST REMOVE node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeRemove (ExecutionNode*,
                                       AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST INSERT node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeInsert (ExecutionNode*,
                                       AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST UPDATE node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeUpdate (ExecutionNode*,
                                       AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan element from an AST REPLACE node
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode* fromNodeReplace (ExecutionNode*,
                                        AstNode const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create an execution plan from an abstract syntax tree node
////////////////////////////////////////////////////////////////////////////////
  
        ExecutionNode* fromNode (AstNode const*);

        ExecutionNode* fromJson (triagens::basics::Json const& Json);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief map from node id to the actual node
////////////////////////////////////////////////////////////////////////////////
        
        std::unordered_map<size_t, ExecutionNode*> _ids;

////////////////////////////////////////////////////////////////////////////////
/// @brief root node of the plan
////////////////////////////////////////////////////////////////////////////////

        ExecutionNode*               _root;

////////////////////////////////////////////////////////////////////////////////
/// @brief get the node where a variable is introducted.
////////////////////////////////////////////////////////////////////////////////

        std::unordered_map<VariableId, ExecutionNode*> _varSetBy;

////////////////////////////////////////////////////////////////////////////////
/// @brief which optimizer rules were applied for a plan
////////////////////////////////////////////////////////////////////////////////

        std::vector<int> _appliedRules;

////////////////////////////////////////////////////////////////////////////////
/// @brief flag to indicate whether the variable usage is computed
////////////////////////////////////////////////////////////////////////////////

        bool _varUsageComputed;

////////////////////////////////////////////////////////////////////////////////
/// @brief auto-increment sequence for node ids
////////////////////////////////////////////////////////////////////////////////

        size_t _nextId;
        
////////////////////////////////////////////////////////////////////////////////
/// @brief the ast
////////////////////////////////////////////////////////////////////////////////

        Ast* _ast;
    };

  }
}

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
