/* Copyright 2014-2018 Baumer Optronic */

#ifndef TLCONSUMER_BGAPI2_GENICAM_BGAPI2_DEF_H_
#define TLCONSUMER_BGAPI2_GENICAM_BGAPI2_DEF_H_

#include "bgapi2_types.h"

class CEventPnPObj;
class CEventDeviceEventObj;
class CSegmentInfoObj;
class CImageInfoObj;

namespace BGAPI2 {

class BGAPI2_DECL String {
 public:
    String();
    String(const char* text);
    String(const char* text, int length);
    String(const String& Obj);
    ~String();

    operator const char *() const;
    bool operator == (const char* text) const;
    bool operator == (const String& ExStr) const;
    bool operator != (const char* text) const;
    bool operator != (const String& ExStr) const;
    bool operator < (const String& ExStr) const;
    const String& operator = (const char);
    const String& operator = (const char *);
    const String& operator = (const String & ExStr);

    const char* get() const;

    void set(const char* text);
    int size() const;

 private:
    void* data;
};

struct _sInterfaceListData;
typedef struct BGAPI2::_sInterfaceListData tInterfaceListData, *ptInterfaceListData;

struct _sSystemData;
typedef struct BGAPI2::_sSystemData tSystemData, *ptSystemData;

struct _sDeviceListData;
typedef struct BGAPI2::_sDeviceListData tDeviceListData, *ptDeviceListData;

struct _sDataStreamListData;
typedef struct BGAPI2::_sDataStreamListData tDataStreamListData, *ptDataStreamListData;

struct _sDataStreamData;
typedef struct BGAPI2::_sDataStreamData tDataStreamData, *ptDataStreamData;

struct _sInterfaceData;
typedef struct BGAPI2::_sInterfaceData tInterfaceData, *ptInterfaceData;

struct _sBrightnessAutoData;
typedef struct BGAPI2::_sBrightnessAutoData tBrightnessAutoData, *ptBrightnessAutoData;

#if defined(_WIN32)
#pragma pack(push, 8)
#endif
    typedef struct {
        bo_ushort *pcRed;       // pointer to hist array offered by user, can be zero
        bo_ushort *pcGreen;     // pointer to hist array offered by user, can be zero
        bo_ushort *pcBlue;      // pointer to hist array offered by user, can be zero
        bo_ushort *pcLuma;      // pointer to hist array offered by user, can be zero
        int   length;
        int * pSizeFilled;      // pointer to fill info for user
        bool ThresEnable;
        int  ThresMin;
        int  ThresMax;
        int* red_under;
        int* red_over;
        int* green_under;
        int* green_over;
        int* blue_under;
        int* blue_over;
    } bo_tHistRecords;
#if defined(_WIN32)
#pragma pack(pop)
#endif

}  // namespace BGAPI2

#endif  // TLCONSUMER_BGAPI2_GENICAM_BGAPI2_DEF_H_
