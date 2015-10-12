#pragma once
#include "includes.h"


class NonCopiable_t
{
	NonCopiable_t (const NonCopiable_t&);
	void operator = (const NonCopiable_t&);
public:
	NonCopiable_t () {}
};

#define DISABLE_CLASS_COPY(type) \
type (const type&); \
type& operator = (const type&);


#define E_NAT(message, code) (message), (code), __LINE__, __FILE__
#define E_CONS(message, code, pt) (message), (code), (pt), __LINE__, __FILE__

#define NAT_EXCEPTION(data, message, code) \
{ \
    ExceptionHandler_t* e_nat = new (data) ExceptionHandler_t (E_NAT ((message), (code))); \
    throw *e_nat; \
}

#define CONS_EXCEPTION(data, message, code, old) \
{ \
    ExceptionHandler_t* ec = new (data) ExceptionHandler_t (E_CONS ((message), (code), (old).pt_)); \
    throw *ec; \
}

#define DETAILED_CONSECUTIVE_CATCH(message, code, expn) \
catch (ExceptionHandler_t& catchedException00172456) \
{ \
    static std::string completeMessage00172456 (__FUNCSIG__); \
    completeMessage00172456 += ": "; \
    completeMessage00172456 += message; \
    CONS_EXCEPTION (expn, completeMessage00172456.c_str (), code, catchedException00172456); \
}

#define DETAILED_CATCH(message, code, catching, expn) \
catch (catching) \
{ \
    static std::string completeMessage43215876 (__FUNCSIG__); \
    completeMessage43215876 += ": ."; \
    completeMessage43215876 += #catching; \
    completeMessage43215876 += ". "; \
    NAT_EXCEPTION (expn, completeMessage43215876.c_str (), code); \
}

#define DETAILED_UNKNOWN_CATCH(message, code, expn) \
catch (...) \
{ \
    static std::string completeMessage67416279 (__FUNCSIG__); \
    completeMessage67416279 += ": .unknown."; \
    NAT_EXCEPTION (expn, completeMessage67416279.c_str (), code); \
}

#define _EXC_N(code, msg) { NAT_EXCEPTION (__EXPN__, CreateStringOnFail (msg).c_str (), ERROR_##code) }
#define _ ,

#define BEGIN_EXCEPTION_HANDLING \
try { ok ();

#define END_EXCEPTION_HANDLING(code)\
ok (); } \
DETAILED_CONSECUTIVE_CATCH ("Error occurred", ERROR_##code, __EXPN__) \
DETAILED_CATCH             ("Error occurred", ERROR_BAD_ALLOC, std::bad_alloc, __EXPN__) \
DETAILED_UNKNOWN_CATCH     ("Error occurred", ERROR_UNKNOWN, __EXPN__)

#define _END_EXCEPTION_HANDLING(code)\
DETAILED_CONSECUTIVE_CATCH ("Error occurred", ERROR_##code, __EXPN__) \
DETAILED_CATCH             ("Error occurred", ERROR_BAD_ALLOC, std::bad_alloc, __EXPN__) \
DETAILED_UNKNOWN_CATCH     ("Error occurred", ERROR_UNKNOWN, __EXPN__)

#define DEFAULT_OK_BLOCK \
NZA_t::ok (); \
if (this == nullptr) \
    _EXC_N (NULL_THIS, "Null this")\

class ExceptionData_t;

class ExceptionHandler_t
{
private:
	const char* message_;
	int error_code_;
	int line_;
	const char* file_;

	const ExceptionHandler_t* cause_;


public:
	const ExceptionHandler_t* pt_;
	ExceptionHandler_t ();
	ExceptionHandler_t (const char*     message,
					  int             error_code_,
					  int             line,
					  const char*     file);

	ExceptionHandler_t (const char*             message,
					  int                     error_code_,
					  const ExceptionHandler_t* cause,
					  int                     line,
					  const char*             file);

	ExceptionHandler_t (const ExceptionHandler_t& that);
	ExceptionHandler_t& operator = (const ExceptionHandler_t& that);

	~ExceptionHandler_t ();
	void WriteLog (ExceptionData_t* data) const;
	void* operator new(size_t s, ExceptionData_t* data) throw();
};

class ExceptionData_t : NonCopiable_t
{
	DISABLE_CLASS_COPY (ExceptionData_t)
public:
	ExceptionHandler_t* allocatedMem_;
	int64_t usedMem_;
	int64_t availableMem_;
	std::string filename_;
	FILE* log_;

	void* mem_alloc (size_t size = 0);

	ExceptionData_t (size_t size, const char* filename);

	~ExceptionData_t ();

	void OpenLog ();

	void CloseLog ();

};

class NZA_t
{
protected:
	int not_yet_destroyed_;

public:
	NZA_t ();

	virtual ~NZA_t ();
	virtual void ok ();
};


#define EXCEPTION_ENVIRONMENT_HEADER(file) \
static ExceptionData_t* __EXPN__ = nullptr; \
void SET_##file##_EXPN (ExceptionData_t* expn) \
{ __EXPN__ = expn; }

#define EXCEPTION_ENVIRONMENT_LINK_FILE(file) \
void SET_##file##_EXPN (ExceptionData_t* expn);

#define EXCEPTION_ENVIRONMENT_ADD_FILE(file) \
SET_##file##_EXPN (__EXPN__);