#ifndef _CLASS_CPYTHONUTIL_H_
#define _CLASS_CPYTHONUTIL_H_

#include <Python.h>

/*
 * This class encaplsulates the Python APIs.
 * you can use this class to achieve that C language invokes Python language
 *
 */
class CinvokePythonUtil {
	private:
		PyObject* pName     = NULL;
		PyObject* pMod      = NULL;
		PyObject* pDict     = NULL;
		PyObject* pClass    = NULL;
		PyObject* pInstance = NULL;
		PyObject* pResult   = NULL;
	public:
		/*
		 * The constructor is to perform a couple of necessary calls
		 * After initializing steps, it will get the python instance.
		 */
		CinvokePythonUtil(const char* module, const char* class_name) {

			Py_Initialize();
			if ( !Py_IsInitialized() ) 
			{ 
				PyErr_Print();
				printf("error Py_IsInitialized.......\n");
				exit(1);
			} 
			PyRun_SimpleString("import sys");
			PyRun_SimpleString("sys.path.append('./')");
			
			callMethod = PyObject_CallMethod;
			parseArg = PyArg_Parse;
			parseTuple = PyArg_ParseTuple;
			
			pName = PyString_FromString(module);

			if( !pName )
			{
				PyErr_Print();
				printf("error pName.......\n");
				exit(1);
			}

			pMod = PyImport_Import(pName);

			if( !pMod )
			{
				PyErr_Print();
				printf("error pMod.......\n");
				exit(1);
			}

			pDict = PyModule_GetDict(pMod);
			if ( !pDict )
			{
				PyErr_Print();
				printf("error pDict.......\n");
				exit(1);
			}

			pClass = PyDict_GetItemString(pDict, class_name);
			if ( !pClass )
			{
				PyErr_Print();
				printf("error pClass.......\n");
				exit(1);
			}

			pInstance = PyInstance_New(pClass, NULL, NULL);
			if ( !pInstance )
			{
				PyErr_Print();
				printf("error pInstance.......\n");
				exit(1);
			}
		}
		/*
		 * release all python objects via destructor
		 */	
		~CinvokePythonUtil() {
			if (pName) Py_DECREF(pName);
			if (pMod) Py_DECREF(pMod);
			if (pDict) Py_DECREF(pDict);
			if (pClass) Py_DECREF(pClass);
			if (pInstance) Py_DECREF(pInstance);
			if (pResult) Py_DECREF(pResult);
			Py_Finalize();
		}
		/*
		 * In order to encapsulate all access to Python APIs, 
		 * using function pointer to point to each methods we need instead of directly using Python APIs
		 */		
		PyObject* (*callMethod)(PyObject *o, char *method, char *format, ...);
		int (*parseArg)(PyObject *args, const char *format, ...);
		int (*parseTuple)(PyObject *args, const char *format, ...);
		
		PyObject* getInstance() {
			return pInstance;
		}
		
		PyObject* getResult() {
			return pResult;
		}
		/*
		 * As long as you call storeResult, u should notice that it can free the previous result. 
		 */	
		void storeResult(PyObject* result) {
			if (pResult) Py_DECREF(pResult);
			pResult = result;
		}
};


#endif 
