#include "STLObject.h"

void STLObject::loadSTL(std::string fileName)
{
	std::ifstream STLFile = std::ifstream(fileName, std::ios::binary);
	if(STLFile.is_open() == false)
	{
		throw std::runtime_error("File does not exist within directory");
	}
	char fileBeg[5];
	STLFile.read(fileBeg, 5);
	STLFile.seekg(0, STLFile.beg);
	std::string solidHeader;
	for(int i = 0; i < 5; i++)
	{
		solidHeader.push_back(fileBeg[i]);
	}
	//ASCII format for STL
	if(solidHeader.compare("solid") == 0)
	{
		if(STLFile.is_open() == false)
		{
			throw std::runtime_error("File does not exist within directory");
		}
		while(1)
		{
			std::string lineHeader;
			std::getline(STLFile, lineHeader);
			if(lineHeader.empty() == false)
			{
				lineHeader.pop_back();
			}
			else
			{
			}
			int spaceIndex = lineHeader.find(' ');
			if(spaceIndex == -1)
			{
				spaceIndex = lineHeader.size();
			}
			std::string stlComponent = lineHeader.substr(0, spaceIndex);
			if(stlComponent.compare("vertex") == 0)
			{
				std::string vertex = lineHeader.substr(spaceIndex + 1);
				setVertex(vertex);
			}
			else if(stlComponent.compare("facet") == 0)
			{
				spaceIndex = 13;
				std::string vertex = lineHeader.substr(spaceIndex);
				setNormals(vertex);
				indexOfFace += 1;
				listOfFaces.resize(listOfFaces.size() + 1);
			}
			else if(stlComponent.compare("solid") == 0 || stlComponent.compare("outer") == 0
				|| stlComponent.compare("endloop") == 0 || stlComponent.compare("endfacet") == 0
				|| stlComponent.compare("endsolid") == 0)
			{
				continue;
			}
			else if(stlComponent[0] == 0)
			{
				indexOfFace = -1;
				break;
			}
			else
			{
				throw std::runtime_error("Not a proper STL file");
			}
		}
	}
	//binary format for STL
	else
	{
		unsigned int fileSize = STLFile.end;
		//bypass first 80 bytes including comments
		char fileHeader[80];
		STLFile.read(fileHeader, 80);
		char faces[4];
		unsigned int numberOfFaces = 1;
		STLFile.read(reinterpret_cast<char*>(&numberOfFaces), sizeof(unsigned int));
		listOfFaces.resize(numberOfFaces);
		int iters = 0;
		while(iters < listOfFaces.size())
		{
			if(iters == 7)
			{
				int ai = 5;
			}
			for(int i = 0; i < 4; i++)
			{
				float p[3];
				STLFile.read(reinterpret_cast<char*>(&p), (sizeof(float) * 3));
				if(!STLFile)
				{
					std::cout << p[0] << " " << p[1] << " " << p[2];
					throw std::runtime_error("eof hit early");
				}
				if(i == 0)
				{
					glm::vec3 vertex = glm::vec3(p[0], p[1], p[2]);
					listOfNormals.push_back(vertex);
					indexOfFace += 1;
				}
				else
				{
					glm::vec3 vertex = glm::vec3(p[0], p[1], p[2]);
					int vertexIndex = findVertex(vertex);
					if(vertexIndex == -1)
					{
						listOfVertices.push_back(vertex);
						listOfIndices.push_back(listOfVertices.size() - 1);
					}
					else
					{
						listOfIndices.push_back(vertexIndex);
					}
					listOfFaces[indexOfFace].push_back(vertex);
				}
			}
			//attribute byte count should be zero
			char attributeByte[2];
			STLFile.read(attributeByte, 2);
			iters += 1;
		}
		indexOfFace = -1;
	}
}

void STLObject::setVertex(std::string lineHeader)
{
	std::vector<float> vertexArray;
	vertexArray.resize(3);
	int coordIndex = 0;
	for(int pos = 0; pos < lineHeader.size();)
	{
		int spaceIndex = lineHeader.find(' ', pos);
		if(spaceIndex == -1)
		{
			spaceIndex = lineHeader.size();
		}
		std::string coord = lineHeader.substr(pos, spaceIndex - pos);
		vertexArray[coordIndex] = std::stof(coord, nullptr);
		pos = spaceIndex + 1;
		coordIndex += 1;
	}
	glm::vec3 vertex = {vertexArray[0], vertexArray[1], vertexArray[2]};
	int vertexIndex = findVertex(vertex);
	if(vertexIndex == -1)
	{
		listOfVertices.push_back(vertex);
		listOfIndices.push_back(listOfVertices.size() - 1);
	}
	else
	{
		listOfIndices.push_back(vertexIndex);
	}
	listOfFaces[indexOfFace].push_back(vertex);
}

void STLObject::setNormals(std::string lineHeader)
{
	std::vector<float> vertexArray;
	vertexArray.resize(3);
	int coordIndex = 0;
	for(int pos = 0; pos < lineHeader.size();)
	{
		int spaceIndex = lineHeader.find(' ', pos);
		if(spaceIndex == -1)
		{
			spaceIndex = lineHeader.size();
		}
		std::string coord = lineHeader.substr(pos, spaceIndex - pos);
		vertexArray[coordIndex] = std::stof(coord, nullptr);
		pos = spaceIndex + 1;
		coordIndex += 1;
	}
	glm::vec3 vertex = {vertexArray[0], vertexArray[1], vertexArray[2]};
	listOfNormals.push_back(vertex);
}

int STLObject::findVertex(glm::vec3 vertex)
{
	for(int i = 0; i < listOfVertices.size(); i++)
	{
		if(listOfVertices[i] == vertex)
		{
			return i;
		}
	}
	return -1;
}

std::vector<glm::vec3> STLObject::getListOfVertices()
{
	return listOfVertices;
}

std::vector<unsigned int> STLObject::getListOfIndices()
{
	return listOfIndices;
}

std::vector<glm::vec3> STLObject::getlistOfNormals()
{
	return listOfNormals;
}

std::vector<std::vector<glm::vec3>> STLObject::getListOfFaces()
{
	return listOfFaces;
}
