#include "stereo_processor/g2o_edges/scale_edge.hpp"
#include <g2o/core/factory.h>

#include <iostream>
using namespace std;

void EdgeScaleDirect::computeError()
{
  if(( v-BATCH_SIZE/2-4 ) <0 || ( v+BATCH_SIZE/2+4 ) >img1.rows)
  {
    _error(0,0) = 1.0;      
    this->setLevel ( 1 );
    return;
  }

  const VertexScale* vtx = static_cast<const VertexScale*> (_vertices[0]);
  const double& scale_inv = vtx->estimate();

  if(scale_inv<0.0)
  {
    _error(0,0) = 999.0;      
    this->setLevel ( 1 );
    return;
  }

  double u = u_base + scale_inv * u_inc;

  if(( u-BATCH_SIZE/2-4 ) <0 || ( u+BATCH_SIZE/2+4 ) >img1.cols)
  {
    _error(0,0) = 1.0;      
    this->setLevel ( 1 );
    return;
  }

  Eigen::VectorXd batch;
  helper::getBatchAround(img1,u,v,batch);
  // cout<<"batch= "<<batch<<endl<<" _measurement= "<<_measurement<<endl;
  // _error = batch - _measurement;
  _error(0,0) = (batch - _measurement).sum() / BATCH_SIZE / BATCH_SIZE;
  // _error(0,0) = (batch - _measurement).sum() / BATCH_SIZE / BATCH_SIZE + 1.0 / scale_inv / scale_inv;
  // cout<<"error "<<_error(0,0)<<endl;

}

void EdgeScaleDirect::linearizeOplus()
{
  if ( level() == 1 )
    {
        _jacobianOplusXi = Eigen::Matrix<double, 1, 1>::Zero();
        return;
    }

  const VertexScale* vtx = static_cast<const VertexScale*> (_vertices[0]);
  const double& scale_inv = vtx->estimate();

  double u = u_base + scale_inv * u_inc;

  Eigen::MatrixXd aI_au(1,2);
  aI_au << (helper::getPixelValue(img1, u+1.0, v)-helper::getPixelValue(img1, u, v)),
       (helper::getPixelValue(img1, u, v+1.0)-helper::getPixelValue(img1, u, v));

  _jacobianOplusXi = aI_au * au_as;

    // if(_jacobianOplusXi.norm()<1000)
    // {
    //     _jacobianOplusXi = Eigen::Matrix<double, 1, 6>::Zero();
    //     this->setLevel ( 1 );
    // }
}
