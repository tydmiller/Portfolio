#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <string>

class STLObject
{
public:
	void loadSTL(std::string fileName);
	void setVertex(std::string lineHeader);
	void setNormals(std::string lineHeader);
	int findVertex(glm::vec3 vertex);
	std::vector<glm::vec3> getListOfVertices();
	std::vector<unsigned int> getListOfIndices();
	std::vector<glm::vec3> getlistOfNormals();
	std::vector<std::vector<glm::vec3>> getListOfFaces();

private:
	std::vector<glm::vec3>listOfVertices;
	std::vector<unsigned int> listOfIndices;
	std::vector<glm::vec3> listOfNormals;
	std::vector<std::vector<glm::vec3>>listOfFaces;
	int indexOfFace = -1;
};
