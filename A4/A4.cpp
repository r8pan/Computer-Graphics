// Fall 2019

#include <glm/ext.hpp>

#include "A4.hpp"
#include "polyroots.hpp"
#include "scene_lua.hpp"
#include "Primitive.hpp"
#include "Material.hpp"
#include "PhongMaterial.hpp"
#include "GeometryNode.hpp"
#include <memory>

using namespace std;
using namespace glm;

// -----------------------------------------------------------------------------

// Variable definitions
float dr = 0;
float dg = 0;
float db = 0;
float sr = 0;
float sg = 0;
float sb = 0;
double front = 0;
bool hasRoot = false;
bool shadow = false;
const float eps = 0.02;
double ka = 0.5;







// -----------------------------------------------------------------------------

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
) {


  // Fill in raytracing code here...

	size_t h = image.height();
	size_t w = image.width();

	for (uint y = 0; y < h; ++y) {
		string row = "";
		for (uint x = 0; x < w; ++x) {
			// ray-object intersection
			bool hasRoot = false;
			dr = dg = db = sr = sg = sb = 0;
			front = -65536;
			vec3 intersection;
			vec3 surface_normal;
			vec3 incomingLight;
			vec3 v;
			vec3 hv;
			string currentObj = "";

			for (SceneNode * node : root->children) {
				double xe = eye.x;
				double ye = eye.y;
				double ze = eye.z;
				int xp = x - w / 2;
				int yp = y - h / 2;
				int zp = 300;



				if (node->m_name[0] == 's') {
					// the object is a sphere
					GeometryNode * gnode = static_cast<GeometryNode*>(node);
					NonhierSphere * ns = static_cast<NonhierSphere*>(gnode->m_primitive);
					PhongMaterial * pm = static_cast<PhongMaterial*>(gnode->m_material);

					double xs = ns->getPos().x;
					double ys = - ns->getPos().y;
					double zs = ns->getPos().z;
					double radius = ns->getRadius();

					// compute a, b, c to find polynomial root as t
					double a = xp*xp + xe*xe - 2*xp*xe + yp*yp + ye*ye - 2*yp*ye
					 + zp*zp + ze*ze - 2*zp*ze;
					double b = (xp*xe-xp*xs-xe*xe+xe*xs + yp*ye-yp*ys-ye*ye+ye*ys
					 + zp*ze-zp*zs-ze*ze+ze*zs) * 2;
					double c = xe*xe + xs*xs - 2*xe*xs + ye*ye + ys*ys - 2*ye*ys
					 + ze*ze + zs*zs - 2*ze*zs - radius*radius;

					double roots[2];
					bool curHasRoot = quadraticRoots(a, b, c, roots);
					if (curHasRoot) {
						hasRoot = true;
						double root1 = ze + roots[0] * (zp - ze);
						double root2 = ze + roots[1] * (zp - ze);
						float newFront;
						if (root1 > root2) {
							newFront = root1;
						}
						else {
							newFront = root2;
						}

						if (newFront > front) {
							dr = pm->getKd().r;
							dg = pm->getKd().g;
							db = pm->getKd().b;
							sr = pm->getKs().r;
							sg = pm->getKs().g;
							sb = pm->getKs().b;
							front = newFront;
							currentObj = node->m_name;
							double tempX, tempY, tempZ;
							if (root1 > root2) {
								tempX = xe + roots[0] * (xp - xe);
								tempY = ye + roots[0] * (yp - ye);
								tempZ = ze + roots[0] * (zp - ze);
							}
							else {
								tempX = xe + roots[1] * (xp - xe);
								tempY = ye + roots[1] * (yp - ye);
								tempZ = ze + roots[1] * (zp - ze);
							}
							intersection = {tempX, tempY, tempZ};
							surface_normal = {
								(intersection.x - xs) / radius,
								(intersection.y - ys) / radius,
								(intersection.z - zs) / radius
							};
							double den = sqrt(pow(xe-intersection.x,2)+pow(ye-intersection.y,2)+pow(ze-intersection.z,2));
							v = {
								(xe-intersection.x)/den, (ye-intersection.y)/den, (ze-intersection.z)/den
							};
						}
					}
				}
				else if (node->m_name[0] == 'c') {
					// the object is a cube
					GeometryNode * gnode = static_cast<GeometryNode*>(node);
					NonhierBox * nb = static_cast<NonhierBox*>(gnode->m_primitive);
					PhongMaterial * pm = static_cast<PhongMaterial*>(gnode->m_material);

					double xs = nb->getPos().x;
					double ys = - nb->getPos().y;
					double zs = nb->getPos().z;
					double size = nb->getSize();

					double t;
					double ix = 0;
					double iy = 0;
					double iz = 0;

					dr = pm->getKd().r;
					dg = pm->getKd().g;
					db = pm->getKd().b;
					sr = pm->getKs().r;
					sg = pm->getKs().g;
					sb = pm->getKs().b;

					double newFront;

					// front
					ix = xe + (zs + size - ze) / (zp - ze) * (xp - xe);
					iy = ye + (zs + size - ze) / (zp - ze) * (yp - ye);
					if (ix >= xs && ix <= xs+size && iy >= ys && iy <= ys+size && zs+size > front) {
						hasRoot = true;
						newFront = zs + size;
						if (newFront > front) {
							front = newFront;
							currentObj = node->m_name;
							intersection = {ix, iy, zs+size};
							surface_normal = {0, 0, 1};
						}
					}
					// back
					ix = xe + (zs - ze) / (zp - ze) * (xp - xe);
					iy = ye + (zs - ze) / (zp - ze) * (yp - ye);
					if (ix >= xs && ix <= xs+size && iy >= ys && iy <= ys+size && zs+size > front) {
						hasRoot = true;
						newFront = zs;
						if (newFront > front) {
							front = newFront;
							currentObj = node->m_name;
							intersection = {ix, iy, zs};
							surface_normal = {0, 0, -1};
						}
					}
					// left
					iz = ze + (xs - xe) / (xp - xe) * (zp - ze);
					iy = ye + (xs - xe) / (xp - xe) * (yp - ye);
					if (iz >= zs && iz <= zs+size && iy >= ys && iy <= ys+size && iz > front) {
						hasRoot = true;
						newFront = iz;
						if (newFront > front) {
							front = newFront;
							currentObj = node->m_name;
							intersection = {xs, iy, iz};
							surface_normal = {-1, 0, 0};
						}
					}
					// right
					iz = ze + (xs + size - xe) / (xp - xe) * (zp - ze);
					iy = ye + (xs + size - xe) / (xp - xe) * (yp - ye);
					if (iz >= zs && iz <= zs+size && iy >= ys && iy <= ys+size && iz > front) {
						hasRoot = true;
						newFront = iz;
						if (newFront > front) {
							front = newFront;
							currentObj = node->m_name;
							intersection = {xs+size, iy, iz};
							surface_normal = {1, 0, 0};
						}
					}
					// top
					iz = ze + (ys + size - ye) / (yp - ye) * (zp - ze);
					ix = xe + (ys + size - ye) / (yp - ye) * (xp - xe);
					if (iz >= zs && iz <= zs+size && ix >= xs && ix <= xs+size && iz > front) {
						hasRoot = true;
						newFront = iz;
						if (newFront > front) {
							newFront = newFront;
							currentObj = node->m_name;
							intersection = {ix, ys+size, iz};
							surface_normal = {0, 1, 0};
						}
					}
					// bottom
					iz = ze + (ys - ye) / (yp - ye) * (zp - ze);
					ix = xe + (ys - ye) / (yp - ye) * (xp - xe);
					if (iz >= zs && iz <= zs+size && ix >= xs && ix <= xs+size && iz > front) {
						hasRoot = true;
						newFront = iz;
						if (newFront > front) {
							front = newFront;
							currentObj = node->m_name;
							intersection = {ix, ys, iz};
							surface_normal = {0, -1, 0};
						}
					}
					double den = sqrt(pow(xe-intersection.x,2)+pow(ye-intersection.y,2)+pow(ze-intersection.z,2));
					v = {
						(xe-intersection.x)/den, (ye-intersection.y)/den, (ze-intersection.z)/den
					};
				}
			}

			// light-object intersection
			shadow = false;
			int numLights = 0;
			int numShadows = 0;
			if (hasRoot) {
				for (auto const& light : lights) {
					numLights++;
					if (true) {
						for (SceneNode * node : root->children) {
							if (node->m_name[0] == 's') {
								// the object is a sphere
								GeometryNode * gnode = static_cast<GeometryNode*>(node);
								NonhierSphere * ns = static_cast<NonhierSphere*>(gnode->m_primitive);

								double xe = intersection.x;
								double ye = intersection.y;
								double ze = intersection.z;
								int xp = light->position.x;
								int yp = light->position.y;
								int zp = light->position.z;
								double xs = ns->getPos().x;
								double ys = -ns->getPos().y;
								double zs = ns->getPos().z;
								double radius = ns->getRadius();

								double den = sqrt(pow(xp-xe,2)+pow(yp-ye,2)+pow(zp-ze,2));
								incomingLight = {
									(xp-xe)/den, (yp-ye)/den, (zp-ze)/den
								};
								vec3 temp = v + incomingLight;
								hv = temp/sqrt(pow(temp.x,2)+pow(temp.y,2)+pow(temp.z,2));

								// compute a, b, c to find polynomial root as t
								double a = xp*xp + xe*xe - 2*xp*xe + yp*yp + ye*ye - 2*yp*ye
								 + zp*zp + ze*ze - 2*zp*ze;
								double b = (xp*xe-xp*xs-xe*xe+xe*xs + yp*ye-yp*ys-ye*ye+ye*ys
								 + zp*ze-zp*zs-ze*ze+ze*zs) * 2;
								double c = xe*xe + xs*xs - 2*xe*xs + ye*ye + ys*ys - 2*ye*ys
								 + ze*ze + zs*zs - 2*ze*zs - radius*radius;

								double roots[2];
								int curHasRoot = quadraticRoots(a, b, c, roots);
								if (curHasRoot == 2) {

								 	double r0x = xe + roots[0] * (xp - xe);
									double r0y = ye + roots[0] * (yp - ye);
									double r0z = ze + roots[0] * (zp - ze);
									double r1x = xe + roots[1] * (xp - xe);
									double r1y = ye + roots[1] * (yp - ye);
									double r1z = ze + roots[1] * (zp - ze);

									float d0 = sqrt(pow(xp-r0x,2)+pow(yp-r0y,2)+pow(zp-r0z,2));
									float d1 = sqrt(pow(xp-r1x,2)+pow(yp-r1y,2)+pow(zp-r1z,2));
									float dis = sqrt(pow(xp-intersection.x,2)+pow(yp-intersection.y,2)+pow(zp-intersection.z,2));

									if (roots[0] != roots[1]) {
										if (currentObj == node->m_name) {
											if ((fabs(intersection.x-r0x) < eps && d0 > d1) || (fabs(intersection.x-r1x) < eps && d1 > d0)) {
													//shadow = true;
													numShadows++;
													break;
											}
										}
										else {
											if (dis > d0 || dis > d1) {
												numShadows++;
												break;
											}
										}
									}
									else {
										if (currentObj != node->m_name) {
											//shadoinfo.htmlw = true;
											numShadows++;
											break;
										}
									}
								}
							}
							if (node->m_name[0] == 'c') {
								GeometryNode * gnode = static_cast<GeometryNode*>(node);
								NonhierBox * nb = static_cast<NonhierBox*>(gnode->m_primitive);

								double xe = intersection.x;
								double ye = intersection.y;
								double ze = intersection.z;
								int xp = light->position.x;
								int yp = light->position.y;
								int zp = light->position.z;
								double xs = nb->getPos().x;
								double ys = -nb->getPos().y;
								double zs = nb->getPos().z;
								double size = nb->getSize();

								double t;
								double ix = 0;
								double iy = 0;
								double iz = 0;

								double den = sqrt(pow(xp-xe,2)+pow(yp-ye,2)+pow(zp-ze,2));
								incomingLight = {
									(xp-xe)/den, (yp-ye)/den, (zp-ze)/den
								};

								vec3 temp = v + incomingLight;
								hv = temp/sqrt(pow(temp.x,2)+pow(temp.y,2)+pow(temp.z,2));
								// cout << "[" << x << ", " << y << "]" << endl;
								// cout << surface_normal.x << surface_normal.y << surface_normal.z << endl;
								// cout << incomingLight.x << incomingLight.y << incomingLight.z << endl;
								// cout << dot(surface_normal, incomingLight) << endl;

								vec3 newI;
								float dis = sqrt(pow(xp-intersection.x,2)+pow(yp-intersection.y,2)+pow(zp-intersection.z,2));

								// front
								ix = xe + (zs + size - ze) / (zp - ze) * (xp - xe);
								iy = ye + (zs + size - ze) / (zp - ze) * (yp - ye);
								if (ix >= xs && ix <= xs+size && iy >= ys && iy <= ys+size && zs+size > front) {
									newI = {ix, iy, zs+size};
									float d0 = sqrt(pow(xp-ix,2)+pow(yp-iy,2)+pow(zp-(zs+size),2));
									if (dis > d0) {
										numShadows++;
										break;
									}
								}
								// back
								ix = xe + (zs - ze) / (zp - ze) * (xp - xe);
								iy = ye + (zs - ze) / (zp - ze) * (yp - ye);
								if (ix >= xs && ix <= xs+size && iy >= ys && iy <= ys+size && zs+size > front) {
									newI = {ix, iy, zs};
									float d0 = sqrt(pow(xp-ix,2)+pow(yp-iy,2)+pow(zp-zs,2));
									if (dis > d0) {
										numShadows++;
										break;
									}
								}
								// left
								iz = ze + (xs - xe) / (xp - xe) * (zp - ze);
								iy = ye + (xs - xe) / (xp - xe) * (yp - ye);
								if (iz >= zs && iz <= zs+size && iy >= ys && iy <= ys+size && iz > front) {
									newI = {xs, iy, iz};
									float d0 = sqrt(pow(xp-xs,2)+pow(yp-iy,2)+pow(zp-iz,2));
									if (dis > d0) {
										numShadows++;
										break;
									}
								}
								// right
								iz = ze + (xs + size - xe) / (xp - xe) * (zp - ze);
								iy = ye + (xs + size - xe) / (xp - xe) * (yp - ye);
								if (iz >= zs && iz <= zs+size && iy >= ys && iy <= ys+size && iz > front) {
									newI = {xs+size, iy, iz};
									float d0 = sqrt(pow(xp-(xs+size),2)+pow(yp-iy,2)+pow(zp-iz,2));
									if (dis > d0) {
										numShadows++;
										break;
									}
								}
								// top
								iz = ze + (ys + size - ye) / (yp - ye) * (zp - ze);
								ix = xe + (ys + size - ye) / (yp - ye) * (xp - xe);
								if (iz >= zs && iz <= zs+size && ix >= xs && ix <= xs+size && iz > front) {
									newI = {xs, ys+size, iz};
									float d0 = sqrt(pow(xp-ix,2)+pow(yp-(ys+size),2)+pow(zp-iz,2));
									if (dis > d0) {
										numShadows++;
										break;
									}
								}
								// bottom
								iz = ze + (ys - ye) / (yp - ye) * (zp - ze);
								ix = xe + (ys - ye) / (yp - ye) * (xp - xe);
								if (iz >= zs && iz <= zs+size && ix >= xs && ix <= xs+size && iz > front) {
									newI = {xs, ys, iz};
									float d0 = sqrt(pow(xp-ix,2)+pow(yp-ys,2)+pow(zp-iz,2));
									if (dis > d0) {
										numShadows++;
										break;
									}
								}
							}
						}
					}
				}
				shadow = (numLights == numShadows);
			}
			else {
				shadow = false;
			}

			// final decision
			if (shadow) {
				// cout << "shadowed" << endl;
				image(x, y, 0) = 0 + ka*ambient.r;
				image(x, y, 1) = 0 + ka*ambient.g;
				image(x, y, 2) = 0 + ka*ambient.b;
			}
			else if (hasRoot) {
				// cout << "hasRoot" << endl;
				// cout << dot(surface_normal,incomingLight) << " " << r << " " << g << " " << b << endl;
				image(x, y, 0) = ka*ambient.r + dr*dot(surface_normal,incomingLight) + sr*pow(dot(hv,surface_normal),25);
				image(x, y, 1) = ka*ambient.g + dg*dot(surface_normal,incomingLight) + sg*pow(dot(hv,surface_normal),25);
				image(x, y, 2) = ka*ambient.b + db*dot(surface_normal,incomingLight) + sb*pow(dot(hv,surface_normal),25);
			}
			else {
				image(x, y, 0) = (double)pow(y, 0.8)/h;
				image(x, y, 1) = (double)pow(y, 0.8)/h;
				image(x, y, 2) = (double)0.3;
			}
		}
	}



  // std::cout << "Calling A4_Render(\n" <<
	// 	  "\t" << *root <<
  //         "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
  //         "\t" << "eye:  " << glm::to_string(eye) << std::endl <<
	// 	  "\t" << "view: " << glm::to_string(view) << std::endl <<
	// 	  "\t" << "up:   " << glm::to_string(up) << std::endl <<
	// 	  "\t" << "fovy: " << fovy << std::endl <<
  //         "\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
	// 	  "\t" << "lights{" << std::endl;
	//
	// for(const Light * light : lights) {
	// 	std::cout << "\t\t" <<  *light << std::endl;
	// }
	// std::cout << "\t}" << std::endl;
	// std:: cout <<")" << std::endl;

	// size_t h = image.height();
	// size_t w = image.width();

	// for (uint y = 0; y < h; ++y) {
	// 	for (uint x = 0; x < w; ++x) {
	// 		// Red:
	// 		image(x, y, 0) = (double)1.0;
	// 		// Green:
	// 		image(x, y, 1) = (double)0.0;
	// 		// Blue:
	// 		image(x, y, 2) = (double)0.0;
	// 	}
	// }

}
