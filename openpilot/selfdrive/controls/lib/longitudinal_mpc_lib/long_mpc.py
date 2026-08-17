#!/usr/bin/env python3
import os
import time
import numpy as np
from openpilot.cereal import log
from opendbc.car.interfaces import ACCEL_MIN, ACCEL_MAX
from openpilot.common.realtime import DT_MDL
from openpilot.common.swaglog import cloudlog
# WARNING: imports outside of constants will not trigger a rebuild
from openpilot.selfdrive.modeld.constants import index_function, ModelConstants

LEAD_T_IDXS_MODEL = np.array(ModelConstants.LEAD_T_IDXS)  # [0, 2, 4, 6, 8, 10]s
from openpilot.selfdrive.controls.radard import _LEAD_ACCEL_TAU

if __name__ == '__main__':  # generating code
  from acados.acados_template import AcadosModel, AcadosOcp, AcadosOcpSolver
else:
  from openpilot.selfdrive.controls.lib.longitudinal_mpc_lib.c_generated_code.acados_ocp_solver_pyx import AcadosOcpSolverCython

from casadi import SX, vertcat

MODEL_NAME = 'long'
LONG_MPC_DIR = os.path.dirname(os.path.abspath(__file__))
EXPORT_DIR = os.path.join(LONG_MPC_DIR, "c_generated_code")
JSON_FILE = os.path.join(LONG_MPC_DIR, "acados_ocp_long.json")

LongitudinalPlanSource = log.LongitudinalPlan.LongitudinalPlanSource
MPC_SOURCES = (LongitudinalPlanSource.lead0, LongitudinalPlanSource.lead1, LongitudinalPlanSource.cruise)

X_DIM = 3
U_DIM = 1
PARAM_DIM = 6
COST_E_DIM = 5
COST_DIM = COST_E_DIM + 1
CONSTR_DIM = 4

X_EGO_OBSTACLE_COST = 3.
X_EGO_COST = 0.
V_EGO_COST = 0.
A_EGO_COST = 0.
J_EGO_COST = 5.
A_CHANGE_COST = 200.
DANGER_ZONE_COST = 100.
CRASH_DISTANCE = .25
LEAD_DANGER_FACTOR = 0.75
LIMIT_COST = 1e6
ACADOS_SOLVER_TYPE = 'SQP_RTI'

# Fewer timestamps don't hurt performance and lead to
# much better convergence of the MPC with low iterations
N = 12
MAX_T = 10.0
T_IDXS_LST = [index_function(idx, max_val=MAX_T, max_idx=N) for idx in range(N+1)]

T_IDXS = np.array(T_IDXS_LST)
FCW_IDXS = T_IDXS < 5.0
T_DIFFS = np.diff(T_IDXS, prepend=[0.])
COMFORT_BRAKE = 2.5
STOP_DISTANCE = 6.0
CRUISE_MIN_ACCEL = -1.2
CRUISE_MAX_ACCEL = 1.2
MIN_X_LEAD_FACTOR = 0.5

# ============================================================
# 前車起步延遲 V2
# ============================================================
START_DELAY_FRAMES = 8 # 起步反應太快增加延遲(20Hz)：10=0.5秒、12=0.6秒、16=0.8秒、20=1.0秒
START_RADAR_SPEED = 0.5
START_MODEL_MOVE = 0.8

# Lead Decel Predictor Adaptive V1
LEAD_HISTORY_SIZE = 4
LEAD_DECEL_COUNT = 2
LEAD_DECEL_BP = [0.2, 0.6, 1.2]
LEAD_OFFSET_BP = [1.0, 2.0, 3.0]
LEAD_DISTANCE_BP = [10.0, 15.0, 20.0, 30.0, 40.0, 55.0, 70.0, 90.0, 120.0]
LEAD_DISTANCE_SCALE = [0.2, 0.5, 0.8, 1.0, 1.2, 1.5, 2.0, 3.0, 4.0]
LEAD_MIN_DECEL_BP = [40.0, 60.0, 90.0, 120.0]
LEAD_MIN_DECEL = [0.00, 0.05, 0.12, 0.20]
# ============================================================
# 前車極端急煞 / 接近靜止安全防護
# ============================================================
SAFETY_STOP_TRIGGER = 5.0
SAFETY_STOP_MIN_DISTANCE = 4.0
SAFETY_STOP_RELEASE_DISTANCE = 5.0
SAFETY_STOP_EGO_SPEED = 25.0 / 3.6
SAFETY_STOP_LEAD_SPEED = 3.0 / 3.6
SAFETY_STOP_DECEL = -1.5



def get_jerk_factor(personality=log.LongitudinalPersonality.standard):
  if personality==log.LongitudinalPersonality.relaxed:
    return 1.0
  elif personality==log.LongitudinalPersonality.standard:
    return 1.0
  elif personality==log.LongitudinalPersonality.aggressive:
    return 0.8
  else:
    raise NotImplementedError("Longitudinal personality not supported")


def get_T_FOLLOW(personality=log.LongitudinalPersonality.standard, v_ego=0.0):
  if personality==log.LongitudinalPersonality.relaxed:
    return 1.55
  elif personality==log.LongitudinalPersonality.standard:
    return 1.25
  elif personality==log.LongitudinalPersonality.aggressive:
    v_kph = v_ego * 3.6
    if v_kph < 35:
      return 1.05
    elif v_kph < 70:
      return 0.95
    else:
      return 0.85
  else:
    raise NotImplementedError("Longitudinal personality not supported")

# 0~35 km/h: 1.05
# 35~70 km/h: 0.95
# 70+ km/h: 0.85

def get_stopped_equivalence_factor(v_lead):
  return (v_lead**2) / (2 * COMFORT_BRAKE)

def get_safe_obstacle_distance(v_ego, t_follow):
  return (v_ego**2) / (2 * COMFORT_BRAKE) + t_follow * v_ego + STOP_DISTANCE

def gen_long_model():
  model = AcadosModel()
  model.name = MODEL_NAME

  # states
  x_ego, v_ego, a_ego = SX.sym('x_ego'), SX.sym('v_ego'), SX.sym('a_ego')
  model.x = vertcat(x_ego, v_ego, a_ego)

  # controls
  j_ego = SX.sym('j_ego')
  model.u = vertcat(j_ego)

  # xdot
  x_ego_dot = SX.sym('x_ego_dot')
  v_ego_dot = SX.sym('v_ego_dot')
  a_ego_dot = SX.sym('a_ego_dot')
  model.xdot = vertcat(x_ego_dot, v_ego_dot, a_ego_dot)

  # live parameters
  a_min = SX.sym('a_min')
  a_max = SX.sym('a_max')
  x_obstacle = SX.sym('x_obstacle')
  a_prev = SX.sym('a_prev')
  lead_t_follow = SX.sym('lead_t_follow')
  lead_danger_factor = SX.sym('lead_danger_factor')
  model.p = vertcat(a_min, a_max, x_obstacle, a_prev, lead_t_follow, lead_danger_factor)

  # dynamics model
  f_expl = vertcat(v_ego, a_ego, j_ego)
  model.f_impl_expr = model.xdot - f_expl
  model.f_expl_expr = f_expl
  return model

def gen_long_ocp():
  ocp = AcadosOcp()
  ocp.model = gen_long_model()

  Tf = T_IDXS[-1]

  # set dimensions
  ocp.dims.N = N

  # set cost module
  ocp.cost.cost_type = 'NONLINEAR_LS'
  ocp.cost.cost_type_e = 'NONLINEAR_LS'

  QR = np.zeros((COST_DIM, COST_DIM))
  Q = np.zeros((COST_E_DIM, COST_E_DIM))

  ocp.cost.W = QR
  ocp.cost.W_e = Q

  x_ego, v_ego, a_ego = ocp.model.x[0], ocp.model.x[1], ocp.model.x[2]
  j_ego = ocp.model.u[0]

  a_min, a_max = ocp.model.p[0], ocp.model.p[1]
  x_obstacle = ocp.model.p[2]
  a_prev = ocp.model.p[3]
  lead_t_follow = ocp.model.p[4]
  lead_danger_factor = ocp.model.p[5]

  ocp.cost.yref = np.zeros((COST_DIM, ))
  ocp.cost.yref_e = np.zeros((COST_E_DIM, ))

  desired_dist_comfort = get_safe_obstacle_distance(v_ego, lead_t_follow)

  # The main cost in normal operation is how close you are to the "desired" distance
  # from an obstacle at every timestep. This obstacle can be a lead car
  # or other object. In e2e mode we can use x_position targets as a cost
  # instead.
  costs = [((x_obstacle - x_ego) - (desired_dist_comfort)) / (v_ego + 10.),
           x_ego,
           v_ego,
           a_ego,
           a_ego - a_prev,
           j_ego]
  ocp.model.cost_y_expr = vertcat(*costs)
  ocp.model.cost_y_expr_e = vertcat(*costs[:-1])

  # Constraints on speed, acceleration and desired distance to
  # the obstacle, which is treated as a slack constraint so it
  # behaves like an asymmetrical cost.
  constraints = vertcat(v_ego,
                        (a_ego - a_min),
                        (a_max - a_ego),
                        ((x_obstacle - x_ego) - lead_danger_factor * (desired_dist_comfort)) / (v_ego + 10.))
  ocp.model.con_h_expr = constraints

  x0 = np.zeros(X_DIM)
  ocp.constraints.x0 = x0
  ocp.parameter_values = np.array([-1.2, 1.2, 0.0, 0.0, get_T_FOLLOW(v_ego=0.0), LEAD_DANGER_FACTOR])


  # We put all constraint cost weights to 0 and only set them at runtime
  cost_weights = np.zeros(CONSTR_DIM)
  ocp.cost.zl = cost_weights
  ocp.cost.Zl = cost_weights
  ocp.cost.Zu = cost_weights
  ocp.cost.zu = cost_weights

  ocp.constraints.lh = np.zeros(CONSTR_DIM)
  ocp.constraints.uh = 1e4*np.ones(CONSTR_DIM)
  ocp.constraints.idxsh = np.arange(CONSTR_DIM)

  # The HPIPM solver can give decent solutions even when it is stopped early
  # Which is critical for our purpose where compute time is strictly bounded
  # We use HPIPM in the SPEED_ABS mode, which ensures fastest runtime. This
  # does not cause issues since the problem is well bounded.
  ocp.solver_options.qp_solver = 'PARTIAL_CONDENSING_HPIPM'
  ocp.solver_options.hessian_approx = 'GAUSS_NEWTON'
  ocp.solver_options.integrator_type = 'ERK'
  ocp.solver_options.nlp_solver_type = ACADOS_SOLVER_TYPE
  ocp.solver_options.qp_solver_cond_N = 1

  # More iterations take too much time and less lead to inaccurate convergence in
  # some situations. Ideally we would run just 1 iteration to ensure fixed runtime.
  ocp.solver_options.qp_solver_iter_max = 10
  ocp.solver_options.qp_tol = 1e-3

  # set prediction horizon
  ocp.solver_options.tf = Tf
  ocp.solver_options.shooting_nodes = T_IDXS

  ocp.code_export_directory = EXPORT_DIR
  return ocp


class LongitudinalMpc:
  def __init__(self, dt=DT_MDL):
    self.dt = dt
    self.solver = AcadosOcpSolverCython(MODEL_NAME, ACADOS_SOLVER_TYPE, N)
    self.reset()
    self.source = LongitudinalPlanSource.cruise

  def reset(self):
    self.solver.reset()

    self.x_sol = np.zeros((N+1, X_DIM))
    self.u_sol = np.zeros((N, 1))
    self.v_solution = np.zeros(N+1)
    self.a_solution = np.zeros(N+1)
    self.j_solution = np.zeros(N)
    self.a_prev = np.array(self.a_solution)
    self.yref = np.zeros((N+1, COST_DIM))

    for i in range(N):
      self.solver.cost_set(i, "yref", self.yref[i])
    self.solver.cost_set(N, "yref", self.yref[N][:COST_E_DIM])

    self.params = np.zeros((N+1, PARAM_DIM))
    for i in range(N+1):
      self.solver.set(i, 'x', np.zeros(X_DIM))

    self.last_cloudlog_t = 0
    self.status = False
    self.crash_cnt = 0.0
    self.solution_status = 0
    # timers
    self.solve_time = 0.0
    self.time_qp_solution = 0.0
    self.time_linearization = 0.0
    self.time_integrator = 0.0
    self.x0 = np.zeros(X_DIM)
    self.lead_xv_0 = np.zeros((N+1, 2))
    self.lead_xv_1 = np.zeros((N+1, 2))
    self.set_weights()
    self.lead_v_history = []
    self.lead_start_counter = 0
    self.safety_stop_active = False

  def set_cost_weights(self, cost_weights, constraint_cost_weights):
    W = np.asfortranarray(np.diag(cost_weights))
    for i in range(N):
      # TODO don't hardcode A_CHANGE_COST idx
      # reduce the cost on (a-a_prev) later in the horizon.
      W[4,4] = cost_weights[4] * np.interp(T_IDXS[i], [0.0, 1.0, 2.0], [1.0, 1.0, 0.0])
      self.solver.cost_set(i, 'W', W)
    # Setting the slice without the copy make the array not contiguous,
    # causing issues with the C interface.
    self.solver.cost_set(N, 'W', np.copy(W[:COST_E_DIM, :COST_E_DIM]))

    # Set L2 slack cost on lower bound constraints
    Zl = np.array(constraint_cost_weights)
    for i in range(N):
      self.solver.cost_set(i, 'Zl', Zl)

  def set_weights(self, prev_accel_constraint=True, personality=log.LongitudinalPersonality.standard):
    jerk_factor = get_jerk_factor(personality)
    a_change_cost = A_CHANGE_COST if prev_accel_constraint else 0
    cost_weights = [X_EGO_OBSTACLE_COST, X_EGO_COST, V_EGO_COST, A_EGO_COST, jerk_factor * a_change_cost, jerk_factor * J_EGO_COST]
    constraint_cost_weights = [LIMIT_COST, LIMIT_COST, LIMIT_COST, DANGER_ZONE_COST]
    self.set_cost_weights(cost_weights, constraint_cost_weights)

  def set_cur_state(self, v, a):
    v_prev = self.x0[1]
    self.x0[1] = v
    self.x0[2] = a
    if abs(v_prev - v) > 2.:  # probably only helps if v < v_prev
      for i in range(N+1):
        self.solver.set(i, 'x', self.x0)

  def process_lead(self, model_lead, radar_lead, is_primary=False):
    v_ego = self.x0[1]

    if model_lead.prob > 0.5 and radar_lead.present:
      # Anchor h=0 to radar and use Model's future trajectory for h>0.
      x_lead_traj = float(radar_lead.dRel) + (np.asarray(model_lead.x, dtype=np.float64) - model_lead.x[0])
      v_lead_traj = float(radar_lead.vLead) + (np.asarray(model_lead.v, dtype=np.float64) - model_lead.v[0])
    else:
      # Fake a fast lead so MPC stays in the same mode.
      x_lead_traj = 50.0 + (v_ego + 10.0) * LEAD_T_IDXS_MODEL
      v_lead_traj = np.full_like(LEAD_T_IDXS_MODEL, v_ego + 10.0)

    # MPC won't converge on immediate crashes; lift h=0 to the minimum braking distance.
    v_lead_0 = v_lead_traj[0]
    min_x_lead = MIN_X_LEAD_FACTOR * (v_ego + v_lead_0) * (v_ego - v_lead_0) / (-ACCEL_MIN * 2)
    x_lead_traj[0] = max(x_lead_traj[0], min_x_lead)

    v_lead_traj = np.clip(v_lead_traj, 0.0, 1e8)

    # 前車鎖定 V1：只套用主要前車 leadOne。
    # 避免 leadTwo 呼叫 process_lead 時把同一個計數器重置。
    if is_primary and radar_lead.present and radar_lead.dRel < 12.0 and v_ego < 2.0:
      if (
        radar_lead.vLead > START_RADAR_SPEED and
        (model_lead.x[1] - model_lead.x[0]) > START_MODEL_MOVE
      ):
        self.lead_start_counter += 1
      else:
        self.lead_start_counter = 0

      if self.lead_start_counter < START_DELAY_FRAMES:
        x_lead_traj[:] = radar_lead.dRel
        v_lead_traj[:] = radar_lead.vLead
    elif is_primary:
      self.lead_start_counter = 0

    x_lead_mpc = np.maximum.accumulate(np.interp(T_IDXS, LEAD_T_IDXS_MODEL, x_lead_traj))
    v_lead_mpc = np.interp(T_IDXS, LEAD_T_IDXS_MODEL, v_lead_traj)
    return np.column_stack((x_lead_mpc, v_lead_mpc))

  def update_safety_stop(self, radarstate, v_ego):
    lead = radarstate.leadOne

    if not lead.present:
      self.safety_stop_active = False
      return

    d_rel = float(lead.dRel)
    v_lead = max(float(lead.vLead), 0.0)

    if self.safety_stop_active:
      if d_rel >= SAFETY_STOP_RELEASE_DISTANCE or v_lead > SAFETY_STOP_LEAD_SPEED * 1.5:
        self.safety_stop_active = False
    elif (
      d_rel <= SAFETY_STOP_TRIGGER and
      v_ego <= SAFETY_STOP_EGO_SPEED and
      v_lead <= SAFETY_STOP_LEAD_SPEED
    ):
      self.safety_stop_active = True

  def apply_safety_stop(self, lead_xv_0):
    if not self.safety_stop_active:
      return

    # 最後一道防線：前車接近停止且自車已低速時，
    # 直接把 MPC 使用的主要前車 obstacle 壓到至少 3m，
    # 同時視為低速前車，避免 MPC 因預測 trajectory 放掉煞車。
    lead_xv_0[:, 0] = np.minimum(lead_xv_0[:, 0], SAFETY_STOP_MIN_DISTANCE)
    lead_xv_0[:, 1] = np.minimum(lead_xv_0[:, 1], SAFETY_STOP_LEAD_SPEED)

  def update(self, v_cruise, modelV2, radarstate, personality=log.LongitudinalPersonality.standard):
    v_ego = self.x0[1]
    t_follow = get_T_FOLLOW(personality, v_ego)
    self.update_safety_stop(radarstate, v_ego)

    model_leads = modelV2.leadsV3
    self.status = model_leads[0].prob > 0.5 or model_leads[1].prob > 0.5

    lead_xv_0 = self.process_lead(model_leads[0], radarstate.leadOne, is_primary=True)
    lead_xv_1 = self.process_lead(model_leads[1], radarstate.leadTwo, is_primary=False)

    self.lead_xv_0 = lead_xv_0
    self.lead_xv_1 = lead_xv_1

    # To estimate a safe distance from a moving lead, we calculate how much stopping
    # distance that lead needs as a minimum. We can add that to the current distance
    # and then treat that as a stopped car/obstacle at this new distance.
    lead_stop_offset_0 = np.where(lead_xv_0[:,1] < 1.0, 1.0, 0.0)
    lead_stop_offset_1 = np.where(lead_xv_1[:,1] < 1.0, 1.0, 0.0)

    lead_0_obstacle = (
      lead_xv_0[:,0]
      + get_stopped_equivalence_factor(lead_xv_0[:,1])
      - lead_stop_offset_0
    )
    lead_1_obstacle = (
      lead_xv_1[:,0]
      + get_stopped_equivalence_factor(lead_xv_1[:,1])
      - lead_stop_offset_1
    )

    # ============================================================
    # Lead Decel Predictor Adaptive V1
    # ============================================================
    lead = radarstate.leadOne

    if lead.present:
      self.lead_v_history.append(float(lead.vLead))
      if len(self.lead_v_history) > LEAD_HISTORY_SIZE:
        self.lead_v_history.pop(0)
    else:
      self.lead_v_history.clear()

    if lead.present and len(self.lead_v_history) == LEAD_HISTORY_SIZE:
      near_lead = lead.dRel <= 40.0

      min_decel = 0.0 if near_lead else np.interp(
        lead.dRel,
        LEAD_MIN_DECEL_BP,
        LEAD_MIN_DECEL
      )

      decel_count = sum(
        (self.lead_v_history[i] - self.lead_v_history[i + 1]) > min_decel
        for i in range(LEAD_HISTORY_SIZE - 1)
      )

      lead_total_decel = 0.0
      if decel_count >= LEAD_DECEL_COUNT and lead.vLead < v_ego:
        lead_total_decel = self.lead_v_history[0] - self.lead_v_history[-1]

      closing_kph = max((v_ego - lead.vLead) * 3.6, 0.0)
      closing_total_decel = np.interp(
        closing_kph,
        [0.0, 10.0, 20.0, 30.0],
        [0.0, 0.2, 0.6, 1.2]
      )

      total_decel = max(lead_total_decel, closing_total_decel)

      if total_decel >= LEAD_DECEL_BP[0]:
        base_offset = np.interp(
          total_decel,
          LEAD_DECEL_BP,
          LEAD_OFFSET_BP
        )

        distance_scale = np.interp(
          lead.dRel,
          LEAD_DISTANCE_BP,
          LEAD_DISTANCE_SCALE
        )

        offset = base_offset * distance_scale

        # 只把主要前車的 obstacle 往前拉近。
        lead_0_obstacle -= offset

    self.apply_safety_stop(lead_xv_0)

    if self.safety_stop_active:
      lead_0_obstacle = np.minimum(lead_0_obstacle, SAFETY_STOP_MIN_DISTANCE)

    # Fake an obstacle for cruise, this ensures smooth acceleration to set speed
    # when the leads are no factor.
    v_lower = v_ego + (T_IDXS * CRUISE_MIN_ACCEL * 1.05)
    # TODO does this make sense when max_a is negative?
    v_upper = v_ego + (T_IDXS * CRUISE_MAX_ACCEL * 1.05)
    v_cruise_clipped = np.clip(v_cruise * np.ones(N+1), v_lower, v_upper)
    cruise_obstacle = np.cumsum(T_DIFFS * v_cruise_clipped) + get_safe_obstacle_distance(v_cruise_clipped, t_follow)

    x_obstacles = np.column_stack([lead_0_obstacle, lead_1_obstacle, cruise_obstacle])
    self.source = MPC_SOURCES[np.argmin(x_obstacles[0])]

    self.yref[:,:] = 0.0
    for i in range(N):
      self.solver.set(i, "yref", self.yref[i])
    self.solver.set(N, "yref", self.yref[N][:COST_E_DIM])

    self.params[:,0] = ACCEL_MIN
    self.params[:,1] = ACCEL_MAX
    self.params[:,2] = np.min(x_obstacles, axis=1)
    self.params[:,3] = np.copy(self.a_prev)
    self.params[:,4] = t_follow
    self.params[:,5] = LEAD_DANGER_FACTOR

    if self.safety_stop_active:
      # 不允許 MPC 在安全鎖定期間產生正加速度。
      self.params[:,1] = np.minimum(self.params[:,1], 0.0)

    # ============================================================
    # 追車加速抑制 V4
    # 距離越近 / 速差越大 -> 限制 MPC 最大加速度
    # ============================================================
    if lead.present and ((v_ego - lead.vLead) * 3.6 > 1.0):
      d = float(lead.dRel)

      v_rel_kph = max(
        (v_ego - lead.vLead) * 3.6,
        0.0
      )

      speed_factor = np.interp(
        v_rel_kph,
        [0.0, 2.0, 5.0, 10.0, 20.0],
        [1.0, 0.8, 0.6, 0.5, 0.4]
      )

      distance_factor = np.interp(
        d,
        [15.0, 25.0, 40.0, 60.0, 80.0],
        [0.4, 0.5, 0.7, 0.9, 1.0]
      )

      reduction = max(speed_factor * distance_factor, 0.15)

      self.params[:,1] *= reduction

    self.run()
    if (np.any(lead_xv_0[FCW_IDXS,0] - self.x_sol[FCW_IDXS,0] < CRASH_DISTANCE) and
            model_leads[0].prob > 0.9):
      self.crash_cnt += 1
    else:
      self.crash_cnt = 0

  def run(self):
    for i in range(N+1):
      self.solver.set(i, 'p', self.params[i])
    self.solver.constraints_set(0, "lbx", self.x0)
    self.solver.constraints_set(0, "ubx", self.x0)

    self.solution_status = self.solver.solve()
    self.solve_time = float(self.solver.get_stats('time_tot')[0])
    self.time_qp_solution = float(self.solver.get_stats('time_qp')[0])
    self.time_linearization = float(self.solver.get_stats('time_lin')[0])
    self.time_integrator = float(self.solver.get_stats('time_sim')[0])

    for i in range(N+1):
      self.x_sol[i] = self.solver.get(i, 'x')
    for i in range(N):
      self.u_sol[i] = self.solver.get(i, 'u')

    self.v_solution = self.x_sol[:,1]
    self.a_solution = self.x_sol[:,2]
    self.j_solution = self.u_sol[:,0]

    self.a_prev = np.interp(T_IDXS + self.dt, T_IDXS, self.a_solution)

    t = time.monotonic()
    if self.solution_status != 0:
      if t > self.last_cloudlog_t + 5.0:
        self.last_cloudlog_t = t
        cloudlog.warning(f"Long mpc reset, solution_status: {self.solution_status}")
      self.reset()


if __name__ == "__main__":
  ocp = gen_long_ocp()
  AcadosOcpSolver.generate(ocp, json_file=JSON_FILE)
