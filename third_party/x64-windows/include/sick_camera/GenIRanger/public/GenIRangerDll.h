// Copyright 2017-2020 SICK AG. All rights reserved.
//#ifdef GENIRANGER_LINKAGE_SHARED_EXPORT
//#  define GENIRANGER_API __declspec(dllexport)
//#elif GENIRANGER_LINKAGE_STATIC
//#  define GENIRANGER_API
//#else
//#  define GENIRANGER_API __declspec(dllimport)
//#endif

#ifdef GENIRANGER_LINKAGE_SHARED_EXPORT
#   define GENIRANGER_API   __declspec(dllexport)
#else
#   define GENIRANGER_API   __declspec(dllimport)
#endif  

