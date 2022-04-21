#include "AABB.h"

#include "GLCommon.h"

AABB::AABB(const glm::vec3& min, const glm::vec3 max) : min(min), max(max)
{

}

AABB::~AABB()
{

}

void AABB::Draw(Ref<Shader> shader, const glm::mat4& transform) const
{
	shader->Bind();
	shader->SetMat4x4("matModel", transform);
	shader->SetMat4x4("matModelInverseTranspose", glm::inverse(transform));

	shader->SetFloat("isIgnoreLighting", (float)GL_TRUE);
	shader->SetFloat("isOverrideColor", (float)GL_TRUE);
	shader->SetFloat4("colorOverride", glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

	glLineWidth(2.0f);
	glBegin(GL_LINES);

	glVertex3f(this->min.x, this->min.y, this->min.z);
	glVertex3f(this->min.x, this->min.y, this->max.z);

	glVertex3f(this->min.x, this->min.y, this->min.z);
	glVertex3f(this->max.x, this->min.y, this->min.z);

	glVertex3f(this->max.x, this->min.y, this->min.z);
	glVertex3f(this->max.x, this->min.y, this->max.z);

	glVertex3f(this->min.x, this->min.y, this->max.z);
	glVertex3f(this->max.x, this->min.y, this->max.z);

	// Draw connecting lines up
	glVertex3f(this->min.x, this->min.y, this->min.z);
	glVertex3f(this->min.x, this->max.y, this->min.z);

	glVertex3f(this->max.x, this->min.y, this->min.z);
	glVertex3f(this->max.x, this->max.y, this->min.z);

	glVertex3f(this->max.x, this->min.y, this->max.z);
	glVertex3f(this->max.x, this->max.y, this->max.z);

	glVertex3f(this->min.x, this->min.y, this->max.z);
	glVertex3f(this->min.x, this->max.y, this->max.z);

	// Draw top square
	glVertex3f(this->min.x, this->max.y, this->min.z);
	glVertex3f(this->min.x, this->max.y, this->max.z);

	glVertex3f(this->min.x, this->max.y, this->min.z);
	glVertex3f(this->max.x, this->max.y, this->min.z);

	glVertex3f(this->max.x, this->max.y, this->min.z);
	glVertex3f(this->max.x, this->max.y, this->max.z);

	glVertex3f(this->min.x, this->max.y, this->max.z);
	glVertex3f(this->max.x, this->max.y, this->max.z);

	glEnd();

	shader->SetFloat("isOverrideColor", (float)GL_FALSE);
	shader->SetFloat("isIgnoreLighting", (float)GL_FALSE);
}

bool AABB::IsInside(const glm::mat4& transform, const glm::vec3& position) const
{
	glm::vec4 transformedMin = transform * glm::vec4(this->min, 1.0f);
	glm::vec4 transformedMax = transform * glm::vec4(this->max, 1.0f);
	return std::min(transformedMax.x, std::max(transformedMin.x, position.x)) == position.x
		&& std::min(transformedMax.y, std::max(transformedMin.y, position.y)) == position.y
		&& std::min(transformedMax.z, std::max(transformedMin.z, position.z)) == position.z;
}