#ifndef __MODULE_LOG__
#define __MODULE_LOG__

#include "globals.h"

class logstream: public std::ostringstream
 {
 public:
     typedef
      enum
       {
        quiet,    // no output
        error,    // only errors are output
        warning,  // errors and warnings are output
        info,     // errors, warnings, and informative messages
        all,      // all messages are output
        none,     // end of enum marker
       } log_level;
 
  private:
     log_level _m_log_level;
     log_level _m_current_level; // only until the next flush/endl

     std::ostream & out;

  public:
     void flush()
      {
       if (_m_current_level<=_m_log_level)
        {
         struct timeval tv;
         gettimeofday(&tv,0);
         time_t now = tv.tv_sec;
         struct tm * now_tm = localtime(&now);
         char buffer[40];
         strftime(buffer,sizeof(buffer),"%Y/%m/%d %H:%M:%S",now_tm);
         out << buffer
             << "." << std::setfill('0') 
             << std::setw(6) << (tv.tv_usec) 
             << std::setfill(' ') << ' '
             << str();
         out.flush();
        }

       str("");
       _m_current_level=none;
      }

   template <typename T>
   inline logstream & operator<<( const T & t )
     {
      (*(std::ostringstream*)this) << t;
      return *this;
     }

  inline logstream & operator<<(const log_level & level) { _m_current_level=level; return *this;} 

  // sets the acceptable message level
  // until next flush/endl
  inline void set_level(log_level level) { _m_current_level = level; }
  inline log_level get_level() const { return _m_current_level; }

  inline void change_log_level(log_level level) { _m_log_level = level; }
  inline log_level get_log_lvel() const { return _m_log_level; }

  /*
  logstream(const char * filename, 
            log_level log_level=all,
            std::ios::openmode mode=std::ios::app)
   : _m_log_level(log_level),
     _m_current_level(none),
     out(filename,mode)
   { 

   }
  */

  logstream ( std::ostream & log_stream,
              log_level level=all)
   : _m_log_level(level),
     _m_current_level(none),
     out(log_stream)
  {
  }
              


  virtual ~logstream() 
   { 
    flush();
   }

  // stubs for manipulators
  typedef logstream & (*logstream_manip)(logstream &);
  logstream & operator<<(logstream_manip manip) { return manip(*this); }


 };

struct __logstream_level { logstream::log_level _m_level; };
inline __logstream_level setlevel(logstream::log_level _level)
 {
  __logstream_level level = { _level };
  return level;
 }

inline logstream & operator<<(logstream & out, const __logstream_level & level )
 {
  out.set_level(level._m_level);
  return out;
 }

namespace std { inline logstream & endl(logstream & out) { out.put('\n'); out.flush(); return out; } }



#endif
 // __MODULE_LOG__
