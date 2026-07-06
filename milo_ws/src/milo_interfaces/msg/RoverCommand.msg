builtin_interfaces/Time stamp
uint32 sequence

# Drive commands [-1, 1]
float32 wheel_fl # front left
float32 wheel_ml # middle left
float32 wheel_rl # rear left
float32 wheel_fr # front right
float32 wheel_mr # middle right
float32 wheel_rr # rear right

# Steer commands [rads]
float32 steer_fl # front left
float32 steer_rl # rear left
float32 steer_fr # front right
float32 steer_rr # rear right

# Auxiliary
bool headlights_on
bool led_strip_on

# Safety
bool enable_motors
bool estop