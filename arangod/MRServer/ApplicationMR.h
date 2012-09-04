////////////////////////////////////////////////////////////////////////////////
/// @brief MR enigne configuration
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2012 triagens GmbH, Cologne, Germany
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
/// @author Dr. Frank Celler
/// @author Copyright 2011-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_REST_SERVER_APPLICATION_MR_H
#define TRIAGENS_REST_SERVER_APPLICATION_MR_H 1

#include "ApplicationServer/ApplicationFeature.h"

#include "MRuby/mr-utils.h"

#include "Basics/ConditionVariable.h"
#include "MRuby/MRLoader.h"

// -----------------------------------------------------------------------------
// --SECTION--                                              forward declarations
// -----------------------------------------------------------------------------

extern "C" {
  struct TRI_vocbase_s;
}

namespace triagens {
  namespace basics {
    class Thread;
  }

// -----------------------------------------------------------------------------
// --SECTION--                                               class ApplicationMR
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

  namespace arango {

////////////////////////////////////////////////////////////////////////////////
/// @brief application simple user and session management feature
////////////////////////////////////////////////////////////////////////////////

    class ApplicationMR : public rest::ApplicationFeature {
      private:
        ApplicationMR (ApplicationMR const&);
        ApplicationMR& operator= (ApplicationMR const&);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief MR isolate and context
////////////////////////////////////////////////////////////////////////////////

        struct MRContext {
          MR_state_t* _mrs;

////////////////////////////////////////////////////////////////////////////////
/// @brief number of requests since last GC of the context 
////////////////////////////////////////////////////////////////////////////////

          size_t _dirt;

////////////////////////////////////////////////////////////////////////////////
/// @brief timestamp of last GC for the context
////////////////////////////////////////////////////////////////////////////////

          double _lastGcStamp;

        };

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

        ApplicationMR (string const& binaryPath);

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

        ~ApplicationMR ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the concurrency
////////////////////////////////////////////////////////////////////////////////

        void setConcurrency (size_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the database
////////////////////////////////////////////////////////////////////////////////

        void setVocbase (struct TRI_vocbase_s*);

////////////////////////////////////////////////////////////////////////////////
/// @brief enters an context
////////////////////////////////////////////////////////////////////////////////

        MRContext* enterContext ();

////////////////////////////////////////////////////////////////////////////////
/// @brief exists an context
////////////////////////////////////////////////////////////////////////////////

        void exitContext (MRContext*);

////////////////////////////////////////////////////////////////////////////////
/// @brief runs the garbage collection
////////////////////////////////////////////////////////////////////////////////

        void collectGarbage ();

////////////////////////////////////////////////////////////////////////////////
/// @brief disables actions
////////////////////////////////////////////////////////////////////////////////

        void disableActions ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                        ApplicationFeature methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ApplicationServer
/// @{
////////////////////////////////////////////////////////////////////////////////

      public:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void setupOptions (map<string, basics::ProgramOptionsDescription>&);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool prepare ();

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool start ();

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void close ();

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void stop ();

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief prepares a MR instance
////////////////////////////////////////////////////////////////////////////////

        bool prepareMRInstance (size_t i);

////////////////////////////////////////////////////////////////////////////////
/// @brief shut downs a MR instances
////////////////////////////////////////////////////////////////////////////////

        void shutdownMRInstance (size_t i);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup ArangoDB
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief path to the directory containing alternate startup scripts
///
/// @CMDOPT{--ruby.directory @CA{directory}}
///
/// Specifies the @CA{directory} path to alternate startup MRuby files.
/// Normally, the server will start using built-in MRuby core functionality. To
/// override the core functionality with a different implementation, this option
/// can be used.
////////////////////////////////////////////////////////////////////////////////

        string _startupPath;

////////////////////////////////////////////////////////////////////////////////
/// @brief semicolon separated list of module directories
///
/// @CMDOPT{--ruby.modules-path @CA{directory}}
///
/// Specifies the @CA{directory} path with user defined MRuby modules.  Multiple
/// paths can be specified separated with commas.
////////////////////////////////////////////////////////////////////////////////

        string _startupModules;

////////////////////////////////////////////////////////////////////////////////
/// @brief path to the system action directory
///
/// @CMDOPT{--ruby.action-directory @CA{directory}}
///
/// Specifies the @CA{directory} containg the system defined MRuby files that
/// can be invoked as actions.
////////////////////////////////////////////////////////////////////////////////

        string _actionPath;

////////////////////////////////////////////////////////////////////////////////
/// @brief MRuby garbage collection interval (each x requests)
///
/// @CMDOPT{--ruby.gc-interval @CA{interval}}
///
/// Specifies the interval (approximately in number of requests) that the
/// garbage collection for MRuby objects will be run in each thread.
////////////////////////////////////////////////////////////////////////////////

        uint64_t _gcInterval;

////////////////////////////////////////////////////////////////////////////////
/// @brief MRuby garbage collection frequency (each x seconds)
///
/// @CMDOPT{--ruby.gc-frequency @CA{frequency}}
///
/// Specifies the frequency in seconds for the automatic garbage collection of
/// MRuby objects. This setting is useful to have the garbage collection 
/// still work in periods with no or little numbers of requests.
////////////////////////////////////////////////////////////////////////////////

        double _gcFrequency;

////////////////////////////////////////////////////////////////////////////////
/// @brief MR startup loader
////////////////////////////////////////////////////////////////////////////////

        MRLoader _startupLoader;

////////////////////////////////////////////////////////////////////////////////
/// @brief MR action loader
////////////////////////////////////////////////////////////////////////////////

        MRLoader _actionLoader;

////////////////////////////////////////////////////////////////////////////////
/// @brief database
////////////////////////////////////////////////////////////////////////////////

        struct TRI_vocbase_s* _vocbase;

////////////////////////////////////////////////////////////////////////////////
/// @brief number of instances to create
////////////////////////////////////////////////////////////////////////////////

        size_t _nrInstances;

////////////////////////////////////////////////////////////////////////////////
/// @brief MR contexts
////////////////////////////////////////////////////////////////////////////////

        MRContext** _contexts;

////////////////////////////////////////////////////////////////////////////////
/// @brief MR contexts queue lock
////////////////////////////////////////////////////////////////////////////////

        basics::ConditionVariable _contextCondition;

////////////////////////////////////////////////////////////////////////////////
/// @brief MR free contexts
////////////////////////////////////////////////////////////////////////////////

        std::vector<MRContext*> _freeContexts;

////////////////////////////////////////////////////////////////////////////////
/// @brief MR free contexts
////////////////////////////////////////////////////////////////////////////////

        std::vector<MRContext*> _dirtyContexts;

////////////////////////////////////////////////////////////////////////////////
/// @brief shutdown in progress
////////////////////////////////////////////////////////////////////////////////

        volatile sig_atomic_t _stopping;

////////////////////////////////////////////////////////////////////////////////
/// @brief garbage collection thread
////////////////////////////////////////////////////////////////////////////////

        basics::Thread* _gcThread;
    };
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:
