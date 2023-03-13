#pragma once
#include <array>
#include <Eigen/Dense>
#include <initializer_list>

#ifndef PUPPET_HITBOX
#define PUPPET_HITBOX

using Eigen::seq;

class Hitbox {
	Eigen::Vector3f shape_;
    Eigen::Vector3f E_sq_; //precomputed square of E
    Eigen::Vector3f E_inv_sq_; //inverse square of E

    Eigen::Vector3f getCriticalPoint(Eigen::Vector3f p) const {
        /* from puppet 1:

        f = np.multiply(self.E_sq, p)
        sig = np.sqrt(f @ p)
        return f/sig
        */
        Eigen::Vector3f f = E_sq_.array() * p.array();
        float sig = sqrt(f.transpose() * p);
        return f / sig;
    }
	
public:
	Hitbox(Eigen::Vector3f shape):shape_(shape), E_sq_(shape_.array().square()), E_inv_sq_(shape_.array().rsqrt()){}

	Hitbox() :shape_({ 0,0,0 }){}

	const Eigen::Vector3f& getShape() const {
		return shape_;
	}

	virtual bool checkCollision(const Hitbox& other,Eigen::Matrix4f position1,Eigen::Matrix4f position2) const {
		/* algorithm via Puppet1 code (python)
		g12 = HomoInv(g1) @ g2
        g21 = HomoInv(g2) @ g1
        p1 = g12[0:3, 3]
        f1 = self.getCrit(p1)
        p2 = g21[0:3, 3]
        f2 = other.getCrit(p2)
        #quickcheck
        cdist = p1-p2
        if cdist @ cdist.T > (self.max_rad+other.max_rad)*(self.max_rad+other.max_rad):
            return False
        intersect12 = np.power(g21[0:3,0:3] @ f1.T + p2, 2)
        if intersect12 @ other.E_inv_sq <= 1:
            return True
        if (intersect12 + 4 * (p2 - f1) @ p2) @ other.E_inv_sq <= 1:
            return True
        intersect21 = np.power(g12[0:3,0:3] @ f2.T + p1, 2)
        if intersect21 @ self.E_inv_sq <= 1:
            return True
        if (intersect21 + 4 * (p1 - f2) @ p1) @ self.E_inv_sq <= 1:
            return True
        return False
		*/
        Eigen::Matrix4f G12 = position1.inverse() * position2;
        Eigen::Matrix4f G21 = position2.inverse() * position1;
        Eigen::Vector3f p1 = G12(seq(0, 2), 2);
        Eigen::Vector3f p2 = G21(seq(0, 2), 2);
        Eigen::Vector3f f1 = getCriticalPoint(p1);
        Eigen::Vector3f f2 = getCriticalPoint(p2);

        Eigen::Vector3f intersect12 = (G21(seq(0, 2), seq(0, 2)) * f1 + p2).array().pow(2);
        if (intersect12.transpose() * other.E_inv_sq_ <= 1) {
            return true;
        } else if((intersect12 + Eigen::Vector3f::Ones()*4*(p2-f1).transpose()*p2).transpose()*other.E_inv_sq_ <= 1) {
            return true;
        } 
        Eigen::Vector3f intersect21 = (G12(seq(0, 2), seq(0, 2)) * f2 + p1).array().pow(2);
        if (intersect21.transpose() * this->E_inv_sq_ <= 1) {
            return true;
        }
        else if ((intersect21 + Eigen::Vector3f::Ones() * 4 * (p1 - f2).transpose() * p1).transpose() * other.E_inv_sq_ <= 1) {
            return true;
        }
		return false;
	}

    std::vector<bool> unitTest() {

    }

};

#endif