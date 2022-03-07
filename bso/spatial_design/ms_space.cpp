#ifndef MS_SPACE_CPP
#define MS_SPACE_CPP

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>
#include <stdexcept>
#include <bitset>

#include <bso/utilities/trim_and_cast.hpp>

namespace bso { namespace spatial_design {

ms_space::ms_space()
{
	this->reset();
} // ms_space()

ms_space::ms_space(unsigned int id, utilities::geometry::vertex coords, utilities::geometry::vector dim, const std::string& spaceType /*= std::string()*/, const std::vector<std::string>& surfaceTypes /*= std::vector<std::string>()*/)
{
	mCoordinates = coords;
	mDimensions = dim;
	mID = id;
	mSpaceType = spaceType;
	mSurfaceTypes = surfaceTypes;
	
	try
	{
		checkValidity();
	}
	catch(std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << std::endl
								 << "The following arguments initialized an invalid ms_space:" << std::endl
								 << "ID: " << id << std::endl
								 << "Coordinates: " << coords.transpose() << std::endl
								 << "Dimensions: " << dim.transpose() << std::endl;
		if (spaceType != "") errorMessage << "spaceType: " << id << std::endl;
		if (!surfaceTypes.empty())
		{
			errorMessage << "SurfaceTypes: ";
			for (auto i = mSurfaceTypes.begin(); i != mSurfaceTypes.end(); i++)
			{
				errorMessage << *i;
				if (std::next(i,1) != mSurfaceTypes.end()) errorMessage << ",";
 			}
			errorMessage << std::endl;
		}			
		errorMessage << "(bso/spatial_design/ms_space.cpp). Got the following error:" << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
} // ms_space()

ms_space::ms_space(std::string line)
{
	// substract sDefMethod
	
	sDefMethod = line.substr (0,1); // space definition method; orthogonal or non-orthogonal
	line.erase(0,2); // Removes the sDefMethod part from the line variable
	
	
	// tokenize the line
	
	boost::char_separator<char> sep(","); // defines what separates tokens in a string
	typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
	t_tokenizer tokens(line, sep); // this is where the tokenized line will be stored
	/*try
	{
		tokens = t_tokenizer(line, sep); // tokenizes the line
	}
	catch (std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << std::endl
								 << "Could not tokenize the following line to initialize an ms_space:" << std::endl
								 << line << std::endl
								 << "(bso/spatial_design/ms_space.cpp). Got the following error:" << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}*/
	
	t_tokenizer::iterator token = tokens.begin(); // set iterator to first token
	int number_of_tokens = std::distance( tokens.begin(), tokens.end()); // count the number of tokens int the line

	
	try 
	{// store values in string line in mDimensions and mCoordinates or pVertex with first value from line variable as mID
		mID = utilities::trim_and_cast_int(*(token));
		utilities::geometry::vertex temp;
				
		for(int i=0; i<((number_of_tokens-1)/3); i++)
		{
			pVertex.push_back(new utilities::geometry::vertex);
					
			temp(0) = utilities::trim_and_cast_double(*(++token));
			temp(1) = utilities::trim_and_cast_double(*(++token));
			temp(2) = utilities::trim_and_cast_double(*(++token));
			
			*pVertex[i] = temp;		
		}
		
				
		if (sDefMethod == "N" || sDefMethod == "n")
		{// handle the tokens for the non-orthogonal cases
			switch (number_of_tokens)
			{
				case 19: // 6 corner polyhedron
				{
					break;
				}
				case 25: // 8 corner polyhedron
				{
					break;
				}
				case 31: // 10 corner polyhedron
				{
					break;
				}
				default:
				{
				std::stringstream errorMessage;
				errorMessage << std::endl
										 << "An invalid amount of input argument was encoutered when trying to initialize the " << sDefMethod << " method defined space with ID " << mID << "." << std::endl
										 << "When trying to parse the following input line: " << std::endl
										 << line << std::endl
										 << "The determined number_of_tokens are: " << number_of_tokens << " and should be: 25" << std::endl
										 << "(bso/spatial_design/ms_space.cpp)" << std::endl;
				throw std::invalid_argument(errorMessage.str());
				
				break;
				}			
			} 
		} 
		else if (sDefMethod == "R" || sDefMethod == "r") //number_of_tokens = 7
		{
			std::cout << "dit is een try out" << std::endl;
			
			mDimensions = *pVertex[0];
			mCoordinates = *pVertex[1];
			pVertex.clear(); // A clearing of values from pVertex to avoid mis use
					
			switch (number_of_tokens)
			{ // handle the tokens for the orthogonal cases
				case 7: // orthogonal regtangal; no space nor surface types have been defined
				{
					break;
				} 
				case 8: // orthogonal regtangal; only a space type has been defined
				{
					mSpaceType = *(++token);
					boost::algorithm::trim(mSpaceType);			
					break; 
				}
				case 13: // orthogonal regtangal; only surface types have been defind
				case 14: // orthogonal regtangal; both a space type and surface types have been defined
				{
					if (number_of_tokens == 14)
					{
						mSpaceType = *(++token);
						boost::algorithm::trim(mSpaceType);
					}
					
					mSurfaceTypes.clear();
					std::string temp;
					
					for (unsigned int i = 0; i < 6; i++)
					{
						temp = *(++token);
						boost::algorithm::trim(temp);
						mSurfaceTypes.push_back(temp);
					}

					break;
				}
				default:
				{
					std::stringstream errorMessage;
					errorMessage << std::endl
											 << "An invalid amount of input argument was encoutered when trying to initialize the " << sDefMethod << " method defined space with ID " << mID << "." << std::endl
											 << "When trying to parse the following input line: " << std::endl
											 << line << std::endl
											 << "(bso/spatial_design/ms_space.cpp)" << std::endl;
					throw std::invalid_argument(errorMessage.str());
					
					break;
				}
			} // switch statement
		} // if statement for sDefMethod == "R" or "r"
	}
	catch (std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << std::endl
								 << "Could not parse the following line to initialize an ms_space:" << std::endl
								 << line << std::endl
								 << "(bso/spatial_design/ms_space.cpp). Got the following error:" << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	
	if (sDefMethod == "R" || sDefMethod == "r") //number_of_tokens = 7
	{
		try
		{
			checkValidity();
		}
		catch(std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << std::endl
									 << "Parsing the following line to initialize leads to an invalid ms_space:" << std::endl
									 << line << std::endl
									 << "(bso/spatial_design/ms_space.cpp). Got the following error:" << std::endl
									 << e.what() << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}
} // ms_space()

ms_space::ms_space(const ms_space& rhs)
{
	mCoordinates  = rhs.mCoordinates;
	mDimensions   = rhs.mDimensions;
	mID           = rhs.mID;
	mSpaceType    = rhs.mSpaceType;
	mSurfaceTypes = rhs.mSurfaceTypes;
	pVertex		  = rhs.pVertex; // Tessa Defined
	sDefMethod	  = rhs.sDefMethod;
	
	if (sDefMethod == "R" || sDefMethod == "r") // the checkValidity test is specifically for the R space type insertion method.
	{
		try
		{
			checkValidity();
		}
		catch(std::exception& e)
		{
			std::stringstream errorMessage;
			errorMessage << std::endl
									 << "Encountered invalid space when copying the space with the following ID:" << rhs.mID << std::endl
									 << "(bso/spatial_design/ms_space.cpp). Got the following error:" << std::endl
									 << e.what() << std::endl;
			throw std::invalid_argument(errorMessage.str());
		}
	}
} // ms_space()

ms_space::~ms_space()
{
	
} // ~ms_space()

void ms_space::reset()
{
	mCoordinates = {0,0,0};
	mDimensions = {0,0,0};
	mID = 0;
	mSpaceType = "";
	mSurfaceTypes.clear();
} // reset()

bool ms_space::checkValidity() const
{
	bool check = mDimensions.minCoeff() < 0;
	if (check)
	{
		std::stringstream errorMessage;
		errorMessage << std::endl
								 << "Found a space with a negative dimension: " << mDimensions.transpose() << std::endl
								 << "Found this in space with ID: " << mID << std::endl
								 << "(bso/spatial_design/ms_space.hpp)" << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
	return check;
}

void ms_space::setCoordinates(const utilities::geometry::vertex& coords)
{
	try
	{
		mCoordinates = coords;
		//checkValidity();
	}
	catch(std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << "Could not set the following coordinates to space with ID \"" << mID << "\": " << coords.transpose() << std::endl
								 << "(bso/spatial_design/ms_space.hpp). Got the following error: " << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
} // setCoordinates()

void ms_space::setDimensions(const utilities::geometry::vector& dims)
{
	try
	{
		mDimensions = dims;
		checkValidity();
	}
	catch(std::exception& e)
	{
		std::stringstream errorMessage;
		errorMessage << "Could not set the following dimensions to space with ID \"" << mID << "\": " << dims.transpose() << std::endl
								 << "(bso/spatial_design/ms_space.hpp). Got the following error: " << std::endl
								 << e.what() << std::endl;
		throw std::invalid_argument(errorMessage.str());
	}
} // setDimensions()

void ms_space::setID(const unsigned int& id)
{
	mID = id;
} // setID()

unsigned int ms_space::getID() const
{
	return mID;
} // getID()

utilities::geometry::vertex ms_space::getCoordinates() const
{
	return mCoordinates;
} // getCoordinates

utilities::geometry::vector ms_space::getDimensions() const
{
	return mDimensions;
} // getDimensions

utilities::geometry::quad_hexahedron ms_space::getGeometry() const
{ // return the geometry of the space
	std::vector<utilities::geometry::vertex> cornerPoints;
	
	if (sDefMethod == "R" || sDefMethod == "r")
	{
		cornerPoints.reserve(8);
		std::vector<unsigned int> vertexOrder = {0,1,3,2,4,5,7,6};
		for (const auto& i : vertexOrder)
		{
			auto tempPoint = mCoordinates;
			for (unsigned int j = 0; j < 3; j++)
			{
				if (std::bitset<3>(i)[j]) tempPoint[j] += mDimensions[j];
			}
			cornerPoints.push_back(tempPoint);
		}
	}
	
	else if (sDefMethod == "N" || sDefMethod == "n")
	{
		for (int i=0; i<pVertex.size(); i++)
		{
			cornerPoints.push_back(*pVertex[i]);
		}
	}
	return bso::utilities::geometry::quad_hexahedron(cornerPoints);
} // getGeometry


bool ms_space::getSpaceType(std::string& spaceType) const
{
	spaceType = mSpaceType;
	return (mSpaceType != "");
} // getSpaceType()

bool ms_space::getSurfaceTypes(std::vector<std::string>& surfaceTypes) const
{
	surfaceTypes = mSurfaceTypes;
	return (!mSurfaceTypes.empty());
} // getSurfaceTypes

double ms_space::getVolume() const
{
	return mDimensions.prod()*1e-9;
} // getVolume()

double ms_space::getFloorArea() const
{
	return mDimensions(0)*mDimensions(1);
} // getFloorArea()

bool ms_space::operator == (const ms_space& rhs) const
{
	if (mID != rhs.mID) return false;
	if (mCoordinates != rhs.mCoordinates) return false;
	if (mDimensions != rhs.mDimensions) return false;
	if (mSpaceType != rhs.mSpaceType) return false;
	if (mSurfaceTypes.size() != rhs.mSurfaceTypes.size()) return false;
	for (unsigned int i = 0; i < mSurfaceTypes.size(); i++)
	{
		if (mSurfaceTypes[i] != rhs.mSurfaceTypes[i]) return false;
	}
	
	return true;
}

bool ms_space::operator != (const ms_space& rhs) const
{
	return !(*this == rhs);
}

std::ostream& operator <<(std::ostream& stream, const ms_space& space)
{
	stream << "R"
				 << "," << space.getID()
				 << "," << space.getDimensions()(0)
				 << "," << space.getDimensions()(1)
				 << "," << space.getDimensions()(2)
				 << "," << space.getCoordinates()(0)
				 << "," << space.getCoordinates()(1)
				 << "," << space.getCoordinates()(2);
	std::string tempStr;
	if (space.getSpaceType(tempStr)) stream << "," << tempStr;
	std::vector<std::string> tempVec;
	if (space.getSurfaceTypes(tempVec)) for (auto i : tempVec) stream << "," << i;
	
	return stream;
}

} // namespace spatial_design
} // namespace bso

#endif // MS_SPACE_CPP