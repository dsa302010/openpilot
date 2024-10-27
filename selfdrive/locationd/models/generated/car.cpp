#include "car.h"

namespace {
#define DIM 9
#define EDIM 9
#define MEDIM 9
typedef void (*Hfun)(double *, double *, double *);

double mass;

void set_mass(double x){ mass = x;}

double rotational_inertia;

void set_rotational_inertia(double x){ rotational_inertia = x;}

double center_to_front;

void set_center_to_front(double x){ center_to_front = x;}

double center_to_rear;

void set_center_to_rear(double x){ center_to_rear = x;}

double stiffness_front;

void set_stiffness_front(double x){ stiffness_front = x;}

double stiffness_rear;

void set_stiffness_rear(double x){ stiffness_rear = x;}
const static double MAHA_THRESH_25 = 3.8414588206941227;
const static double MAHA_THRESH_24 = 5.991464547107981;
const static double MAHA_THRESH_30 = 3.8414588206941227;
const static double MAHA_THRESH_26 = 3.8414588206941227;
const static double MAHA_THRESH_27 = 3.8414588206941227;
const static double MAHA_THRESH_29 = 3.8414588206941227;
const static double MAHA_THRESH_28 = 3.8414588206941227;
const static double MAHA_THRESH_31 = 3.8414588206941227;

/******************************************************************************
 *                       Code generated with SymPy 1.12                       *
 *                                                                            *
 *              See http://www.sympy.org/ for more information.               *
 *                                                                            *
 *                         This file is part of 'ekf'                         *
 ******************************************************************************/
void err_fun(double *nom_x, double *delta_x, double *out_8377796624028463417) {
   out_8377796624028463417[0] = delta_x[0] + nom_x[0];
   out_8377796624028463417[1] = delta_x[1] + nom_x[1];
   out_8377796624028463417[2] = delta_x[2] + nom_x[2];
   out_8377796624028463417[3] = delta_x[3] + nom_x[3];
   out_8377796624028463417[4] = delta_x[4] + nom_x[4];
   out_8377796624028463417[5] = delta_x[5] + nom_x[5];
   out_8377796624028463417[6] = delta_x[6] + nom_x[6];
   out_8377796624028463417[7] = delta_x[7] + nom_x[7];
   out_8377796624028463417[8] = delta_x[8] + nom_x[8];
}
void inv_err_fun(double *nom_x, double *true_x, double *out_3733121449147292039) {
   out_3733121449147292039[0] = -nom_x[0] + true_x[0];
   out_3733121449147292039[1] = -nom_x[1] + true_x[1];
   out_3733121449147292039[2] = -nom_x[2] + true_x[2];
   out_3733121449147292039[3] = -nom_x[3] + true_x[3];
   out_3733121449147292039[4] = -nom_x[4] + true_x[4];
   out_3733121449147292039[5] = -nom_x[5] + true_x[5];
   out_3733121449147292039[6] = -nom_x[6] + true_x[6];
   out_3733121449147292039[7] = -nom_x[7] + true_x[7];
   out_3733121449147292039[8] = -nom_x[8] + true_x[8];
}
void H_mod_fun(double *state, double *out_2037772241286162231) {
   out_2037772241286162231[0] = 1.0;
   out_2037772241286162231[1] = 0;
   out_2037772241286162231[2] = 0;
   out_2037772241286162231[3] = 0;
   out_2037772241286162231[4] = 0;
   out_2037772241286162231[5] = 0;
   out_2037772241286162231[6] = 0;
   out_2037772241286162231[7] = 0;
   out_2037772241286162231[8] = 0;
   out_2037772241286162231[9] = 0;
   out_2037772241286162231[10] = 1.0;
   out_2037772241286162231[11] = 0;
   out_2037772241286162231[12] = 0;
   out_2037772241286162231[13] = 0;
   out_2037772241286162231[14] = 0;
   out_2037772241286162231[15] = 0;
   out_2037772241286162231[16] = 0;
   out_2037772241286162231[17] = 0;
   out_2037772241286162231[18] = 0;
   out_2037772241286162231[19] = 0;
   out_2037772241286162231[20] = 1.0;
   out_2037772241286162231[21] = 0;
   out_2037772241286162231[22] = 0;
   out_2037772241286162231[23] = 0;
   out_2037772241286162231[24] = 0;
   out_2037772241286162231[25] = 0;
   out_2037772241286162231[26] = 0;
   out_2037772241286162231[27] = 0;
   out_2037772241286162231[28] = 0;
   out_2037772241286162231[29] = 0;
   out_2037772241286162231[30] = 1.0;
   out_2037772241286162231[31] = 0;
   out_2037772241286162231[32] = 0;
   out_2037772241286162231[33] = 0;
   out_2037772241286162231[34] = 0;
   out_2037772241286162231[35] = 0;
   out_2037772241286162231[36] = 0;
   out_2037772241286162231[37] = 0;
   out_2037772241286162231[38] = 0;
   out_2037772241286162231[39] = 0;
   out_2037772241286162231[40] = 1.0;
   out_2037772241286162231[41] = 0;
   out_2037772241286162231[42] = 0;
   out_2037772241286162231[43] = 0;
   out_2037772241286162231[44] = 0;
   out_2037772241286162231[45] = 0;
   out_2037772241286162231[46] = 0;
   out_2037772241286162231[47] = 0;
   out_2037772241286162231[48] = 0;
   out_2037772241286162231[49] = 0;
   out_2037772241286162231[50] = 1.0;
   out_2037772241286162231[51] = 0;
   out_2037772241286162231[52] = 0;
   out_2037772241286162231[53] = 0;
   out_2037772241286162231[54] = 0;
   out_2037772241286162231[55] = 0;
   out_2037772241286162231[56] = 0;
   out_2037772241286162231[57] = 0;
   out_2037772241286162231[58] = 0;
   out_2037772241286162231[59] = 0;
   out_2037772241286162231[60] = 1.0;
   out_2037772241286162231[61] = 0;
   out_2037772241286162231[62] = 0;
   out_2037772241286162231[63] = 0;
   out_2037772241286162231[64] = 0;
   out_2037772241286162231[65] = 0;
   out_2037772241286162231[66] = 0;
   out_2037772241286162231[67] = 0;
   out_2037772241286162231[68] = 0;
   out_2037772241286162231[69] = 0;
   out_2037772241286162231[70] = 1.0;
   out_2037772241286162231[71] = 0;
   out_2037772241286162231[72] = 0;
   out_2037772241286162231[73] = 0;
   out_2037772241286162231[74] = 0;
   out_2037772241286162231[75] = 0;
   out_2037772241286162231[76] = 0;
   out_2037772241286162231[77] = 0;
   out_2037772241286162231[78] = 0;
   out_2037772241286162231[79] = 0;
   out_2037772241286162231[80] = 1.0;
}
void f_fun(double *state, double dt, double *out_8306308797768162476) {
   out_8306308797768162476[0] = state[0];
   out_8306308797768162476[1] = state[1];
   out_8306308797768162476[2] = state[2];
   out_8306308797768162476[3] = state[3];
   out_8306308797768162476[4] = state[4];
   out_8306308797768162476[5] = dt*((-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]))*state[6] - 9.8000000000000007*state[8] + stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*state[1]) + (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*state[4])) + state[5];
   out_8306308797768162476[6] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*state[4])) + state[6];
   out_8306308797768162476[7] = state[7];
   out_8306308797768162476[8] = state[8];
}
void F_fun(double *state, double dt, double *out_438348106830506366) {
   out_438348106830506366[0] = 1;
   out_438348106830506366[1] = 0;
   out_438348106830506366[2] = 0;
   out_438348106830506366[3] = 0;
   out_438348106830506366[4] = 0;
   out_438348106830506366[5] = 0;
   out_438348106830506366[6] = 0;
   out_438348106830506366[7] = 0;
   out_438348106830506366[8] = 0;
   out_438348106830506366[9] = 0;
   out_438348106830506366[10] = 1;
   out_438348106830506366[11] = 0;
   out_438348106830506366[12] = 0;
   out_438348106830506366[13] = 0;
   out_438348106830506366[14] = 0;
   out_438348106830506366[15] = 0;
   out_438348106830506366[16] = 0;
   out_438348106830506366[17] = 0;
   out_438348106830506366[18] = 0;
   out_438348106830506366[19] = 0;
   out_438348106830506366[20] = 1;
   out_438348106830506366[21] = 0;
   out_438348106830506366[22] = 0;
   out_438348106830506366[23] = 0;
   out_438348106830506366[24] = 0;
   out_438348106830506366[25] = 0;
   out_438348106830506366[26] = 0;
   out_438348106830506366[27] = 0;
   out_438348106830506366[28] = 0;
   out_438348106830506366[29] = 0;
   out_438348106830506366[30] = 1;
   out_438348106830506366[31] = 0;
   out_438348106830506366[32] = 0;
   out_438348106830506366[33] = 0;
   out_438348106830506366[34] = 0;
   out_438348106830506366[35] = 0;
   out_438348106830506366[36] = 0;
   out_438348106830506366[37] = 0;
   out_438348106830506366[38] = 0;
   out_438348106830506366[39] = 0;
   out_438348106830506366[40] = 1;
   out_438348106830506366[41] = 0;
   out_438348106830506366[42] = 0;
   out_438348106830506366[43] = 0;
   out_438348106830506366[44] = 0;
   out_438348106830506366[45] = dt*(stiffness_front*(-state[2] - state[3] + state[7])/(mass*state[1]) + (-stiffness_front - stiffness_rear)*state[5]/(mass*state[4]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[6]/(mass*state[4]));
   out_438348106830506366[46] = -dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(mass*pow(state[1], 2));
   out_438348106830506366[47] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_438348106830506366[48] = -dt*stiffness_front*state[0]/(mass*state[1]);
   out_438348106830506366[49] = dt*((-1 - (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*pow(state[4], 2)))*state[6] - (-stiffness_front*state[0] - stiffness_rear*state[0])*state[5]/(mass*pow(state[4], 2)));
   out_438348106830506366[50] = dt*(-stiffness_front*state[0] - stiffness_rear*state[0])/(mass*state[4]) + 1;
   out_438348106830506366[51] = dt*(-state[4] + (-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(mass*state[4]));
   out_438348106830506366[52] = dt*stiffness_front*state[0]/(mass*state[1]);
   out_438348106830506366[53] = -9.8000000000000007*dt;
   out_438348106830506366[54] = dt*(center_to_front*stiffness_front*(-state[2] - state[3] + state[7])/(rotational_inertia*state[1]) + (-center_to_front*stiffness_front + center_to_rear*stiffness_rear)*state[5]/(rotational_inertia*state[4]) + (-pow(center_to_front, 2)*stiffness_front - pow(center_to_rear, 2)*stiffness_rear)*state[6]/(rotational_inertia*state[4]));
   out_438348106830506366[55] = -center_to_front*dt*stiffness_front*(-state[2] - state[3] + state[7])*state[0]/(rotational_inertia*pow(state[1], 2));
   out_438348106830506366[56] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_438348106830506366[57] = -center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_438348106830506366[58] = dt*(-(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])*state[5]/(rotational_inertia*pow(state[4], 2)) - (-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])*state[6]/(rotational_inertia*pow(state[4], 2)));
   out_438348106830506366[59] = dt*(-center_to_front*stiffness_front*state[0] + center_to_rear*stiffness_rear*state[0])/(rotational_inertia*state[4]);
   out_438348106830506366[60] = dt*(-pow(center_to_front, 2)*stiffness_front*state[0] - pow(center_to_rear, 2)*stiffness_rear*state[0])/(rotational_inertia*state[4]) + 1;
   out_438348106830506366[61] = center_to_front*dt*stiffness_front*state[0]/(rotational_inertia*state[1]);
   out_438348106830506366[62] = 0;
   out_438348106830506366[63] = 0;
   out_438348106830506366[64] = 0;
   out_438348106830506366[65] = 0;
   out_438348106830506366[66] = 0;
   out_438348106830506366[67] = 0;
   out_438348106830506366[68] = 0;
   out_438348106830506366[69] = 0;
   out_438348106830506366[70] = 1;
   out_438348106830506366[71] = 0;
   out_438348106830506366[72] = 0;
   out_438348106830506366[73] = 0;
   out_438348106830506366[74] = 0;
   out_438348106830506366[75] = 0;
   out_438348106830506366[76] = 0;
   out_438348106830506366[77] = 0;
   out_438348106830506366[78] = 0;
   out_438348106830506366[79] = 0;
   out_438348106830506366[80] = 1;
}
void h_25(double *state, double *unused, double *out_5971982590284797884) {
   out_5971982590284797884[0] = state[6];
}
void H_25(double *state, double *unused, double *out_3963824281975169266) {
   out_3963824281975169266[0] = 0;
   out_3963824281975169266[1] = 0;
   out_3963824281975169266[2] = 0;
   out_3963824281975169266[3] = 0;
   out_3963824281975169266[4] = 0;
   out_3963824281975169266[5] = 0;
   out_3963824281975169266[6] = 1;
   out_3963824281975169266[7] = 0;
   out_3963824281975169266[8] = 0;
}
void h_24(double *state, double *unused, double *out_1517252688726049001) {
   out_1517252688726049001[0] = state[4];
   out_1517252688726049001[1] = state[5];
}
void H_24(double *state, double *unused, double *out_782685044991339768) {
   out_782685044991339768[0] = 0;
   out_782685044991339768[1] = 0;
   out_782685044991339768[2] = 0;
   out_782685044991339768[3] = 0;
   out_782685044991339768[4] = 1;
   out_782685044991339768[5] = 0;
   out_782685044991339768[6] = 0;
   out_782685044991339768[7] = 0;
   out_782685044991339768[8] = 0;
   out_782685044991339768[9] = 0;
   out_782685044991339768[10] = 0;
   out_782685044991339768[11] = 0;
   out_782685044991339768[12] = 0;
   out_782685044991339768[13] = 0;
   out_782685044991339768[14] = 1;
   out_782685044991339768[15] = 0;
   out_782685044991339768[16] = 0;
   out_782685044991339768[17] = 0;
}
void h_30(double *state, double *unused, double *out_7997689976486561273) {
   out_7997689976486561273[0] = state[4];
}
void H_30(double *state, double *unused, double *out_7566229450242765595) {
   out_7566229450242765595[0] = 0;
   out_7566229450242765595[1] = 0;
   out_7566229450242765595[2] = 0;
   out_7566229450242765595[3] = 0;
   out_7566229450242765595[4] = 1;
   out_7566229450242765595[5] = 0;
   out_7566229450242765595[6] = 0;
   out_7566229450242765595[7] = 0;
   out_7566229450242765595[8] = 0;
}
void h_26(double *state, double *unused, double *out_6647436109976065231) {
   out_6647436109976065231[0] = state[7];
}
void H_26(double *state, double *unused, double *out_222320963101113042) {
   out_222320963101113042[0] = 0;
   out_222320963101113042[1] = 0;
   out_222320963101113042[2] = 0;
   out_222320963101113042[3] = 0;
   out_222320963101113042[4] = 0;
   out_222320963101113042[5] = 0;
   out_222320963101113042[6] = 0;
   out_222320963101113042[7] = 1;
   out_222320963101113042[8] = 0;
}
void h_27(double *state, double *unused, double *out_5368666705608320367) {
   out_5368666705608320367[0] = state[3];
}
void H_27(double *state, double *unused, double *out_8705751311666361110) {
   out_8705751311666361110[0] = 0;
   out_8705751311666361110[1] = 0;
   out_8705751311666361110[2] = 0;
   out_8705751311666361110[3] = 1;
   out_8705751311666361110[4] = 0;
   out_8705751311666361110[5] = 0;
   out_8705751311666361110[6] = 0;
   out_8705751311666361110[7] = 0;
   out_8705751311666361110[8] = 0;
}
void h_29(double *state, double *unused, double *out_1731217177354298017) {
   out_1731217177354298017[0] = state[1];
}
void H_29(double *state, double *unused, double *out_6992388584796810077) {
   out_6992388584796810077[0] = 0;
   out_6992388584796810077[1] = 1;
   out_6992388584796810077[2] = 0;
   out_6992388584796810077[3] = 0;
   out_6992388584796810077[4] = 0;
   out_6992388584796810077[5] = 0;
   out_6992388584796810077[6] = 0;
   out_6992388584796810077[7] = 0;
   out_6992388584796810077[8] = 0;
}
void h_28(double *state, double *unused, double *out_366993437947474722) {
   out_366993437947474722[0] = state[0];
}
void H_28(double *state, double *unused, double *out_1909989567727279503) {
   out_1909989567727279503[0] = 1;
   out_1909989567727279503[1] = 0;
   out_1909989567727279503[2] = 0;
   out_1909989567727279503[3] = 0;
   out_1909989567727279503[4] = 0;
   out_1909989567727279503[5] = 0;
   out_1909989567727279503[6] = 0;
   out_1909989567727279503[7] = 0;
   out_1909989567727279503[8] = 0;
}
void h_31(double *state, double *unused, double *out_1791282666535874557) {
   out_1791282666535874557[0] = state[8];
}
void H_31(double *state, double *unused, double *out_3994470243852129694) {
   out_3994470243852129694[0] = 0;
   out_3994470243852129694[1] = 0;
   out_3994470243852129694[2] = 0;
   out_3994470243852129694[3] = 0;
   out_3994470243852129694[4] = 0;
   out_3994470243852129694[5] = 0;
   out_3994470243852129694[6] = 0;
   out_3994470243852129694[7] = 0;
   out_3994470243852129694[8] = 1;
}
#include <eigen3/Eigen/Dense>
#include <iostream>

typedef Eigen::Matrix<double, DIM, DIM, Eigen::RowMajor> DDM;
typedef Eigen::Matrix<double, EDIM, EDIM, Eigen::RowMajor> EEM;
typedef Eigen::Matrix<double, DIM, EDIM, Eigen::RowMajor> DEM;

void predict(double *in_x, double *in_P, double *in_Q, double dt) {
  typedef Eigen::Matrix<double, MEDIM, MEDIM, Eigen::RowMajor> RRM;

  double nx[DIM] = {0};
  double in_F[EDIM*EDIM] = {0};

  // functions from sympy
  f_fun(in_x, dt, nx);
  F_fun(in_x, dt, in_F);


  EEM F(in_F);
  EEM P(in_P);
  EEM Q(in_Q);

  RRM F_main = F.topLeftCorner(MEDIM, MEDIM);
  P.topLeftCorner(MEDIM, MEDIM) = (F_main * P.topLeftCorner(MEDIM, MEDIM)) * F_main.transpose();
  P.topRightCorner(MEDIM, EDIM - MEDIM) = F_main * P.topRightCorner(MEDIM, EDIM - MEDIM);
  P.bottomLeftCorner(EDIM - MEDIM, MEDIM) = P.bottomLeftCorner(EDIM - MEDIM, MEDIM) * F_main.transpose();

  P = P + dt*Q;

  // copy out state
  memcpy(in_x, nx, DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
}

// note: extra_args dim only correct when null space projecting
// otherwise 1
template <int ZDIM, int EADIM, bool MAHA_TEST>
void update(double *in_x, double *in_P, Hfun h_fun, Hfun H_fun, Hfun Hea_fun, double *in_z, double *in_R, double *in_ea, double MAHA_THRESHOLD) {
  typedef Eigen::Matrix<double, ZDIM, ZDIM, Eigen::RowMajor> ZZM;
  typedef Eigen::Matrix<double, ZDIM, DIM, Eigen::RowMajor> ZDM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, EDIM, Eigen::RowMajor> XEM;
  //typedef Eigen::Matrix<double, EDIM, ZDIM, Eigen::RowMajor> EZM;
  typedef Eigen::Matrix<double, Eigen::Dynamic, 1> X1M;
  typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> XXM;

  double in_hx[ZDIM] = {0};
  double in_H[ZDIM * DIM] = {0};
  double in_H_mod[EDIM * DIM] = {0};
  double delta_x[EDIM] = {0};
  double x_new[DIM] = {0};


  // state x, P
  Eigen::Matrix<double, ZDIM, 1> z(in_z);
  EEM P(in_P);
  ZZM pre_R(in_R);

  // functions from sympy
  h_fun(in_x, in_ea, in_hx);
  H_fun(in_x, in_ea, in_H);
  ZDM pre_H(in_H);

  // get y (y = z - hx)
  Eigen::Matrix<double, ZDIM, 1> pre_y(in_hx); pre_y = z - pre_y;
  X1M y; XXM H; XXM R;
  if (Hea_fun){
    typedef Eigen::Matrix<double, ZDIM, EADIM, Eigen::RowMajor> ZAM;
    double in_Hea[ZDIM * EADIM] = {0};
    Hea_fun(in_x, in_ea, in_Hea);
    ZAM Hea(in_Hea);
    XXM A = Hea.transpose().fullPivLu().kernel();


    y = A.transpose() * pre_y;
    H = A.transpose() * pre_H;
    R = A.transpose() * pre_R * A;
  } else {
    y = pre_y;
    H = pre_H;
    R = pre_R;
  }
  // get modified H
  H_mod_fun(in_x, in_H_mod);
  DEM H_mod(in_H_mod);
  XEM H_err = H * H_mod;

  // Do mahalobis distance test
  if (MAHA_TEST){
    XXM a = (H_err * P * H_err.transpose() + R).inverse();
    double maha_dist = y.transpose() * a * y;
    if (maha_dist > MAHA_THRESHOLD){
      R = 1.0e16 * R;
    }
  }

  // Outlier resilient weighting
  double weight = 1;//(1.5)/(1 + y.squaredNorm()/R.sum());

  // kalman gains and I_KH
  XXM S = ((H_err * P) * H_err.transpose()) + R/weight;
  XEM KT = S.fullPivLu().solve(H_err * P.transpose());
  //EZM K = KT.transpose(); TODO: WHY DOES THIS NOT COMPILE?
  //EZM K = S.fullPivLu().solve(H_err * P.transpose()).transpose();
  //std::cout << "Here is the matrix rot:\n" << K << std::endl;
  EEM I_KH = Eigen::Matrix<double, EDIM, EDIM>::Identity() - (KT.transpose() * H_err);

  // update state by injecting dx
  Eigen::Matrix<double, EDIM, 1> dx(delta_x);
  dx  = (KT.transpose() * y);
  memcpy(delta_x, dx.data(), EDIM * sizeof(double));
  err_fun(in_x, delta_x, x_new);
  Eigen::Matrix<double, DIM, 1> x(x_new);

  // update cov
  P = ((I_KH * P) * I_KH.transpose()) + ((KT.transpose() * R) * KT);

  // copy out state
  memcpy(in_x, x.data(), DIM * sizeof(double));
  memcpy(in_P, P.data(), EDIM * EDIM * sizeof(double));
  memcpy(in_z, y.data(), y.rows() * sizeof(double));
}




}
extern "C" {

void car_update_25(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_25, H_25, NULL, in_z, in_R, in_ea, MAHA_THRESH_25);
}
void car_update_24(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<2, 3, 0>(in_x, in_P, h_24, H_24, NULL, in_z, in_R, in_ea, MAHA_THRESH_24);
}
void car_update_30(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_30, H_30, NULL, in_z, in_R, in_ea, MAHA_THRESH_30);
}
void car_update_26(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_26, H_26, NULL, in_z, in_R, in_ea, MAHA_THRESH_26);
}
void car_update_27(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_27, H_27, NULL, in_z, in_R, in_ea, MAHA_THRESH_27);
}
void car_update_29(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_29, H_29, NULL, in_z, in_R, in_ea, MAHA_THRESH_29);
}
void car_update_28(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_28, H_28, NULL, in_z, in_R, in_ea, MAHA_THRESH_28);
}
void car_update_31(double *in_x, double *in_P, double *in_z, double *in_R, double *in_ea) {
  update<1, 3, 0>(in_x, in_P, h_31, H_31, NULL, in_z, in_R, in_ea, MAHA_THRESH_31);
}
void car_err_fun(double *nom_x, double *delta_x, double *out_8377796624028463417) {
  err_fun(nom_x, delta_x, out_8377796624028463417);
}
void car_inv_err_fun(double *nom_x, double *true_x, double *out_3733121449147292039) {
  inv_err_fun(nom_x, true_x, out_3733121449147292039);
}
void car_H_mod_fun(double *state, double *out_2037772241286162231) {
  H_mod_fun(state, out_2037772241286162231);
}
void car_f_fun(double *state, double dt, double *out_8306308797768162476) {
  f_fun(state,  dt, out_8306308797768162476);
}
void car_F_fun(double *state, double dt, double *out_438348106830506366) {
  F_fun(state,  dt, out_438348106830506366);
}
void car_h_25(double *state, double *unused, double *out_5971982590284797884) {
  h_25(state, unused, out_5971982590284797884);
}
void car_H_25(double *state, double *unused, double *out_3963824281975169266) {
  H_25(state, unused, out_3963824281975169266);
}
void car_h_24(double *state, double *unused, double *out_1517252688726049001) {
  h_24(state, unused, out_1517252688726049001);
}
void car_H_24(double *state, double *unused, double *out_782685044991339768) {
  H_24(state, unused, out_782685044991339768);
}
void car_h_30(double *state, double *unused, double *out_7997689976486561273) {
  h_30(state, unused, out_7997689976486561273);
}
void car_H_30(double *state, double *unused, double *out_7566229450242765595) {
  H_30(state, unused, out_7566229450242765595);
}
void car_h_26(double *state, double *unused, double *out_6647436109976065231) {
  h_26(state, unused, out_6647436109976065231);
}
void car_H_26(double *state, double *unused, double *out_222320963101113042) {
  H_26(state, unused, out_222320963101113042);
}
void car_h_27(double *state, double *unused, double *out_5368666705608320367) {
  h_27(state, unused, out_5368666705608320367);
}
void car_H_27(double *state, double *unused, double *out_8705751311666361110) {
  H_27(state, unused, out_8705751311666361110);
}
void car_h_29(double *state, double *unused, double *out_1731217177354298017) {
  h_29(state, unused, out_1731217177354298017);
}
void car_H_29(double *state, double *unused, double *out_6992388584796810077) {
  H_29(state, unused, out_6992388584796810077);
}
void car_h_28(double *state, double *unused, double *out_366993437947474722) {
  h_28(state, unused, out_366993437947474722);
}
void car_H_28(double *state, double *unused, double *out_1909989567727279503) {
  H_28(state, unused, out_1909989567727279503);
}
void car_h_31(double *state, double *unused, double *out_1791282666535874557) {
  h_31(state, unused, out_1791282666535874557);
}
void car_H_31(double *state, double *unused, double *out_3994470243852129694) {
  H_31(state, unused, out_3994470243852129694);
}
void car_predict(double *in_x, double *in_P, double *in_Q, double dt) {
  predict(in_x, in_P, in_Q, dt);
}
void car_set_mass(double x) {
  set_mass(x);
}
void car_set_rotational_inertia(double x) {
  set_rotational_inertia(x);
}
void car_set_center_to_front(double x) {
  set_center_to_front(x);
}
void car_set_center_to_rear(double x) {
  set_center_to_rear(x);
}
void car_set_stiffness_front(double x) {
  set_stiffness_front(x);
}
void car_set_stiffness_rear(double x) {
  set_stiffness_rear(x);
}
}

const EKF car = {
  .name = "car",
  .kinds = { 25, 24, 30, 26, 27, 29, 28, 31 },
  .feature_kinds = {  },
  .f_fun = car_f_fun,
  .F_fun = car_F_fun,
  .err_fun = car_err_fun,
  .inv_err_fun = car_inv_err_fun,
  .H_mod_fun = car_H_mod_fun,
  .predict = car_predict,
  .hs = {
    { 25, car_h_25 },
    { 24, car_h_24 },
    { 30, car_h_30 },
    { 26, car_h_26 },
    { 27, car_h_27 },
    { 29, car_h_29 },
    { 28, car_h_28 },
    { 31, car_h_31 },
  },
  .Hs = {
    { 25, car_H_25 },
    { 24, car_H_24 },
    { 30, car_H_30 },
    { 26, car_H_26 },
    { 27, car_H_27 },
    { 29, car_H_29 },
    { 28, car_H_28 },
    { 31, car_H_31 },
  },
  .updates = {
    { 25, car_update_25 },
    { 24, car_update_24 },
    { 30, car_update_30 },
    { 26, car_update_26 },
    { 27, car_update_27 },
    { 29, car_update_29 },
    { 28, car_update_28 },
    { 31, car_update_31 },
  },
  .Hes = {
  },
  .sets = {
    { "mass", car_set_mass },
    { "rotational_inertia", car_set_rotational_inertia },
    { "center_to_front", car_set_center_to_front },
    { "center_to_rear", car_set_center_to_rear },
    { "stiffness_front", car_set_stiffness_front },
    { "stiffness_rear", car_set_stiffness_rear },
  },
  .extra_routines = {
  },
};

ekf_lib_init(car)
