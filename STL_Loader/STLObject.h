/*
* File: STLObject.h
* Name: Tyler Miller
* Description: creates a STL loader object that will load in STL extension files in ASCII and binary.
*/
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
	/*Loads a STL file based on path given parameter.
 	* Parameters: std::string fileName, the directory that the STL is located
 	* inside of the project directory*/
	void loadSTL(std::string fileName);
	/*Sets vertex based on current data read in from file
 	* Parameter: std::string lineHeader, the current block of data that is currently being red in*/
	void setVertex(std::string lineHeader);
	/*Sets normals of the faces based on data read in.
 	* Parameter: std::string lineHeader, the current block of data that is currently being red in*/
	void setNormals(std::string lineHeader);
	/*Looks to see if vertex has already been read in
 	* Parameter: glm::vec3 vertex, the point being search for*/
	int findVertex(glm::vec3 vertex);
	/*Gets the completed list of vertices read in from the STL file
 	* Return: std::vector<glm::vec3>, the list of vertices from the file*/
	std::vector<glm::vec3> getListOfVertices();
	/*Gets the completed list of indices based on the vertices read in from the STL file
 	* Return: std::vector<unsigned int>, the list of indices*/
	std::vector<unsigned int> getListOfIndices();
	/*Gets the completed list of face normals read in from the STL file
 	* Return: std::vector<unsigned int>, the list of face normals*/
	std::vector<glm::vec3> getlistOfNormals();
	/*Gets the completed list of face read in from the STL file
 	* Return: std::vector<std::vector<glm::vec3>>, the list of vertices triplets*/
	std::vector<std::vector<glm::vec3>> getListOfFaces();

private:
	//List of vertices gotten from file (STL or OBJ)
	std::vector<glm::vec3>listOfVertices;
	//List of indices gotten from file (STL or OBJ)
	std::vector<unsigned int> listOfIndices;
	//List of normals gotten from file (STL or OBJ)
	std::vector<glm::vec3> listOfNormals;
	//Depending on file either faces is gotten from file or created at runtime
	std::vector<std::vector<glm::vec3>>listOfFaces;
	int indexOfFace = -1;
};
