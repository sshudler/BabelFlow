/*
 * Copyright (c) 2017 University of Utah 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TASKGRAPH_H
#define TASKGRAPH_H

#include <vector>
#include <cstdio>
#include <cassert>
#include <string>

#include "Task.h"
#include "Payload.h"

namespace BabelFlow {

class TaskMap;

/*! The task graph defines a baseclass for all algorithms
 *  that want to use the data flow for communication. It consists
 *  of pure a interface to return (subsets of) tasks.
 */
class TaskGraph
{
public:

  //! Default constructor
  TaskGraph(std::string config = "") {}

  //! Default destructor
  virtual ~TaskGraph() {}

  //! Compute the fully specified tasks for the given controller
  virtual std::vector<Task> localGraph(ShardId id, const TaskMap* task_map) const = 0;

  //! Return the task for the given global task id
  virtual Task task(uint64_t gId) const = 0;

  //! Return the global id of the given task id
  virtual uint64_t gId(TaskId tId) const = 0;

  //! Return the global id of the given leaf id
  //virtual uint64_t leaf(uint64_t lId) const = 0;

  //! Return the total number of tasks (or some reasonable upper bound)
  virtual TaskId size() const = 0;

  //! Serialize a task graph
  virtual Payload serialize() const {assert(false);return Payload();}

  //! Deserialize a task graph. This will consume the payload
  virtual void deserialize(Payload buffer) {assert(false);}

  //! Output the entire graph as dot file
  virtual int output_graph(ShardId count, const TaskMap* task_map, FILE* output);
};

/*! The task map defines an abstract baseclass to define the global
 *  Task-to-Controller map as well as the reverse
 */
class TaskMap
{
public:

  //! Default constructor
  TaskMap() {}

  //! Default destructor
  virtual ~TaskMap() {}

  //! Return which controller is assigned to the given task
  virtual ShardId shard(TaskId id) const = 0;

  //! Return the set of task assigned to the given controller
  virtual std::vector<TaskId> tasks(ShardId id) const = 0;
};

/*! The controller map defines a baseclass to define the controller
 *   to MPI_RANK map and its reverse. The default map is identity.
 *   We assume an rank can have at most one controller but not
 *   every rank must have one.
 */
class ControllerMap
{
public:

  //! Default constructor
  ControllerMap() {}

  //! Default destructor
  virtual ~ControllerMap() {}

  //! Return the MPI_RANK to which the given controller is assigned
  virtual uint32_t rank(ShardId id) const {return id;}

  //! Return the controller assigned to the given rank (could be CNULL)
  virtual ShardId controller(uint32_t rank) const {return rank;}
};
  
}

#endif /* TASKGRAPH_H_ */
