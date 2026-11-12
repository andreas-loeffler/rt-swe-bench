# Mecanum drive keeps three wheels turning when the reference stops

We run `mecanum_drive_controller` chained behind a higher-level controller.
When that controller stops writing references (for example when it is
stopped or loses its input), the mecanum controller should command all four
wheels to zero. In our setup only the front-left wheel stops; the front-right,
rear-right and rear-left wheels keep the last commanded velocity.

Expected: when the reference interfaces hold NaN, every wheel is commanded to
zero velocity. When a valid reference arrives again, the robot should start
from standstill, as it would after activation.
