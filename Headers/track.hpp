#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include <shader.hpp>
#include <rc_spline.h>

struct Orientation {
	// Front
	glm::vec3 Front;
	// Up
	glm::vec3 Up;
	// Right
	glm::vec3 Right;
	// origin
	glm::vec3 origin;
	//u
	float u_value;
};


class Track
{
public:

	// VAO
	unsigned int VAO;

	// Control Points Loading Class for loading from File
	rc_Spline g_Track;

	// Vector of control points
	std::vector<glm::vec3> controlPoints;

	// Track data
	std::vector<Vertex> vertices;
	// indices for EBO
	std::vector<unsigned int> indices;

	// hmax for camera
	float hmax = 0.0f;


	// constructor, just use same VBO as before, 
	Track(const char* trackPath)
	{

		// load Track data
		load_track(trackPath);

		create_track();

		for (int i = 0; i < 24; i++) {
			printf("Vertex %d: (%f,%f,%f)\n", i, vertices.at(i).Position.x, vertices.at(i).Position.y, vertices.at(i).Position.z);
		}

		setup_track();
	}

	// render the mesh
	void Draw(Shader shader, unsigned int textureID)
	{
		shader.use();
		glm::mat4 track;
		// active proper texture unit before binding
		glActiveTexture(GL_TEXTURE0);
		// and finally bind the textures
		glBindTexture(GL_TEXTURE_2D, textureID);
		// And again for the second texture
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureID);

		shader.setMat4("track", track);
		// draw mesh
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}

	// given an s float, find the point
	//  S is defined as the distance on the spline, so s=1.5 is the at the halfway point between the 1st and 2nd control point
	glm::vec3 get_point(float s)
	{
		float i;
		float u = modf(s, &i);
		int c0 = (int)std::abs(i) % (controlPoints.size());
		int c1 = (int)std::abs(i + 1) % (controlPoints.size());
		int c2 = (int)std::abs(i + 2) % (controlPoints.size());
		int c3 = (int)std::abs(i + 3) % (controlPoints.size());

		return interpolate(controlPoints[c0], controlPoints[c1], controlPoints[c2], controlPoints[c3], 0.5f, u);
	}


	void delete_buffers()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

private:


	/*  Render data  */
	unsigned int VBO, EBO;

	void load_track(const char* trackPath)
	{
		// Set folder path for our projects (easier than repeatedly defining it)
		g_Track.folder = "../Project_2/Media/";

		// Load the control points
		g_Track.loadSplineFrom(trackPath);

	}


	// Implement the Catmull-Rom Spline here
	//     Given 4 points, a tau and the u value 
	//     Since you can just use linear algebra from glm, just make the vectors and matrices and multiply them.  
	//     This should not be a very complicated function
	glm::vec3 interpolate(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, glm::vec3 pointD, float tau, float u)
	{
		glm::vec3 pointRes;

		pointRes = ((-tau) * u + 2 * tau * u * u - tau * u * u * u) * pointA + (1 + (tau - 3) * u * u + (2 - tau) * u * u * u) * pointB + (tau * u + (3 - 2 * tau) * u * u + (tau - 2) * u * u * u) * pointC + ((-tau) * u * u + tau * u * u * u) * pointD;
		// Just returning the first point at the moment, you need to return the interpolated point.  
		return pointRes;
	}

	// Here is the class where you will make the vertices or positions of the necessary objects of the track (calling subfunctions)
	//  For example, to make a basic roller coster:
	//    First, make the vertices for each rail here (and indices for the EBO if you do it that way).  
	//        You need the XYZ world coordinates, the Normal Coordinates, and the texture coordinates.
	//        The normal coordinates are necessary for the lighting to work.  
	//    Second, make vector of transformations for the planks across the rails
	void create_track()
	{
		// Create the vertices and indices (optional) for the rails
		//    One trick in creating these is to move along the spline and 
		//    shift left and right (from the forward direction of the spline) 
		//     to find the 3D coordinates of the rails.


		// Create the plank transformations or just creating the planks vertices
		//   Remember, you have to make planks be on the rails and in the same rotational direction 
		//       (look at the pictures from the project description to give you ideas).  



		// Here is just visualizing of using the control points to set the box transformatins with boxes. 
		//       You can take this code out for your rollercoster, this is just showing you how to access the control points
		glm::vec3 currentpos = glm::vec3(-2.0f, 0.0f, -2.0f);

		/* iterate throught  the points	g_Track.points() returns the vector containing all the control points */
		for (pointVectorIter ptsiter = g_Track.points().begin(); ptsiter != g_Track.points().end(); ptsiter++)
		{
			/* get the next point from the iterator */
			glm::vec3 pt(*ptsiter);

			// Print the Box
			std::cout << pt.x << "  " << pt.y << "  " << pt.z << std::endl;

			/* now just the uninteresting code that is no use at all for this project */
			currentpos += pt; 
			//  Mutliplying by two and translating (in initialization) just to move the boxes further apart.  
			controlPoints.push_back(currentpos * 2.0f);

		}

		Orientation current;
		Orientation future;

		current.origin = controlPoints[0];
		current.Right = glm::vec3(0.0f, 0.0f, 1.0f);
		current.Front = glm::vec3(1.0f, 0.0f, 0.0f);
		current.Up = glm::vec3(0.0f, 1.0f, 0.0f);

		int temp = controlPoints.size();

		float u = 0.10f;


		for (u; u < temp - 3; u += 0.1) {
			future.origin = get_point(u);
			future.Front = glm::normalize(future.origin - current.origin);
			future.Right = glm::normalize(glm::cross(current.Up, future.Front));
			future.Up = glm::normalize(glm::cross(future.Front, future.Right));

			makeRailPart(current, future, glm::vec2(0, 0));
			current = future;
		}
	}


	// Given 3 Points, create a triangle and push it into vertices (and EBO if you are using one)
		// Optional boolean to flip the normal if you need to
	void make_triangle(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, bool flipNormal)
	{
		Vertex a, b, c;

		a.Position.x = pointA.x;
		a.Position.y = pointA.y;
		a.Position.z = pointA.z;
		a.TexCoords.x = 0;
		a.TexCoords.y = 1;

		b.Position.x = pointB.x;
		b.Position.y = pointB.y;
		b.Position.z = pointB.z;
		b.TexCoords.x = 0;
		b.TexCoords.y = 0;

		c.Position.x = pointC.x;
		c.Position.y = pointC.y;
		c.Position.z = pointC.z;
		c.TexCoords.x = 1;
		c.TexCoords.y = 0;

		set_normals(a, b, c);

		if (flipNormal) {
			a.Normal = -a.Normal;
			b.Normal = -b.Normal;
			c.Normal = -c.Normal;
		}

		


		vertices.push_back(a);
		vertices.push_back(b);
		vertices.push_back(c);
	}

	// Given two orintations, create the rail between them.  Offset can be useful if you want to call this for more than for multiple rails
	void makeRailPart(Orientation ori_prev, Orientation ori_cur, glm::vec2 offset)
	{
		glm::vec3 a1 = ori_prev.origin - ori_prev.Right - 0.3f * ori_prev.Up;
		glm::vec3 a2 = ori_prev.origin - ori_prev.Right - 0.1f * ori_prev.Up;
		glm::vec3 a3 = ori_prev.origin + ori_prev.Right - 0.3f * ori_prev.Up;
		glm::vec3 a4 = ori_prev.origin + ori_prev.Right - 0.1f * ori_prev.Up;

		glm::vec3 b1 = ori_cur.origin - ori_cur.Right - 0.3f * ori_cur.Up;
		glm::vec3 b2 = ori_cur.origin - ori_cur.Right - 0.1f * ori_cur.Up;
		glm::vec3 b3 = ori_cur.origin + ori_cur.Right - 0.3f * ori_cur.Up;
		glm::vec3 b4 = ori_cur.origin + ori_cur.Right - 0.1f * ori_cur.Up;

		//left
		make_triangle(a1, a2, b2, true);
		make_triangle(b2, b1, a1, true);
		//right
		make_triangle(a3, a4, b4, true);
		make_triangle(b4, b3, a3, true);
		//top
		make_triangle(a2, a4, b4, true);
		make_triangle(b4, b2, a2, true);
		//bototm
		make_triangle(a1, a3, b3, true);
		make_triangle(b3, b1, a1, true);
	}

	// Find the normal for each triangle uisng the cross product and then add it to all three vertices of the triangle.  
	//   The normalization of all the triangles happens in the shader which averages all norms of adjacent triangles.   
	//   Order of the triangles matters here since you want to normal facing out of the object.  
	void set_normals(Vertex& p1, Vertex& p2, Vertex& p3)
	{
		glm::vec3 normal = glm::cross(p2.Position - p1.Position, p3.Position - p1.Position);
		p1.Normal += normal;
		p2.Normal += normal;
		p3.Normal += normal;
	}

	void setup_track()
	{
		// Like the heightmap project, this will create the buffers and send the information to OpenGL
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		// load data into vertex buffers

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/3/2 array which
		// again translates to 3/3/2 floats which translates to a byte array.

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		//vertex normal coords
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Normal));

		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,TexCoords));

		glBindVertexArray(0);
	}

};

