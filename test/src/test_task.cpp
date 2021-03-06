#include "harness_task.hpp"
#include "harness_waypoints.hpp"
#include "test_debug.hpp"
#include "TaskEventsPrint.hpp"

int main(int argc, char** argv)
{
  if (!parse_args(argc,argv)) {
    return 0;
  }

  #define NUM_RANDOM 50

  plan_tests(NUM_TASKS+2+NUM_RANDOM+8);

  TaskEventsPrint default_events(verbose);
  GlidePolar glide_polar(fixed_two);

  Waypoints waypoints;
  setup_waypoints(waypoints);

  {
    TaskManager task_manager(default_events,
                             waypoints);
    task_manager.set_glide_polar(glide_polar);
    test_task_bad(task_manager,waypoints);
  }

  for (int i=0; i<NUM_TASKS+2; i++) {
    TaskManager task_manager(default_events,
                             waypoints);
    task_manager.set_glide_polar(glide_polar);
    ok(test_task(task_manager, waypoints, i),test_name("construction",i,0),0);
  }

  for (int i=0; i<NUM_RANDOM; i++) {
    TaskManager task_manager(default_events,
                             waypoints);
    task_manager.set_glide_polar(glide_polar);
    ok(test_task(task_manager, waypoints, 7),test_name("construction",7,0),0);
  }

  return exit_status();
}

