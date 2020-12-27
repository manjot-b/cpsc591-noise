#pragma once

#include <string>
#include <glm/glm.hpp>
#include <memory>

#include "Shader.h"
#include "Mesh.h"

class Model
{
	public:
		enum NoiseType
		{
			GRASS = 0,
			WOOD,
			WATER,
			NONE,
			/*
			 * COUNT is not a NoiseType. It stores how many enums there are.
			 */
			COUNT
		};

		struct FragmentSettings
		{
			NoiseType noiseEffect;

			// Turbulence parameters.
			float persistence;
			int octaveCount;
			int octaveStart;

			// Wave parameters
			int waveCenters;
			float minFrequency;
			float maxFrequency;
			float phaseSpeed;
		};

		Model(const std::string &objPath);
		~Model();
		void draw(const Shader& shader) const;
		void update();
		void rotate(const glm::vec3 &rotate);
		void scale(float scale);
		void translate(const glm::vec3 &translate);
		FragmentSettings fragmentSettings;

	private:
		std::vector<std::unique_ptr<Mesh>> meshes;

		BoundingBox boundingBox;
		glm::mat4 modelMatrix;
		glm::vec3 m_rotate;			// how much to rotate along each axis
		float m_scale;				// scale to apply to model
		glm::vec3 m_translate;		// translation vector

		void sendUniforms(const Shader& shader) const;
		void extractDataFromNode(const aiScene* scene, const aiNode* node);
		void scaleToViewport();
};
