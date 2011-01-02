/*
 *  Copyright 2010 (C) CNRS
 *  Author: Florent Lamiraux
 */

#include <Python.h>
#include <iostream>
#include <sstream>
#include <string>

#include <dynamic-graph/debug.h>
#include <dynamic-graph/exception-factory.h>
#include <dynamic-graph/signal-base.h>

namespace dynamicgraph {
  namespace python {

    // Declare functions defined in other source files
    namespace signalBase {
      extern PyObject* create(PyObject* self, PyObject* args);
      extern PyObject* getTime(PyObject* self, PyObject* args);
      extern PyObject* getValue(PyObject* self, PyObject* args);
      extern PyObject* setValue(PyObject* self, PyObject* args);
      extern PyObject* recompute(PyObject* self, PyObject* args);
    }
    namespace entity {
      extern PyObject* create(PyObject* self, PyObject* args);
      extern PyObject* getName(PyObject* self, PyObject* args);
      extern PyObject* getSignal(PyObject* self, PyObject* args);
      extern PyObject* displaySignals(PyObject* self, PyObject* args);
      extern PyObject* executeCommand(PyObject* self, PyObject* args);
      extern PyObject* listCommands(PyObject* self, PyObject* args);
      extern PyObject* getCommandDocstring(PyObject* self, PyObject* args);
    }

    namespace factory {
      PyObject* getEntityClassList(PyObject* self, PyObject* args);
    }
    namespace signalCaster {
      PyObject* getSignalTypeList(PyObject* self, PyObject* args);
    }

    PyObject* error;

    /**
       \brief plug a signal into another one.
    */
    PyObject*
    plug(PyObject* self, PyObject* args)
    {
      PyObject* objOut = NULL;
      PyObject* objIn = NULL;
      void* pObjOut;
      void* pObjIn;

      if (!PyArg_ParseTuple(args,"OO", &objOut, &objIn))
	return NULL;

      if (!PyCObject_Check(objOut))
	return NULL;
      if (!PyCObject_Check(objIn))
	return NULL;

      pObjIn = PyCObject_AsVoidPtr(objIn);
	SignalBase<int>* signalIn = (SignalBase<int>*)pObjIn;
      pObjOut = PyCObject_AsVoidPtr(objOut);
	SignalBase<int>* signalOut = (SignalBase<int>*)pObjOut;
      std::ostringstream os;

      try {
	signalIn->plug(signalOut);
      } catch (std::exception& exc) {
	PyErr_SetString(error, exc.what());
	return NULL;
      }
      return Py_BuildValue("");
    }

    PyObject*
    enableTrace(PyObject* self, PyObject* args)
    {
      PyObject* boolean;
      char* filename = NULL;

      if (PyArg_ParseTuple(args,"Os", &boolean, &filename)) {
	if (!PyBool_Check(boolean)) {
	  PyErr_SetString(PyExc_TypeError, "enableTrace takes as first "
			  "argument True or False,\n""           and as "
			  "second argument a filename.");
	  return NULL;
	}
	if (boolean == Py_True) {
	  try {
	    DebugTrace::openFile(filename);
	  } catch (const std::exception& exc) {
	    PyErr_SetString(PyExc_IOError, exc.what());
	    return NULL;
	  }
	} else {
	  try {
	    DebugTrace::closeFile(filename);
	  } catch (const std::exception& exc) {
	    PyErr_SetString(PyExc_IOError, exc.what());
	    return NULL;
	  }
	}
      } else {
	return NULL;
      }
      return Py_BuildValue("");
    }
  }
}

/**
   \brief List of python functions
*/
static PyMethodDef dynamicGraphMethods[] = {
  {"w_plug",  dynamicgraph::python::plug, METH_VARARGS,
   "plug an output signal into an input signal"},
  {"enableTrace",  dynamicgraph::python::enableTrace, METH_VARARGS,
   "Enable or disable tracing debug info in a file"},
  // Signals
  {"create_signal_base", dynamicgraph::python::signalBase::create, METH_VARARGS,
   "create a SignalBase C++ object"},
  {"signal_base_get_time", dynamicgraph::python::signalBase::getTime,
   METH_VARARGS, "Get time of  a SignalBase"},
  {"signal_base_get_value", dynamicgraph::python::signalBase::getValue,
   METH_VARARGS, "Read the value of a signal"},
  {"signal_base_set_value", dynamicgraph::python::signalBase::setValue,
   METH_VARARGS, "Set the value of a signal"},
  {"signal_base_recompute", dynamicgraph::python::signalBase::recompute,
   METH_VARARGS, "Recompute the signal at given time"},
  // Entity
  {"create_entity", dynamicgraph::python::entity::create, METH_VARARGS,
   "create an Entity C++ object"},
  {"entity_get_name", dynamicgraph::python::entity::getName, METH_VARARGS,
   "get the name of an Entity"},
  {"entity_get_signal", dynamicgraph::python::entity::getSignal, METH_VARARGS,
   "get signal by name from an Entity"},
  {"entity_display_signals", dynamicgraph::python::entity::displaySignals,
   METH_VARARGS,
   "Display the list of signals of an entity in standard output"},
  {"entity_execute_command",
   dynamicgraph::python::entity::executeCommand,
   METH_VARARGS,
   "execute a command"},
  {"entity_list_commands",
   dynamicgraph::python::entity::listCommands,
   METH_VARARGS,
   "list the commands of an entity"},
  {"entity_get_command_docstring",
   dynamicgraph::python::entity::getCommandDocstring,
   METH_VARARGS,
   "get the docstring of an entity command"},
  {"factory_get_entity_class_list",
   dynamicgraph::python::factory::getEntityClassList,
   METH_VARARGS,
   "return the list of entity classes"},
  {"signal_caster_get_type_list",
   dynamicgraph::python::signalCaster::getSignalTypeList,
   METH_VARARGS,
   "return the list of signal type names"},
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC
initwrap(void)
{
    PyObject *m;

    m = Py_InitModule("wrap", dynamicGraphMethods);
    if (m == NULL)
        return;

    std::string msg("dynamic_graph.error");

    dynamicgraph::python::error =
      PyErr_NewException(const_cast<char*>(msg.c_str()), NULL, NULL);
    Py_INCREF(dynamicgraph::python::error);
    PyModule_AddObject(m, "error", dynamicgraph::python::error);
}
