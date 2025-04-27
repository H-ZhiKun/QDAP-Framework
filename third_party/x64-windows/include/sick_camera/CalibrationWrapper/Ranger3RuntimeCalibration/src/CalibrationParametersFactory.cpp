#include "../include/CalibrationParametersFactory.h"
#include "../../tinyxml2/tinyxml2.h"
#include <stdexcept>

using namespace tinyxml2;
using namespace ranger3calibration;

CalibrationParametersFactory::CalibrationParametersFactory()
{
}


void CalibrationParametersFactory::loadFromFile(
	const std::string & pathToCalibrationXML,
	CalibrationParameters & outParameters)
{

	XMLDocument doc;
	XMLError err = doc.LoadFile(pathToCalibrationXML.c_str());
	if (err != XML_SUCCESS) {
		throw std::domain_error(doc.ErrorName());
	}

	XMLElement* root = doc.FirstChildElement("COORDINATOR_CALIBRATION");
	XMLElement* rootAlt = doc.FirstChildElement("EasyRangerCalibration");
	if (rootAlt != nullptr) {
		root = rootAlt;
	}
	else if (root == nullptr) {
		throw std::domain_error("COORDINATOR_CALIBRATION Element not found in XML");
	}

	const char* version = root->Attribute("version");
	//Validate that the correct version of calibration is being loaded. 
	if (*version == '1') {
		double data;
		outParameters.version = std::atoi(version);

		//Extract the HOMOGRAPHY element and read the values into the struct.
		tinyxml2::XMLElement* homography = root->FirstChildElement("HOMOGRAPHY")->FirstChildElement();
		readOutHomography(homography, outParameters.H);

		//Read out optical center parameters
		tinyxml2::XMLElement* optcenter = root->FirstChildElement("OPTICAL_CENTER");
		optcenter->FirstChildElement("U")->QueryDoubleText(&data);
		outParameters.opticalCenter[0] = data;

		optcenter->FirstChildElement("V")->QueryDoubleText(&data);
		outParameters.opticalCenter[1] = data;


		// The misspelling of Distortion is intentional to ensure backwards compatability.
		tinyxml2::XMLElement* lensdist = root->FirstChildElement("LENS_DISTORSION")->FirstChildElement();
		for (size_t i = 0; i < 4; ++i) {
			lensdist->QueryDoubleText(&data);
			outParameters.lensDistortion[i] = data;
			lensdist = lensdist->NextSiblingElement();
		}
	}
	else if (*version == '2') {
		double data;
		outParameters.version = std::atoi(version);

		//Extract the HOMOGRAPHY element and read the values into the struct.
		tinyxml2::XMLElement* homography = root->FirstChildElement("Homography")->FirstChildElement();
		readOutHomography(homography, outParameters.H);

		//Read out optical center parameters
		tinyxml2::XMLElement* lens = root->FirstChildElement("Lens");
		lens->FirstChildElement("centerx")->QueryDoubleText(&data);
		outParameters.opticalCenter[0] = data;

		lens->FirstChildElement("centery")->QueryDoubleText(&data);
		outParameters.opticalCenter[1] = data;

		//Skip two elements, (centerx/y)
		lens = root->FirstChildElement("Lens")->FirstChildElement();
		lens = lens->NextSiblingElement();
		//readout r1,r2,t1,t2
		for (size_t i = 0; i < 4; ++i) {
			lens = lens->NextSiblingElement();
			lens->QueryDoubleText(&data);
			outParameters.lensDistortion[i] = data;
		}

		tinyxml2::XMLElement* scheimp = root->FirstChildElement("Scheimpflug");
		scheimp->FirstChildElement("tilt")->QueryDoubleText(&data);
		outParameters.scheimpflugTilt = data;
		scheimp->FirstChildElement("roll")->QueryDoubleText(&data);
		outParameters.scheimpflugRoll = data;

		tinyxml2::XMLElement* metrics = root->FirstChildElement("Metrics");
		metrics->FirstChildElement("coverage")->QueryDoubleText(&data);
		outParameters.coverage = data;
	}
}

CalibrationParametersFactory::~CalibrationParametersFactory()
{
	
}

void ranger3calibration::CalibrationParametersFactory::readOutHomography(
	XMLElement * homography,
	double * H)
{
	double data;
	for (size_t i = 0; i < 3; ++i) {
		for (size_t j = 0; j < 3; ++j) {
			homography->QueryDoubleText(&data);
			H[3 * i + j] = data; // TODO Conversion from double in XML to Float in code. Possible loss of precision. 
			homography = homography->NextSiblingElement();
		}
	}
}
