#include "collision.hpp"

template<class A, class B>
bool checkCollision(const A* PrimarySurf, const B* SecondarySurf, Eigen::Matrix4f PrimaryPosition, Eigen::Matrix4f SecondaryPosition) {
	//static_assert(false, "Not A Function");
	std::cerr << "cannot call generic collision function!" << "\n";
	return true;
};

bool SurfaceNodeCollision(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, Eigen::Matrix4f SecondaryPosition) {
	Eigen::Matrix3f R = SecondaryPosition(seq(0, 2), seq(0, 2));
	Eigen::Vector3f p = SecondaryPosition(seq(0, 2), 3);
	for (auto& e : SecondarySurf->getEdges()) {
		//if (PrimarySurf->crossesSurface(R*SecondarySurf->getVerts()[e.first]+p, R*SecondarySurf->getVerts()[e.second]+p)) {
		//should at least let the user choose is the mesh is irrotational?
		if (PrimarySurf->crossesSurface(SecondarySurf->getVerts()[e.first] + p, SecondarySurf->getVerts()[e.second] + p)) {
			return true;
		}
	}
	return false;
}
bool SurfaceNodeCollision(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, Eigen::Matrix4f SecondaryPosition, std::vector<bool>* collision_info) {
	Eigen::Matrix3f R = SecondaryPosition(seq(0, 2), seq(0, 2));
	Eigen::Vector3f p = SecondaryPosition(seq(0, 2), 3);
	bool result = false;
	int i = 0;
	for (auto& e : SecondarySurf->getEdges()) {
		//if (PrimarySurf->crossesSurface(R*SecondarySurf->getVerts()[e.first]+p, R*SecondarySurf->getVerts()[e.second]+p)) {
		//should at least let the user choose is the mesh is irrotational?
		(*collision_info)[i] = (PrimarySurf->crossesSurface(SecondarySurf->getVerts()[e.first] + p, SecondarySurf->getVerts()[e.second] + p));
		if (result == false && collision_info->back() == true) {
			result = true;
		}
		i++;
	}
	return result;
}

template<>
bool checkCollision<Surface<3>, MeshSurface>(const Surface<3>* PrimarySurf, const MeshSurface* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition) {
	return SurfaceNodeCollision(PrimarySurf, SecondarySurf, PrimaryPosition.inverse() * SecondaryPosition);
}

template<>
bool checkCollision<Ellipse, Ellipse>(const Ellipse* PrimarySurf, const Ellipse* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition) {
	return true;
}

bool triIntersection(Eigen::Vector3f l1, Eigen::Vector3f l2, Eigen::Vector3f t1, Eigen::Vector3f t2, Eigen::Vector3f t3) {
	Eigen::Vector3f tri_norm = (t1-t2).cross(t3 - t1).normalized();
	//(k(l2-l1)+l1)*tri_norm = 0
	float k = l1.dot(tri_norm) / (l2 - l1).dot(tri_norm);
	if (k > 1 || k < 0) { //line is too "short" to intersect triangle
		return false;
	} else {
		Eigen::Vector3f p = k * (l1 - l2) + l1;
		//...
	}
	return true;

}

template<>
bool checkCollision<MeshSurface, MeshSurface>(const MeshSurface* PrimarySurf, const MeshSurface* SecondarySurf, const Eigen::Matrix4f PrimaryPosition, const Eigen::Matrix4f SecondaryPosition) {

	return true;
}
