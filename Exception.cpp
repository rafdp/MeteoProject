#include "Builder.h"

void* ExceptionData_t::mem_alloc (size_t size)
{
	if (size > 0)
	{
		allocatedMem_ = new ExceptionHandler_t[size];
		usedMem_ = 0;
		availableMem_ = static_cast<int64_t> (size);
	}
	else
	{
		if (allocatedMem_ != nullptr)
		{
			if (availableMem_ == 0)
			{
				MessageBoxW (GetForegroundWindow (),
							 L"Not enough memory for exceptions\n", 
							 APPLICATION_TITLE_W, MB_OK);
				return nullptr;
			}
			usedMem_++;
			availableMem_--;

			return reinterpret_cast<void*>(allocatedMem_ + usedMem_ - 1);
		}
		else
		{
			printf ("Memory for exceptions has not been allocated\n");
		}
	}
	return nullptr;
}

ExceptionData_t::ExceptionData_t (size_t size, const char* filename) :
	allocatedMem_ (nullptr),
	usedMem_ (0),
	availableMem_ (0),
	filename_ (filename),
	log_ (nullptr)
{
	mem_alloc (size);
}

ExceptionData_t::~ExceptionData_t ()
{
	CloseLog ();
	if (allocatedMem_ != nullptr)
	{
		delete[] allocatedMem_;
		allocatedMem_ = nullptr;
	}
	usedMem_ = 0;
	availableMem_ = 0;
}

void ExceptionData_t::OpenLog ()
{
	if (log_) fclose (log_);
	fopen_s (&log_, filename_.c_str (), "w");
}

void ExceptionData_t::CloseLog ()
{
	if (log_) fclose (log_);
	log_ = nullptr;
}


ExceptionHandler_t::ExceptionHandler_t (const ExceptionHandler_t& that) :
	message_ (that.message_),
	error_code_ (that.error_code_),
	line_ (that.line_),
	file_ (that.file_),
	cause_ (that.cause_),
	pt_ (that.pt_)
{}

ExceptionHandler_t& ExceptionHandler_t::operator = (const ExceptionHandler_t& that)
{
	message_ = that.message_;
	error_code_ = that.error_code_;
	line_ = that.line_;
	file_ = that.file_;
	cause_ = that.cause_;
	pt_ = that.pt_;
	return *this;
}

ExceptionHandler_t::ExceptionHandler_t () :
	message_ (""),
	error_code_ (0),
	line_ (0),
	file_ (""),
	cause_ (nullptr),
	pt_ (this)
{}

ExceptionHandler_t::ExceptionHandler_t (const char*     message,
									int             error_code,
									int             line,
									const char*     file) :
	message_ (message),
	error_code_ (error_code),
	line_ (line),
	file_ (file),
	cause_ (nullptr),
	pt_ (this)
{}

ExceptionHandler_t::ExceptionHandler_t (const char*             message,
									int                     error_code,
									const ExceptionHandler_t* cause,
									int                     line,
									const char*             file) :
	message_ (message),
	error_code_ (error_code),
	line_ (line),
	file_ (file),
	cause_ (cause),
	pt_ (this)
{}

ExceptionHandler_t::~ExceptionHandler_t ()
{}

void ExceptionHandler_t::WriteLog (ExceptionData_t* data) const
{
	if (!data->log_)
	{
		//printf ("Log file not initialised\n");
		data->OpenLog ();
	}
	fprintf (data->log_, "Exception with error code %d,\n", error_code_);
	fprintf (data->log_, "error message: \"%s\"\n", message_);
	fprintf (data->log_, "occurred in file: \"%s\" on line %d\n", file_, line_);
	if (cause_ != nullptr)
	{
		fprintf (data->log_, "caused by:\n");
		cause_->WriteLog (data);
	}
	if (data->log_) data->CloseLog ();
}

void* ExceptionHandler_t::operator new (size_t, ExceptionData_t* data)
{
	return data->mem_alloc ();
}


NZA_t::NZA_t () :
	not_yet_destroyed_ (1)
{}

NZA_t::~NZA_t ()
{
	not_yet_destroyed_ = 0;
}
void  NZA_t::ok ()
{
	if (this == nullptr)
		_EXC_N (NULL_THIS, "Null this")
	if (not_yet_destroyed_ == 0)
		_EXC_N (DESTROYED, "Trying to access destroyed object")
}