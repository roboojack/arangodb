////////////////////////////////////////////////////////////////////////////////
/// @brief general server job
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2011 triagens GmbH, Cologne, Germany
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
/// @author Achim Brandt
/// @author Copyright 2009-2011, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_FYN_GENERAL_SERVER_GENERAL_SERVER_JOB_H
#define TRIAGENS_FYN_GENERAL_SERVER_GENERAL_SERVER_JOB_H 1

#include <Basics/Common.h>

#include <Logger/Logger.h>
#include <Basics/Exceptions.h>
#include <Basics/StringUtils.h>
#include <Basics/Mutex.h>
#include <Rest/Handler.h>

#include "Dispatcher/Job.h"
#include "Scheduler/AsyncTask.h"

namespace triagens {
  namespace rest {
    class Dispatcher;
    class Scheduler;

    ////////////////////////////////////////////////////////////////////////////////
    /// @brief general server job
    ////////////////////////////////////////////////////////////////////////////////

    template<typename S, typename H>
    class GeneralServerJob : public Job {
      GeneralServerJob (GeneralServerJob const&);
      GeneralServerJob& operator= (GeneralServerJob const&);

      public:

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief constructs a new server job
        ///
        /// Note that the job owns the handler. It is its responsibility to destroy
        /// the handler.
        ////////////////////////////////////////////////////////////////////////////////

        GeneralServerJob (S* server, Scheduler* scheduler, Dispatcher* dispatcher, AsyncTask* task, H* handler)
          : Job("HttpServerJob"),
            _server(server),
            _scheduler(scheduler),
            _dispatcher(dispatcher),
            _task(task),
            _handler(handler),
            _shutdown(0),
            _done(0) {
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief destructs a server job
        ////////////////////////////////////////////////////////////////////////////////

        ~GeneralServerJob () {
          if (_handler) {
            delete _handler;
          }
        }

      public:

        ////////////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        ////////////////////////////////////////////////////////////////////////////////

        JobType type () {
          return _handler->type();
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        ////////////////////////////////////////////////////////////////////////////////

        string const& queue () {
          return _handler->queue();
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        ////////////////////////////////////////////////////////////////////////////////

        void setDispatcherThread (DispatcherThread* thread) {
          _handler->setDispatcherThread(thread);
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        ////////////////////////////////////////////////////////////////////////////////

        status_e work () {
          LOGGER_TRACE << "beginning job " << static_cast<Job*>(this);

          if (_shutdown != 0) { 
            return Job::JOB_DONE;
          }

          Handler::status_e status = _handler->execute();

          LOGGER_TRACE << "finished job " << static_cast<Job*>(this) << " with status " << status;

          switch (status) {
            case Handler::HANDLER_DONE:    return Job::JOB_DONE;
            case Handler::HANDLER_REQUEUE: return Job::JOB_REQUEUE;
            case Handler::HANDLER_FAILED:  return Job::JOB_FAILED;
          }

          return Job::JOB_FAILED;
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        ////////////////////////////////////////////////////////////////////////////////

        void cleanup () {
          if (_shutdown != 0) {
            delete this;
          }
          else {
            assert(_task);

            _done = 1;
            _task->signal();
          }
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        ////////////////////////////////////////////////////////////////////////////////

        void finish (void*) {
          LOGGER_ERROR << "finished called for GeneralServerJob";
          cleanup();
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// {@inheritDoc}
        ////////////////////////////////////////////////////////////////////////////////

        void handleError (basics::TriagensError const& ex) {
          _handler->handleError(ex);
        }
        
        ////////////////////////////////////////////////////////////////////////////////
        /// @brief shuts down the execution and deletes everything
        ////////////////////////////////////////////////////////////////////////////////

        bool beginShutdown () {
          LOGGER_TRACE << "shutdown, job (" << ((void*) this) << ") is " << (_done ? "done" : "still running");
  
          if (_done != 0) {
            delete this;
            return true;
          }
          else {
            _shutdown = 1;
          }

          return false;
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief general server
        ////////////////////////////////////////////////////////////////////////////////

        S* getServer () const {
          return _server;
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief scheduler
        ////////////////////////////////////////////////////////////////////////////////

        Scheduler* getScheduler () const {
          return _scheduler;
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief dispatcher
        ////////////////////////////////////////////////////////////////////////////////

        Dispatcher* getDispatcher () const {
          return _dispatcher;
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief task
        ////////////////////////////////////////////////////////////////////////////////

        AsyncTask* getTask () const {
          return _task;
        }

        ////////////////////////////////////////////////////////////////////////////////
       /// @brief handler
        ////////////////////////////////////////////////////////////////////////////////

        H* getHandler () const {
          return _handler;
        }

      protected:

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief general server
        ////////////////////////////////////////////////////////////////////////////////

        S* _server;

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief scheduler
        ////////////////////////////////////////////////////////////////////////////////

        Scheduler* _scheduler;

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief dispatcher
        ////////////////////////////////////////////////////////////////////////////////

        Dispatcher* _dispatcher;

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief task
        /////////////////o///////////////////////////////////////////////////////////////

        AsyncTask* _task;

        ////////////////////////////////////////////////////////////////////////////////
        /// @brief handler
        ////////////////////////////////////////////////////////////////////////////////

        H* _handler;

      private:
        volatile sig_atomic_t _shutdown;
        volatile sig_atomic_t _done;
    };
  }
}

#endif
