// Fall 2019

#pragma once

#include <glm/glm.hpp>

class Primitive {
public:
  virtual ~Primitive();
};

class Sphere : public Primitive {
public:
  virtual ~Sphere();
};

class Cube : public Primitive {
public:
  virtual ~Cube();
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const glm::vec3& pos, double radius)
    : m_pos(pos), m_radius(radius)
  {
  }
  virtual ~NonhierSphere();
  glm::vec3 getPos() {
    return m_pos;
  }
  double getRadius() {
    return m_radius;
  }

private:
  glm::vec3 m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const glm::vec3& pos, double size)
    : m_pos(pos), m_size(size)
  {
  }
  virtual ~NonhierBox();

  glm::vec3 getPos() {
    return m_pos;
  }
  double getSize() {
    return m_size;
  }

private:
  glm::vec3 m_pos;
  double m_size;
};
