
//#include "..\..\..\stdafx.h"

#include "..\DllLoaderContainer.h"
#include "..\DllLoader.h"
//#include "emu_msvcrt.h"

#pragma warning (disable:4391)
#pragma warning (disable:4392)

extern "C" void* dll_close();
extern "C" void* dll_lseek();
extern "C" void* dll_read();
extern "C" void* dll_write();
extern "C" void* dll__dllonexit();
extern "C" void* __mb_cur_max();
extern "C" void* _assert();
extern "C" void* _errno();
extern "C" void* _ftime();
extern "C" void* _iob();
extern "C" void* _isctype();
extern "C" void* dll_lseeki64();
extern "C" void* dll_open();
extern "C" void* _snprintf();
extern "C" void* _stricmp();
extern "C" void* _strnicmp();
extern "C" void* _vsnprintf();
extern "C" void* dllabort();
extern "C" void* atof();
extern "C" void* atoi();
extern "C" void* cos();
extern "C" void* cosh();
extern "C" void* exp();
extern "C" void* dll_fflush();
extern "C" void* floor();
extern "C" void* dll_fprintf();
extern "C" void* dllfree();
extern "C" void* frexp();
extern "C" void* fwrite();
extern "C" void* gmtime();
extern "C" void* ldexp();
extern "C" void* localtime();
extern "C" void* log();
extern "C" void* log10();
extern "C" void* dllmalloc();
extern "C" void* memcpy();
extern "C" void* memmove();
extern "C" void* memset();
extern "C" void* mktime();
extern "C" void* dllperror();
extern "C" void* dllprintf();
extern "C" void* putchar();
extern "C" void* dllputs();
extern "C" void* qsort();
extern "C" void* dllrealloc();
extern "C" void* sin();
extern "C" void* sinh();
extern "C" void* sprintf();
extern "C" void* sqrt();
extern "C" void* sscanf();
extern "C" void* strchr();
extern "C" void* strcmp();
extern "C" void* strcpy();
extern "C" void* strlen();
extern "C" void* strncpy();
extern "C" void* strrchr();
extern "C" void* strtod();
extern "C" void* strtok();
extern "C" void* strtol();
extern "C" void* strtoul();
extern "C" void* tan();
extern "C" void* tanh();
extern "C" void* time();
extern "C" void* toupper();
extern "C" void* _memccpy();
extern "C" void* dll_fstat();
extern "C" void* dll_mkdir();
extern "C" void* _pclose();
extern "C" void* _popen();
extern "C" void* dll_sleep();
extern "C" void* dll_stat();
extern "C" void* dll_strdup();
extern "C" void* _swab();
extern "C" void* _findclose();
extern "C" void* dll_findfirst();
extern "C" void* _findnext();
extern "C" void* _fullpath();
extern "C" void* _pctype();
extern "C" void* dllcalloc();
extern "C" void* ceil();
extern "C" void* ctime();
extern "C" void* dllexit();
extern "C" void* dll_fclose();
extern "C" void* dll_feof();
extern "C" void* dll_fgets();
extern "C" void* dll_fopen();
extern "C" void* dll_fputc();
extern "C" void* dll_fputs();
extern "C" void* dll_fread();
extern "C" void* dll_fseek();
extern "C" void* dll_ftell();
extern "C" void* dll_getc();
extern "C" void* dll_getenv();
extern "C" void* putc();
extern "C" void* rand();
extern "C" void* remove();
extern "C" void* rewind();
extern "C" void* setlocale();
extern "C" void* dll_signal();
extern "C" void* srand();
extern "C" void* strcat();
extern "C" void* strcoll();
extern "C" void* dllstrerror();
extern "C" void* strncat();
extern "C" void* strncmp();
extern "C" void* strpbrk();
extern "C" void* strstr();
extern "C" void* tolower();
extern "C" void* acos();
extern "C" void* atan();
extern "C" void* memchr();
extern "C" void* dll_getc();
extern "C" void* _CIpow();
extern "C" int _purecall();
extern "C" void* _adjust_fdiv();
extern "C" void* dll_initterm();
extern "C" void* swscanf();
extern "C" void* dllfree();
extern "C" void* iswspace();
extern "C" void* wcscmp();
extern "C" void* dll_vfprintf();
extern "C" void* vsprintf();
extern "C" void* longjmp();
extern "C" void* _ftol();
extern "C" void* strspn();
extern "C" void* strcspn();
extern "C" void* dll_fgetpos();
extern "C" void* dll_fsetpos();
extern "C" void* dll_stati64();
extern "C" void* dll_fstati64();
extern "C" void* dll_telli64();
extern "C" void* dll_tell();
extern "C" void* dll_setmode();
extern "C" void* dll_beginthreadex();
extern "C" void* dll_fileno();
extern "C" void* dll_getcwd();
extern "C" void* _isatty();
extern "C" void* dll_putenv();
extern "C" void* _atoi64();
extern "C" void* dll_ctype();
extern "C" void* _filbuf();
extern "C" void* _fmode();
extern "C" int _setjmp(int);
extern "C" void* asin();
extern "C" void* atol();
extern "C" void* atol();
extern "C" void* bsearch();
extern "C" void* dll_ferror();
extern "C" void* freopen();
extern "C" void* fscanf();
extern "C" void* localeconv();
extern "C" void* raise();
extern "C" void* setvbuf();
extern "C" void* strftime();
extern "C" void* strxfrm();
extern "C" void* dll_ungetc();
extern "C" void* dll_fdopen();
extern "C" void* dll_system();
extern "C" void* _flsbuf();
extern "C" void* isdigit();
extern "C" void* isalnum();
extern "C" void* isxdigit();
extern "C" void* pow();
extern "C" void* dll_onexit();
extern "C" void* modf();
extern "C" void* _get_osfhandle();
//extern "C" void* _CxxThrowException();
extern "C" void* _itoa();
extern "C" void* memcmp();
extern "C" void* _except_handler3();
extern "C" void* __CxxFrameHandler();
extern "C" void* abort();
extern "C" void* free();
extern "C" void* malloc();
extern "C" void* perror();
extern "C" void* realloc();
extern "C" void* _strdup();
extern "C" void* calloc();
extern "C" void* exit();
extern "C" void* strerror();
extern "C" void* strcmpi();
extern "C" void* fabs();
extern "C" void* dllmalloc71();
extern "C" void* dllfree71();
extern "C" void* wcslen();
extern "C" void* _wcsicmp();
extern "C" void* _wcsnicmp();
extern "C" void* _CIacos();
extern "C" void* _CIasin();
extern "C" void* dllfree71();
extern "C" void* isalpha();
extern "C" void* _setjmp3();
extern "C" void* isprint();
extern "C" void* abs();
extern "C" void* labs();
extern "C" void* islower();
extern "C" void* isupper();
extern "C" void* wcscoll();
extern "C" void* _CIsinh();
extern "C" void* _CIcosh();
extern "C" void* _isnan();
extern "C" void* _finite();
extern "C" void* _CIfmod();
extern "C" void* atan2();
extern "C" void* fmod();
extern "C" void* _endthread();
extern "C" void* _beginthread();
extern "C" void* clock();
extern "C" void* _hypot();
extern "C" void* asctime();
extern "C" void* __security_error_handler();
extern "C" void* __CppXcptFilter();
extern "C" void* _tzset();
extern "C" void* _tzname();
extern "C" void* _daylight();
extern "C" void* _timezone();
extern "C" void* _sys_nerr();
extern "C" void* _sys_errlist();
extern "C" void* dll_getpid();
extern "C" void* _HUGE();
extern "C" void* isspace();
extern "C" void* dll_fwrite();
extern "C" void* _stat();
extern "C" void* dll_findnext();
extern "C" void* fsetpos();

// tracker functions
extern "C" void* track_close();
extern "C" void* track_open();
extern "C" void* track_free();
extern "C" void* track_malloc();
extern "C" void* track_realloc();
extern "C" void* track_strdup();
extern "C" void* track_calloc();
extern "C" void* track_fclose();
extern "C" void* track_fopen();
extern "C" void* track_freopen();

void export_msvcrt()
{
  g_dlls.msvcrt.AddExport("_close", (unsigned long)dll_close, (void*)track_close);
  g_dlls.msvcrt.AddExport("_lseek", (unsigned long)dll_lseek);
  g_dlls.msvcrt.AddExport("_read", (unsigned long)dll_read);
  g_dlls.msvcrt.AddExport("_write", (unsigned long)dll_write);
  g_dlls.msvcrt.AddExport("__dllonexit", (unsigned long)dll__dllonexit);
  g_dlls.msvcrt.AddExport("__mb_cur_max", (unsigned long)__mb_cur_max);
  g_dlls.msvcrt.AddExport("_assert", (unsigned long)_assert);
  g_dlls.msvcrt.AddExport("_errno", (unsigned long)_errno);
  g_dlls.msvcrt.AddExport("_ftime", (unsigned long)_ftime);
  g_dlls.msvcrt.AddExport("_iob", (unsigned long)_iob);
  g_dlls.msvcrt.AddExport("_isctype", (unsigned long)_isctype);
  g_dlls.msvcrt.AddExport("_lseeki64", (unsigned long)dll_lseeki64);
  g_dlls.msvcrt.AddExport("_open", (unsigned long)dll_open, (void*)track_open);
  g_dlls.msvcrt.AddExport("_snprintf", (unsigned long)_snprintf);
  g_dlls.msvcrt.AddExport("_stricmp", (unsigned long)_stricmp);
  g_dlls.msvcrt.AddExport("_strnicmp", (unsigned long)_strnicmp);
  g_dlls.msvcrt.AddExport("_vsnprintf", (unsigned long)_vsnprintf);
  g_dlls.msvcrt.AddExport("abort", (unsigned long)dllabort);
  g_dlls.msvcrt.AddExport("atof", (unsigned long)atof);
  g_dlls.msvcrt.AddExport("atoi", (unsigned long)atoi);
  g_dlls.msvcrt.AddExport("cos", (unsigned long)cos);
  g_dlls.msvcrt.AddExport("cosh", (unsigned long)cosh);
  g_dlls.msvcrt.AddExport("exp", (unsigned long)exp);
  g_dlls.msvcrt.AddExport("fflush", (unsigned long)dll_fflush);
  g_dlls.msvcrt.AddExport("floor", (unsigned long)floor);
  g_dlls.msvcrt.AddExport("fprintf", (unsigned long)dll_fprintf);
  g_dlls.msvcrt.AddExport("free", (unsigned long)dllfree, (void*)track_free);
  g_dlls.msvcrt.AddExport("frexp", (unsigned long)frexp);
  g_dlls.msvcrt.AddExport("fwrite", (unsigned long)fwrite);
  g_dlls.msvcrt.AddExport("gmtime", (unsigned long)gmtime);
  g_dlls.msvcrt.AddExport("ldexp", (unsigned long)ldexp);
  g_dlls.msvcrt.AddExport("localtime", (unsigned long)localtime);
  g_dlls.msvcrt.AddExport("log", (unsigned long)log);
  g_dlls.msvcrt.AddExport("log10", (unsigned long)log10);
  g_dlls.msvcrt.AddExport("malloc", (unsigned long)dllmalloc, (void*)track_malloc);
  g_dlls.msvcrt.AddExport("memcpy", (unsigned long)memcpy);
  g_dlls.msvcrt.AddExport("memmove", (unsigned long)memmove);
  g_dlls.msvcrt.AddExport("memset", (unsigned long)memset);
  g_dlls.msvcrt.AddExport("mktime", (unsigned long)mktime);
  g_dlls.msvcrt.AddExport("perror", (unsigned long)dllperror);
  g_dlls.msvcrt.AddExport("printf", (unsigned long)dllprintf);
  g_dlls.msvcrt.AddExport("putchar", (unsigned long)putchar);
  g_dlls.msvcrt.AddExport("puts", (unsigned long)dllputs);
  g_dlls.msvcrt.AddExport("qsort", (unsigned long)qsort);
  g_dlls.msvcrt.AddExport("realloc", (unsigned long)dllrealloc, (void*)track_realloc);
  g_dlls.msvcrt.AddExport("sin", (unsigned long)sin);
  g_dlls.msvcrt.AddExport("sinh", (unsigned long)sinh);
  g_dlls.msvcrt.AddExport("sprintf", (unsigned long)sprintf);
  g_dlls.msvcrt.AddExport("sqrt", (unsigned long)sqrt);
  g_dlls.msvcrt.AddExport("sscanf", (unsigned long)sscanf);
  g_dlls.msvcrt.AddExport("strchr", (unsigned long)strchr);
  g_dlls.msvcrt.AddExport("strcmp", (unsigned long)strcmp);
  g_dlls.msvcrt.AddExport("strcpy", (unsigned long)strcpy);
  g_dlls.msvcrt.AddExport("strlen", (unsigned long)strlen);
  g_dlls.msvcrt.AddExport("strncpy", (unsigned long)strncpy);
  g_dlls.msvcrt.AddExport("strrchr", (unsigned long)strrchr);
  g_dlls.msvcrt.AddExport("strtod", (unsigned long)strtod);
  g_dlls.msvcrt.AddExport("strtok", (unsigned long)strtok);
  g_dlls.msvcrt.AddExport("strtol", (unsigned long)strtol);
  g_dlls.msvcrt.AddExport("strtoul", (unsigned long)strtoul);
  g_dlls.msvcrt.AddExport("tan", (unsigned long)tan);
  g_dlls.msvcrt.AddExport("tanh", (unsigned long)tanh);
  g_dlls.msvcrt.AddExport("time", (unsigned long)time);
  g_dlls.msvcrt.AddExport("toupper", (unsigned long)toupper);
  g_dlls.msvcrt.AddExport("_memccpy", (unsigned long)_memccpy);
  g_dlls.msvcrt.AddExport("_fstat", (unsigned long)dll_fstat);
  g_dlls.msvcrt.AddExport("_mkdir", (unsigned long)dll_mkdir);
  g_dlls.msvcrt.AddExport("_pclose", (unsigned long)_pclose);
  g_dlls.msvcrt.AddExport("_popen", (unsigned long)_popen);
  g_dlls.msvcrt.AddExport("_sleep", (unsigned long)dll_sleep);
  g_dlls.msvcrt.AddExport("_stat", (unsigned long)dll_stat);
  g_dlls.msvcrt.AddExport("_strdup", (unsigned long)dll_strdup, (void*)track_strdup);
  g_dlls.msvcrt.AddExport("_swab", (unsigned long)_swab);
  g_dlls.msvcrt.AddExport("_findclose", (unsigned long)_findclose);
  g_dlls.msvcrt.AddExport("_findfirst", (unsigned long)dll_findfirst);
  g_dlls.msvcrt.AddExport("_findnext", (unsigned long)_findnext);
  g_dlls.msvcrt.AddExport("_fullpath", (unsigned long)_fullpath);
  g_dlls.msvcrt.AddExport("_pctype", (unsigned long)_pctype);
  g_dlls.msvcrt.AddExport("calloc", (unsigned long)dllcalloc, (void*)track_calloc);
  g_dlls.msvcrt.AddExport("ceil", (unsigned long)ceil);
  g_dlls.msvcrt.AddExport("ctime", (unsigned long)ctime);
  g_dlls.msvcrt.AddExport("exit", (unsigned long)dllexit);
  g_dlls.msvcrt.AddExport("fclose", (unsigned long)dll_fclose, (void*)track_fclose);
  g_dlls.msvcrt.AddExport("feof", (unsigned long)dll_feof);
  g_dlls.msvcrt.AddExport("fgets", (unsigned long)dll_fgets);
  g_dlls.msvcrt.AddExport("fopen", (unsigned long)dll_fopen, (void*)track_fopen);
  g_dlls.msvcrt.AddExport("fputc", (unsigned long)dll_fputc);
  g_dlls.msvcrt.AddExport("fputs", (unsigned long)dll_fputs);
  g_dlls.msvcrt.AddExport("fread", (unsigned long)dll_fread);
  g_dlls.msvcrt.AddExport("fseek", (unsigned long)dll_fseek);
  g_dlls.msvcrt.AddExport("ftell", (unsigned long)dll_ftell);
  g_dlls.msvcrt.AddExport("getc", (unsigned long)dll_getc);
  g_dlls.msvcrt.AddExport("getenv", (unsigned long)dll_getenv);
  g_dlls.msvcrt.AddExport("putc", (unsigned long)putc);
  g_dlls.msvcrt.AddExport("rand", (unsigned long)rand);
  g_dlls.msvcrt.AddExport("remove", (unsigned long)remove);
  g_dlls.msvcrt.AddExport("rewind", (unsigned long)rewind);
  g_dlls.msvcrt.AddExport("setlocale", (unsigned long)setlocale);
  g_dlls.msvcrt.AddExport("signal", (unsigned long)dll_signal);
  g_dlls.msvcrt.AddExport("srand", (unsigned long)srand);
  g_dlls.msvcrt.AddExport("strcat", (unsigned long)strcat);
  g_dlls.msvcrt.AddExport("strcoll", (unsigned long)strcoll);
  g_dlls.msvcrt.AddExport("strerror", (unsigned long)dllstrerror);
  g_dlls.msvcrt.AddExport("strncat", (unsigned long)strncat);
  g_dlls.msvcrt.AddExport("strncmp", (unsigned long)strncmp);
  g_dlls.msvcrt.AddExport("strpbrk", (unsigned long)strpbrk);
  g_dlls.msvcrt.AddExport("strstr", (unsigned long)strstr);
  g_dlls.msvcrt.AddExport("tolower", (unsigned long)tolower);
  g_dlls.msvcrt.AddExport("acos", (unsigned long)acos);
  g_dlls.msvcrt.AddExport("atan", (unsigned long)atan);
  g_dlls.msvcrt.AddExport("memchr", (unsigned long)memchr);
  g_dlls.msvcrt.AddExport("fgetc", (unsigned long)dll_getc);
  g_dlls.msvcrt.AddExport("_CIpow", (unsigned long)_CIpow);
  g_dlls.msvcrt.AddExport("_purecall", (unsigned long)_purecall);
  g_dlls.msvcrt.AddExport("_adjust_fdiv", (unsigned long)_adjust_fdiv);
  g_dlls.msvcrt.AddExport("_initterm", (unsigned long)dll_initterm);
  g_dlls.msvcrt.AddExport("swscanf", (unsigned long)swscanf);
  g_dlls.msvcrt.AddExport("??2@YAPAXI@Z", (unsigned long)dllmalloc, (void*)track_malloc);
  g_dlls.msvcrt.AddExport("??3@YAXPAX@Z", (unsigned long)dllfree, (void*)track_free);
  g_dlls.msvcrt.AddExport("iswspace", (unsigned long)iswspace);
  g_dlls.msvcrt.AddExport("wcscmp", (unsigned long)wcscmp);
  g_dlls.msvcrt.AddExport("vfprintf", (unsigned long)dll_vfprintf);
  g_dlls.msvcrt.AddExport("vsprintf", (unsigned long)vsprintf);
  g_dlls.msvcrt.AddExport("longjmp", (unsigned long)longjmp);
  g_dlls.msvcrt.AddExport("_ftol", (unsigned long)_ftol);
  g_dlls.msvcrt.AddExport("strspn", (unsigned long)strspn);
  g_dlls.msvcrt.AddExport("strcspn", (unsigned long)strcspn);
  g_dlls.msvcrt.AddExport("fgetpos", (unsigned long)dll_fgetpos);
  g_dlls.msvcrt.AddExport("fsetpos", (unsigned long)dll_fsetpos);
  g_dlls.msvcrt.AddExport("_stati64", (unsigned long)dll_stati64);
  g_dlls.msvcrt.AddExport("_fstati64", (unsigned long)dll_fstati64);
  g_dlls.msvcrt.AddExport("_telli64", (unsigned long)dll_telli64);
  g_dlls.msvcrt.AddExport("_tell", (unsigned long)dll_tell);
  g_dlls.msvcrt.AddExport("_setmode", (unsigned long)dll_setmode);
  g_dlls.msvcrt.AddExport("_beginthreadex", (unsigned long)dll_beginthreadex);
  g_dlls.msvcrt.AddExport("_fileno", (unsigned long)dll_fileno);
  g_dlls.msvcrt.AddExport("_getcwd", (unsigned long)dll_getcwd);
  g_dlls.msvcrt.AddExport("_isatty", (unsigned long)_isatty);
  g_dlls.msvcrt.AddExport("_putenv", (unsigned long)dll_putenv);
  g_dlls.msvcrt.AddExport("_atoi64", (unsigned long)_atoi64);
  g_dlls.msvcrt.AddExport("_ctype", (unsigned long)dll_ctype);
  g_dlls.msvcrt.AddExport("_filbuf", (unsigned long)_filbuf);
  g_dlls.msvcrt.AddExport("_fmode", (unsigned long)_fmode);
  g_dlls.msvcrt.AddExport("_setjmp", (unsigned long)_setjmp);
  g_dlls.msvcrt.AddExport("asin", (unsigned long)asin);
  g_dlls.msvcrt.AddExport("atol", (unsigned long)atol);
  g_dlls.msvcrt.AddExport("atol", (unsigned long)atol);
  g_dlls.msvcrt.AddExport("bsearch", (unsigned long)bsearch);
  g_dlls.msvcrt.AddExport("ferror", (unsigned long)dll_ferror);
  g_dlls.msvcrt.AddExport("freopen", (unsigned long)freopen, (void*)track_freopen);
  g_dlls.msvcrt.AddExport("fscanf", (unsigned long)fscanf);
  g_dlls.msvcrt.AddExport("localeconv", (unsigned long)localeconv);
  g_dlls.msvcrt.AddExport("raise", (unsigned long)raise);
  g_dlls.msvcrt.AddExport("setvbuf", (unsigned long)setvbuf);
  g_dlls.msvcrt.AddExport("strftime", (unsigned long)strftime);
  g_dlls.msvcrt.AddExport("strxfrm", (unsigned long)strxfrm);
  g_dlls.msvcrt.AddExport("ungetc", (unsigned long)dll_ungetc);
  g_dlls.msvcrt.AddExport("_fdopen", (unsigned long)dll_fdopen);
  g_dlls.msvcrt.AddExport("system", (unsigned long)dll_system);
  g_dlls.msvcrt.AddExport("_flsbuf", (unsigned long)_flsbuf);
  g_dlls.msvcrt.AddExport("isdigit", (unsigned long)isdigit);
  g_dlls.msvcrt.AddExport("isalnum", (unsigned long)isalnum);
  g_dlls.msvcrt.AddExport("isxdigit", (unsigned long)isxdigit);
  g_dlls.msvcrt.AddExport("pow", (unsigned long)pow);
  g_dlls.msvcrt.AddExport("_onexit", (unsigned long)dll_onexit);
  g_dlls.msvcrt.AddExport("modf", (unsigned long)modf);
  g_dlls.msvcrt.AddExport("_get_osfhandle", (unsigned long)_get_osfhandle);
  g_dlls.msvcrt.AddExport("_CxxThrowException", (unsigned long)_CxxThrowException);
  g_dlls.msvcrt.AddExport("_itoa", (unsigned long)_itoa);
  g_dlls.msvcrt.AddExport("memcmp", (unsigned long)memcmp);
  g_dlls.msvcrt.AddExport("_except_handler3", (unsigned long)_except_handler3);
  g_dlls.msvcrt.AddExport("__CxxFrameHandler", (unsigned long)__CxxFrameHandler);
}

void export_msvcr71()
{
  g_dlls.msvcr71.AddExport("_close", (unsigned long)dll_close, (void*)track_close);
  g_dlls.msvcr71.AddExport("_lseek", (unsigned long)dll_lseek);
  g_dlls.msvcr71.AddExport("_read", (unsigned long)dll_read);
  g_dlls.msvcr71.AddExport("_write", (unsigned long)dll_write);
  g_dlls.msvcr71.AddExport("__dllonexit", (unsigned long)dll__dllonexit);
  g_dlls.msvcr71.AddExport("__mb_cur_max", (unsigned long)__mb_cur_max);
  g_dlls.msvcr71.AddExport("_assert", (unsigned long)_assert);
  g_dlls.msvcr71.AddExport("_errno", (unsigned long)_errno);
  g_dlls.msvcr71.AddExport("_ftime", (unsigned long)_ftime);
  g_dlls.msvcr71.AddExport("_iob", (unsigned long)_iob);
  g_dlls.msvcr71.AddExport("_isctype", (unsigned long)_isctype);
  g_dlls.msvcr71.AddExport("_lseeki64", (unsigned long)dll_lseeki64);
  g_dlls.msvcr71.AddExport("_open", (unsigned long)dll_open, (void*)track_open);
  g_dlls.msvcr71.AddExport("_snprintf", (unsigned long)_snprintf);
  g_dlls.msvcr71.AddExport("_stricmp", (unsigned long)_stricmp);
  g_dlls.msvcr71.AddExport("_strnicmp", (unsigned long)_strnicmp);
  g_dlls.msvcr71.AddExport("_vsnprintf", (unsigned long)_vsnprintf);
  g_dlls.msvcr71.AddExport("abort", (unsigned long)abort);
  g_dlls.msvcr71.AddExport("atof", (unsigned long)atof);
  g_dlls.msvcr71.AddExport("atoi", (unsigned long)atoi);
  g_dlls.msvcr71.AddExport("cos", (unsigned long)cos);
  g_dlls.msvcr71.AddExport("cosh", (unsigned long)cosh);
  g_dlls.msvcr71.AddExport("exp", (unsigned long)exp);
  g_dlls.msvcr71.AddExport("fflush", (unsigned long)dll_fflush);
  g_dlls.msvcr71.AddExport("floor", (unsigned long)floor);
  g_dlls.msvcr71.AddExport("fprintf", (unsigned long)dll_fprintf);
  g_dlls.msvcr71.AddExport("free", (unsigned long)free, (void*)track_free);
  g_dlls.msvcr71.AddExport("frexp", (unsigned long)frexp);
  g_dlls.msvcr71.AddExport("fwrite", (unsigned long)dll_fwrite);
  g_dlls.msvcr71.AddExport("gmtime", (unsigned long)gmtime);
  g_dlls.msvcr71.AddExport("ldexp", (unsigned long)ldexp);
  g_dlls.msvcr71.AddExport("localtime", (unsigned long)localtime);
  g_dlls.msvcr71.AddExport("log", (unsigned long)log);
  g_dlls.msvcr71.AddExport("log10", (unsigned long)log10);
  g_dlls.msvcr71.AddExport("malloc", (unsigned long)malloc, (void*)track_malloc);
  g_dlls.msvcr71.AddExport("memcpy", (unsigned long)memcpy);
  g_dlls.msvcr71.AddExport("memmove", (unsigned long)memmove);
  g_dlls.msvcr71.AddExport("memset", (unsigned long)memset);
  g_dlls.msvcr71.AddExport("mktime", (unsigned long)mktime);
  g_dlls.msvcr71.AddExport("perror", (unsigned long)perror);
  g_dlls.msvcr71.AddExport("printf", (unsigned long)dllprintf);
  g_dlls.msvcr71.AddExport("putchar", (unsigned long)putchar);
  g_dlls.msvcr71.AddExport("puts", (unsigned long)dllputs);
  g_dlls.msvcr71.AddExport("qsort", (unsigned long)qsort);
  g_dlls.msvcr71.AddExport("realloc", (unsigned long)realloc, (void*)track_realloc);
  g_dlls.msvcr71.AddExport("sin", (unsigned long)sin);
  g_dlls.msvcr71.AddExport("sinh", (unsigned long)sinh);
  g_dlls.msvcr71.AddExport("sprintf", (unsigned long)sprintf);
  g_dlls.msvcr71.AddExport("sqrt", (unsigned long)sqrt);
  g_dlls.msvcr71.AddExport("sscanf", (unsigned long)sscanf);
  g_dlls.msvcr71.AddExport("strchr", (unsigned long)strchr);
  g_dlls.msvcr71.AddExport("strcmp", (unsigned long)strcmp);
  g_dlls.msvcr71.AddExport("strcpy", (unsigned long)strcpy);
  g_dlls.msvcr71.AddExport("strlen", (unsigned long)strlen);
  g_dlls.msvcr71.AddExport("strncpy", (unsigned long)strncpy);
  g_dlls.msvcr71.AddExport("strrchr", (unsigned long)strrchr);
  g_dlls.msvcr71.AddExport("strtod", (unsigned long)strtod);
  g_dlls.msvcr71.AddExport("strtok", (unsigned long)strtok);
  g_dlls.msvcr71.AddExport("strtol", (unsigned long)strtol);
  g_dlls.msvcr71.AddExport("strtoul", (unsigned long)strtoul);
  g_dlls.msvcr71.AddExport("tan", (unsigned long)tan);
  g_dlls.msvcr71.AddExport("tanh", (unsigned long)tanh);
  g_dlls.msvcr71.AddExport("time", (unsigned long)time);
  g_dlls.msvcr71.AddExport("toupper", (unsigned long)toupper);
  g_dlls.msvcr71.AddExport("_memccpy", (unsigned long)_memccpy);
  g_dlls.msvcr71.AddExport("_fstat", (unsigned long)dll_fstat);
  g_dlls.msvcr71.AddExport("_memccpy", (unsigned long)_memccpy);
  g_dlls.msvcr71.AddExport("_mkdir", (unsigned long)dll_mkdir);
  g_dlls.msvcr71.AddExport("_pclose", (unsigned long)_pclose);
  g_dlls.msvcr71.AddExport("_popen", (unsigned long)_popen);
  g_dlls.msvcr71.AddExport("_sleep", (unsigned long)dll_sleep);
  g_dlls.msvcr71.AddExport("_stat", (unsigned long)_stat);
  g_dlls.msvcr71.AddExport("_strdup", (unsigned long)_strdup, (void*)track_strdup);
  g_dlls.msvcr71.AddExport("_swab", (unsigned long)_swab);
  g_dlls.msvcr71.AddExport("_findclose", (unsigned long)_findclose);
  g_dlls.msvcr71.AddExport("_findfirst", (unsigned long)dll_findfirst);
  g_dlls.msvcr71.AddExport("_findnext", (unsigned long)dll_findnext);
  g_dlls.msvcr71.AddExport("_fullpath", (unsigned long)_fullpath);
  g_dlls.msvcr71.AddExport("_pctype", (unsigned long)_pctype);
  g_dlls.msvcr71.AddExport("calloc", (unsigned long)calloc, (void*)track_calloc);
  g_dlls.msvcr71.AddExport("ceil", (unsigned long)ceil);
  g_dlls.msvcr71.AddExport("ctime", (unsigned long)ctime);
  g_dlls.msvcr71.AddExport("exit", (unsigned long)exit);
  g_dlls.msvcr71.AddExport("fclose", (unsigned long)dll_fclose, (void*)track_fclose);
  g_dlls.msvcr71.AddExport("feof", (unsigned long)dll_feof);
  g_dlls.msvcr71.AddExport("fgets", (unsigned long)dll_fgets);
  g_dlls.msvcr71.AddExport("fopen", (unsigned long)dll_fopen, (void*)track_fopen);
  g_dlls.msvcr71.AddExport("fputc", (unsigned long)dll_fputc);
  g_dlls.msvcr71.AddExport("fputs", (unsigned long)dll_fputs);
  g_dlls.msvcr71.AddExport("fread", (unsigned long)dll_fread);
  g_dlls.msvcr71.AddExport("fseek", (unsigned long)dll_fseek);
  g_dlls.msvcr71.AddExport("ftell", (unsigned long)dll_ftell);
  g_dlls.msvcr71.AddExport("getc", (unsigned long)dll_getc);
  g_dlls.msvcr71.AddExport("getenv", (unsigned long)dll_getenv);
  g_dlls.msvcr71.AddExport("putc", (unsigned long)putc);
  g_dlls.msvcr71.AddExport("rand", (unsigned long)rand);
  g_dlls.msvcr71.AddExport("remove", (unsigned long)remove);
  g_dlls.msvcr71.AddExport("rewind", (unsigned long)rewind);
  g_dlls.msvcr71.AddExport("setlocale", (unsigned long)setlocale);
  g_dlls.msvcr71.AddExport("signal", (unsigned long)dll_signal);
  g_dlls.msvcr71.AddExport("srand", (unsigned long)srand);
  g_dlls.msvcr71.AddExport("strcat", (unsigned long)strcat);
  g_dlls.msvcr71.AddExport("strcoll", (unsigned long)strcoll);
  g_dlls.msvcr71.AddExport("strerror", (unsigned long)strerror);
  g_dlls.msvcr71.AddExport("strncat", (unsigned long)strncat);
  g_dlls.msvcr71.AddExport("strncmp", (unsigned long)strncmp);
  g_dlls.msvcr71.AddExport("strpbrk", (unsigned long)strpbrk);
  g_dlls.msvcr71.AddExport("strstr", (unsigned long)strstr);
  g_dlls.msvcr71.AddExport("tolower", (unsigned long)tolower);
  g_dlls.msvcr71.AddExport("acos", (unsigned long)acos);
  g_dlls.msvcr71.AddExport("atan", (unsigned long)atan);
  g_dlls.msvcr71.AddExport("memchr", (unsigned long)memchr);
  g_dlls.msvcr71.AddExport("isdigit", (unsigned long)isdigit);
  g_dlls.msvcr71.AddExport("_strcmpi", (unsigned long)strcmpi);
  g_dlls.msvcr71.AddExport("_CIpow", (unsigned long)_CIpow);
  g_dlls.msvcr71.AddExport("_adjust_fdiv", (unsigned long)_adjust_fdiv);
  g_dlls.msvcr71.AddExport("pow", (unsigned long)pow);
  g_dlls.msvcr71.AddExport("fabs", (unsigned long)fabs);
  g_dlls.msvcr71.AddExport("??2@YAPAXI@Z", (unsigned long)dllmalloc, (void*)track_malloc);
  g_dlls.msvcr71.AddExport("??3@YAXPAX@Z", (unsigned long)dllfree, (void*)track_free);
  g_dlls.msvcr71.AddExport("_beginthreadex", (unsigned long)dll_beginthreadex);
  g_dlls.msvcr71.AddExport("_fdopen", (unsigned long)dll_fdopen);
  g_dlls.msvcr71.AddExport("_fileno", (unsigned long)dll_fileno);
  g_dlls.msvcr71.AddExport("_getcwd", (unsigned long)dll_getcwd);
  g_dlls.msvcr71.AddExport("_isatty", (unsigned long)_isatty);
  g_dlls.msvcr71.AddExport("_putenv", (unsigned long)dll_putenv);
  g_dlls.msvcr71.AddExport("_atoi64", (unsigned long)_atoi64);
  g_dlls.msvcr71.AddExport("_ctype", (unsigned long)dll_ctype);
  g_dlls.msvcr71.AddExport("_filbuf", (unsigned long)_filbuf);
  g_dlls.msvcr71.AddExport("_fmode", (unsigned long)_fmode);
  g_dlls.msvcr71.AddExport("_setjmp", (unsigned long)_setjmp);
  g_dlls.msvcr71.AddExport("asin", (unsigned long)asin);
  g_dlls.msvcr71.AddExport("atol", (unsigned long)atol);
  g_dlls.msvcr71.AddExport("atol", (unsigned long)atol);
  g_dlls.msvcr71.AddExport("bsearch", (unsigned long)bsearch);
  g_dlls.msvcr71.AddExport("ferror", (unsigned long)dll_ferror);
  g_dlls.msvcr71.AddExport("freopen", (unsigned long)freopen);
  g_dlls.msvcr71.AddExport("fscanf", (unsigned long)fscanf);
  g_dlls.msvcr71.AddExport("localeconv", (unsigned long)localeconv);
  g_dlls.msvcr71.AddExport("raise", (unsigned long)raise);
  g_dlls.msvcr71.AddExport("setvbuf", (unsigned long)setvbuf);
  g_dlls.msvcr71.AddExport("strftime", (unsigned long)strftime);
  g_dlls.msvcr71.AddExport("strxfrm", (unsigned long)strxfrm);
  g_dlls.msvcr71.AddExport("ungetc", (unsigned long)dll_ungetc);
  g_dlls.msvcr71.AddExport("system", (unsigned long)dll_system);
  g_dlls.msvcr71.AddExport("_flsbuf", (unsigned long)_flsbuf);
  g_dlls.msvcr71.AddExport("strspn", (unsigned long)strspn);
  g_dlls.msvcr71.AddExport("strcspn", (unsigned long)strcspn);
  g_dlls.msvcr71.AddExport("wcslen", (unsigned long)wcslen);
  g_dlls.msvcr71.AddExport("_wcsicmp", (unsigned long)_wcsicmp);
  g_dlls.msvcr71.AddExport("fgetpos", (unsigned long)dll_fgetpos);
  g_dlls.msvcr71.AddExport("_wcsnicmp", (unsigned long)_wcsnicmp);
  g_dlls.msvcr71.AddExport("_CIacos", (unsigned long)_CIacos);
  g_dlls.msvcr71.AddExport("_CIasin", (unsigned long)_CIasin);
  g_dlls.msvcr71.AddExport("??_V@YAXPAX@Z", (unsigned long)dllfree, (void*)track_free);
  g_dlls.msvcr71.AddExport("isalpha", (unsigned long)isalpha);
  g_dlls.msvcr71.AddExport("_CxxThrowException", (unsigned long)_CxxThrowException);
  g_dlls.msvcr71.AddExport("__CxxFrameHandler", (unsigned long)__CxxFrameHandler);
  g_dlls.msvcr71.AddExport("memcmp", (unsigned long)memcmp);
  g_dlls.msvcr71.AddExport("fsetpos", (unsigned long)fsetpos);
  g_dlls.msvcr71.AddExport("_setjmp3", (unsigned long)_setjmp3);
  g_dlls.msvcr71.AddExport("longjmp", (unsigned long)longjmp);
  g_dlls.msvcr71.AddExport("isprint", (unsigned long)isprint);
  g_dlls.msvcr71.AddExport("vsprintf", (unsigned long)vsprintf);
  g_dlls.msvcr71.AddExport("abs", (unsigned long)abs);
  g_dlls.msvcr71.AddExport("labs", (unsigned long)labs);
  g_dlls.msvcr71.AddExport("islower", (unsigned long)islower);
  g_dlls.msvcr71.AddExport("isupper", (unsigned long)isupper);
  g_dlls.msvcr71.AddExport("wcscoll", (unsigned long)wcscoll);
  g_dlls.msvcr71.AddExport("_CIsinh", (unsigned long)_CIsinh);
  g_dlls.msvcr71.AddExport("_CIcosh", (unsigned long)_CIcosh);
  g_dlls.msvcr71.AddExport("modf", (unsigned long)modf);
  g_dlls.msvcr71.AddExport("_isnan", (unsigned long)_isnan);
  g_dlls.msvcr71.AddExport("_finite", (unsigned long)_finite);
  g_dlls.msvcr71.AddExport("_CIfmod", (unsigned long)_CIfmod);
  g_dlls.msvcr71.AddExport("atan2", (unsigned long)atan2);
  g_dlls.msvcr71.AddExport("fmod", (unsigned long)fmod);
  g_dlls.msvcr71.AddExport("isxdigit", (unsigned long)isxdigit);
  g_dlls.msvcr71.AddExport("_endthread", (unsigned long)_endthread);
  g_dlls.msvcr71.AddExport("_beginthread", (unsigned long)_beginthread);
  g_dlls.msvcr71.AddExport("clock", (unsigned long)clock);
  g_dlls.msvcr71.AddExport("_hypot", (unsigned long)_hypot);
  g_dlls.msvcr71.AddExport("_except_handler3", (unsigned long)_except_handler3);
  g_dlls.msvcr71.AddExport("asctime", (unsigned long)asctime);
  g_dlls.msvcr71.AddExport("__security_error_handler", (unsigned long)__security_error_handler);
  g_dlls.msvcr71.AddExport("__CppXcptFilter", (unsigned long)__CppXcptFilter);
  g_dlls.msvcr71.AddExport("_tzset", (unsigned long)_tzset);
  g_dlls.msvcr71.AddExport("_tzname", (unsigned long)&_tzname);
  g_dlls.msvcr71.AddExport("_daylight", (unsigned long)&_daylight);
  g_dlls.msvcr71.AddExport("_timezone", (unsigned long)&_timezone);
  g_dlls.msvcr71.AddExport("_sys_nerr", (unsigned long)&_sys_nerr);
  g_dlls.msvcr71.AddExport("_sys_errlist", (unsigned long)&_sys_errlist);
  g_dlls.msvcr71.AddExport("_getpid", (unsigned long)dll_getpid);
  g_dlls.msvcr71.AddExport("_exit", (unsigned long)dllexit);
  g_dlls.msvcr71.AddExport("_onexit", (unsigned long)dll_onexit);
  g_dlls.msvcr71.AddExport("_HUGE", (unsigned long)_HUGE);
  g_dlls.msvcr71.AddExport("_initterm", (unsigned long)dll_initterm);
  g_dlls.msvcr71.AddExport("_purecall", (unsigned long)_purecall);
  g_dlls.msvcr71.AddExport("isalnum", (unsigned long)isalnum);
  g_dlls.msvcr71.AddExport("isspace", (unsigned long)isspace);
}

void export_pncrt()
{
  g_dlls.pncrt.AddExport("malloc", (unsigned long)malloc, (void*)track_malloc);
  g_dlls.pncrt.AddExport("??3@YAXPAX@Z", (unsigned long)free, (void*)track_free);
  g_dlls.pncrt.AddExport("memmove", (unsigned long)memmove);
  g_dlls.pncrt.AddExport("_purecall", (unsigned long)_purecall);
  g_dlls.pncrt.AddExport("_ftol", (unsigned long)_ftol);
  g_dlls.pncrt.AddExport("_CIpow", (unsigned long)_CIpow);
  g_dlls.pncrt.AddExport("??2@YAPAXI@Z", (unsigned long)malloc, (void*)track_malloc);
  g_dlls.pncrt.AddExport("free", (unsigned long)free, (void*)track_free);
  g_dlls.pncrt.AddExport("_initterm", (unsigned long)dll_initterm);
  g_dlls.pncrt.AddExport("_adjust_fdiv", (unsigned long)&_adjust_fdiv);
  g_dlls.pncrt.AddExport("_beginthreadex", (unsigned long)dll_beginthreadex);
  g_dlls.pncrt.AddExport("_iob", (unsigned long)&_iob);
  g_dlls.pncrt.AddExport("fprintf", (unsigned long)dll_fprintf);
  g_dlls.pncrt.AddExport("floor", (unsigned long)floor);
  g_dlls.pncrt.AddExport("_assert", (unsigned long)_assert);
  g_dlls.pncrt.AddExport("__dllonexit", (unsigned long)dll__dllonexit);
  g_dlls.pncrt.AddExport("calloc", (unsigned long)calloc, (void*)track_calloc);
  g_dlls.pncrt.AddExport("strncpy", (unsigned long)strncpy);
  g_dlls.pncrt.AddExport("ldexp", (unsigned long)ldexp);
  g_dlls.pncrt.AddExport("frexp", (unsigned long)frexp);
  g_dlls.pncrt.AddExport("rand", (unsigned long)rand);
}
