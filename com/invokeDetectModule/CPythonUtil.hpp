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
		CinvokePythonUtil(const char* module, const char* class_name); 

		/*
		 * release all python objects via destructor
		 */	
		~CinvokePythonUtil(); 
		/*
		 * In order to encapsulate all access to Python APIs, 
		 * using function pointer to point to each methods we need instead of directly using Python APIs
		 */		
		PyObject* (*callMethod)(PyObject *o, char *method, char *format, ...);
		int (*parseArg)(PyObject *args, const char *format, ...);
		int (*parseTuple)(PyObject *args, const char *format, ...);

		PyObject* getInstance(); 
		PyObject* getResult();
		/*
		 * As long as you call storeResult, u should notice that it can free the previous result. 
		 */	
		void storeResult(PyObject* result); 
		
		PyObject* createPyArray(const unsigned char* frame, int height, int width, int channels);
};


#endif 
