/**	@file	Parameters.h
*
*	@brief	Define Class Selector, Parameter and ParameterVector.
*
*	@attention
*	These classes are used to parameter setting and getting.
*
*
*	@copyright	Copyright 2016-2020 SICK AG. All rights reserved.
*	@author		Vision Lab, SICK GCN
*
*/

#pragma once
#include "Typedef.h"
  

namespace SickCam
{

////////////////////////////////////////////////////////////////////////////////

class Selector
{
public:
	Selector		();
	~Selector		();

	typedef std::shared_ptr<Selector> SelecPtr;
	bool				intiSelector(const std::string & _line, const std::string & son_key_value);
	// from string
	bool				hasParent	()	const;
	bool				isValid		()	const { return !(m_key.empty() || m_value.empty()); };
	SelecPtr			getParent	();
	const std::string&	getKey		()	const { return m_key;		};
	const std::string&	getValue	()	const { return m_value;		};

private:
	std::string		m_key;
	std::string		m_value;		/**< Selcector points to this value	*/
	std::string		m_parentKey;	/**< only one						*/

public:
	static std::map<std::string, SelecPtr>	mapSelectors;
};

typedef std::shared_ptr<Selector> SelecPtr;


////////////////////////////////////////////////////////////////////////////////


/** @brief Class of one parameter, contains some basic info like name \ value \ options etc. */
class EXPORT_TO_DLL Parameter
{
public:
	Parameter	()	
		:	m_name(""), m_category(""), m_selecPtr(nullptr), m_csvName(""), 
		m_strValue(""), m_options(std::vector<std::string>()), m_readOnly(true)
	{}

	~Parameter	()
	{}

	bool initValueFromDevice	(const std::string & _aLine, const std::string & _category);

	const std::string&			getName		()		const { return m_name; };
	const std::string&			getCategory	()		const { return m_category; };
	const std::string&			getCsvName	()		const { return m_csvName; };
	const std::string&			getStrValue	()		const { return m_strValue;				};
	SelecPtr					getSeletor	()			  { return m_selecPtr;				};
	const std::vector<std::string>&	
								getOptions	()		const { return m_options; };
	bool						isValid		()		const { return !m_strValue.empty();		};
	bool						isReadOnly	()		const { return m_readOnly; };

	bool setValueToDevice		(const GenApi::CNodeMapRef & _nodeMap, const std::string & _v)	;
	bool getValueFromDevice		(const GenApi::CNodeMapRef & _nodeMap, std::string & _v);

protected:
	bool modifyValue			(const std::string & _v);
	bool isWithInOptitons		(const std::string & _v) const;
	bool moveSelector			(const GenApi::CNodeMapRef & _nodeMap);


private:
	std::string					m_name;			/**< same name as in Ranger3Studio, as nodeName also				  */
	std::string					m_category;		/**< #name in csv file												  */
	SelecPtr					m_selecPtr;		/**< selectors queue, should be last								  */
	std::string					m_csvName;		/**< same name as in csv file, "NodeName_SelectorKey_SeletorValue"	  */
	std::string					m_strValue;		/**< value in string format											  */
	std::vector<std::string>	m_options;		/**< only used in enum parameter									  */
	bool						m_readOnly;	

};


////////////////////////////////////////////////////////////////////////////////


template<class T> using Ptr = std::shared_ptr<T>;
typedef Ptr<Parameter>									ParaPtr;
typedef std::vector<std::string>						Categories;
typedef std::map<std::string, std::vector<ParaPtr>>		Params;    // Categories_Names, ParaPtr
typedef std::map<std::string, ParaPtr>					AllParams; // Parameters_FullNames, ParaPtr
#ifndef DISABLE_CAL_IN_PC
typedef SiCaliWrapper::RectMethod						RectMethod;
#endif

/** @brief Class to hold and process all parameters.
*
* 
* ## 1 User Parameters & Device Parameters
*
*
* User Parameters | not saved in device
* --------------- | -------------------------
*   RectMethod	  | m_RectMethod;		
*   double		  | m_scaleY;			
*   float		  | m_missingData;		
*   std::string	  | m_CaliPath; 	   		
*   std::string	  | m_ParaPath;	   		
*   uint64_t	  | m_timeOut;			
*
*
* Device Parameters | Saved in device, can be found in csv file
* ------------------|-----------------------
*  DeviceScanType	| ...;		
*  Height_RegionSelector_Region1| ...;			
*  ...				| ...;		
*
*
* ## 2. Mac can not be set.
* ## 3. IP and Subnet both have 2 kinds of setting.
*
* Camera Status	| Special name		| Notes									
* --------------| ------------------| -------------------------------------------
* disconnect	| ForceIP			| Ip and subnet settings is temporary. They will be recoverd when power off.
* connect		| [No special name]	| Ip and subnet settings is permanent. They will be kept when power off.
* 
*
*/																						   
class ParameterVector
{
public:
	ParameterVector		();
	~ParameterVector	();
	
	/** @brief Traverse all available parameters of device.
	*																								
	*	1. Scan parameters of the device.
	*	2. Save all available parameters as <"parameter.tmp"> from device.
	*	3. Parse the file, and initialize Class ParameterVector.
	*	4. Delete <"parameter.tmp">.
	*
	*	@param [in] nodeMap Node map of device parameters.
	*	@param [in] path Temporary saving path.
	*
	*/
	bool ScanDeviceParameters	(GenApi::INodeMap * const nodeMap, const std::string path = "parameter.tmp");

	bool		isValid				()	const { return !m_AllParams.empty();	};
	const Categories& getCategories	()	const {	return	m_Categories;			};
	const Params	& getParams		()	const {	return	m_Params;				};
	const AllParams	& getAllParams	()	const {	return	m_AllParams;			};

	/**	@brief Get Device Parameters
	*
	*	@param [in] nodeMap			Node map of device parameters.
	*	@param [in] ParamterName	The name listed in csv file.
	*	@param [in] value			New value to be set.
	*
	*	@note Available input string names are listed in csv file.
	*
	*/
	bool getParameter(	const GenApi::CNodeMapRef	& nodeMap, 
						const std::string			& ParameterName,
						std::string					& value) const;

	/** @brief	Get IP
	*
	*	@param [in] nodeMap			Node map of device parameters.
	*	@param [in] interfaceHandle	Interface handle of connecting device.
	*	@param [in] tl				Interface api pointer.
	*	@param [in] id				Id of the device in interface.
	*	@param [in] isPersistent	If isPersistent == true, process forceIp; else, process permanent IP setting.
	*/
	std::string getIP(	const GenApi::CNodeMapRef	& nodeMap,
                            const GenTL::IF_HANDLE		& interfaceHandle,
                            const SPtr<GenTLApi>		  tl,
                            const int64_t				& id,
                            const bool					isPersistent) const;

	/** @brief	Get Device subnet
	*
	*	@param [in] nodeMap			Node map of device parameters.
	*	@param [in] interfaceHandle	Interface handle of connecting device.
	*	@param [in] tl				Interface api pointer.
	*	@param [in] id				Id of the device in interface.
	*	@param [in] isPersistent	If isPersistent == true, process forceIp; else, process permanent IP setting.
	*/
	std::string getSubnet(
            const GenApi::CNodeMapRef	& nodeMap,
            const GenTL::IF_HANDLE		& interfaceHandle,
            const SPtr<GenTLApi>		  tl,
            const int64_t				& id,
            const bool					isPersistent) const;

#ifdef __linux__
    std::string getIP(	const GenApi::CNodeMapRef	& nodeMap,
                    const GenTL::IF_HANDLE		& interfaceHandle,
                    const GenTLApi				* tl,
                    const int64_t				& id,
                    const bool					isPersistent) const;
    std::string getSubnet(
						const GenApi::CNodeMapRef	& nodeMap,
						const GenTL::IF_HANDLE		& interfaceHandle,
						const GenTLApi				* tl,
						const int64_t				& id,
						const bool					isPersistent) const;
#endif

	/** @brief setParameter
	*
	*	Set Device Parameters
	*
	*	@note Avaliable input string names are listed in csv file.
	*
	*/
	bool setParameter		(const GenApi::CNodeMapRef & nodeMap, const std::string & paramterName, const std::string & value);

	bool setIP				(const GenApi::CNodeMapRef	& nodeMap, 
							 const GenTL::IF_HANDLE		& interfaceHandle,
							 const SPtr<GenTLApi>		  tl, 
							 const int64_t				& id, 
							 const std::string			& IP, 
							 const bool isPersistent = false);

	bool setSubnet			(const GenApi::CNodeMapRef	& nodeMap, 
							 const GenTL::IF_HANDLE		& interfaceHandle,
							 const SPtr<GenTLApi>		  tl,
							 const int64_t				& id, 
							 const std::string			& subNet, 
							 const bool isPersistent = false);
#ifdef __linux__
    bool setIP				(const GenApi::CNodeMapRef	& nodeMap,
                               const GenTL::IF_HANDLE		& interfaceHandle,
                               const GenTLApi*      		  tl,
                               const int64_t				& id,
                               const std::string			& IP,
                               const bool isPersistent = false);

    bool setSubnet			(const GenApi::CNodeMapRef	& nodeMap,
                               const GenTL::IF_HANDLE		& interfaceHandle,
                               const GenTLApi*      		  tl,
                               const int64_t				& id,
                               const std::string			& subNet,
                               const bool isPersistent = false);
#endif
	Str printParameters		() const;

protected:
	/** Both IP and Subnet are necessary. */
	bool _forceIP			(const GenApi::CNodeMapRef	& nodeMap,
							 const GenTL::IF_HANDLE		& interfaceHandle,
							 const SPtr<GenTLApi>		  tl,
							 const int64_t				& id, 
							 const std::string			& IP, 
							 const std::string			& subNet);

#ifdef __linux__
	bool _forceIP(const GenApi::CNodeMapRef &nodeMap, GenTL::IF_HANDLE const &interfaceHandle, const GenTLApi *tl,
                  const int64_t &id, const std::string &IP, const std::string &subNet);
#endif

	bool _persistentIP		(const GenApi::CNodeMapRef	& nodeMap, 
							 const GenTL::IF_HANDLE		& interfaceHandle,
							 const SPtr<GenTLApi>		  tl,
							 const int64_t				& id,
							 const std::string			& IP, 
							 const std::string			& subNet);

private:
	Categories		m_Categories;
	Params			m_Params;			/**< key is category, for CMD. Need update after seting, base				  */
	AllParams		m_AllParams;		/**< key is csv_name, for device parameters settings. Need update after seting*/
	
public:
	std::string		m_CaliPath	{ "" };			/**< 【In-PC】[Calibration] The path of calibration file. */
	double			m_scaleY	{1.0};			/**< [Calibration] Scale of Y, mm/pixel. */
	float			m_missingData{MISSING_DATA};/**< 【In-PC】[Calibration] Missing data, the default is -1.0e6, defined as MISSING_DATA. */
	bool			m_enableSSE		{false};	/**< 【In-PC】[Calibration] Enabele SSE to accelerate calibration. */
	int				m_ThreadNumber	{ -1 };		/**< 【In-PC】[Calibration] Number of threads will be used in in-PC caliration. -1 means the number of logic cpu cores.*/
	int				m_runMode		{ -1 };		/**< 【In-PC】[Calibration] CPU runmode. -1 default, 0 CPU not support AVX. Please check with CPU-Z. */
	
	bool			m_doRectify	{true};			/**< 【In-PC】[Rectification] Is doing rectification after calibration. */
#ifndef DISABLE_CAL_IN_PC
	RectMethod		m_RectMethod;				/**< [Rectification] Rectification method. */
#endif
	uint32_t		m_RectWidth {2560};			/**< [Rectification] Rectification width, width of output calibrated data. If calibration without rectification, it will be the same as aoiWidth. */
	double			m_RectSpread{ 1.2 };		/**< [Rectification] Rectification spread, splat. */

	std::string		m_ParaPath	{ "" };			/**< [Init] The path of parameter file. */
	std::string		m_TmpPath	{ "" };			/**< [Init] The path of tmp file of scaning device parameters.*/

	uint32_t		m_timeOut	{1};			/**< [Grab] The maximum time of waiting buffer. */
	uint32_t		m_buffersCount{20};			/**< [Grab] The number of pre-allocating buffers. */

};

}

