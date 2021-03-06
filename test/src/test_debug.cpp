#include "test_debug.hpp"
#include "harness_task.hpp"
#include "harness_flight.hpp"
#include <stdlib.h>

#ifdef DO_PRINT
#include <stdio.h>
#endif

int n_samples = 0;
int interactive = 0;
int verbose = 0;
int output_skip = 1;
double bearing_noise = 40.0;
double target_noise = 0.2;
double turn_speed = 25.0;
double sink_factor = 1.0;
double climb_factor = 1.0;
double start_alt = 1500.0;
int terrain_height = 1;
std::string replay_file = "test/data/9crx3101.igc";
bool enable_bestcruisetrack = false;

#ifdef INSTRUMENT_TASK
extern long count_mc;
long count_intersections;
long count_dijkstra_links = 0;
long count_dijkstra_queries = 0;
extern unsigned n_queries;
extern unsigned count_distbearing;
long count_olc = 0;
#endif


void distance_counts() {
  if (n_samples) {
#ifdef DO_PRINT
    printf("# Instrumentation\n");
#ifdef INSTRUMENT_TASK
    printf("#     dist+bearing calcs/c %d\n",count_distbearing/n_samples); 
    printf("#     mc calcs/c %d\n",(int)(count_mc/n_samples));
    if (n_queries>0) {
      printf("#     intersection tests/q %d\n",(unsigned)(count_intersections/n_queries));
      printf("#    (total queries %d)\n\n",n_queries);
    }
    if (count_olc>0) {
      printf("#     q/olc_res %d\n",(unsigned)(n_samples/count_olc));
    }
    if (count_dijkstra_queries>0) {
      printf("#     dijkstra links/q %d\n", (unsigned)(count_dijkstra_links/count_dijkstra_queries));
    }
#endif
    printf("#    (total cycles %d)\n\n",n_samples);
#endif
  }
  n_samples = 0;
#ifdef INSTRUMENT_TASK
  count_dijkstra_links = 0;
  count_dijkstra_queries = 0;
  count_intersections = 0;
  n_queries = 0;
  count_distbearing = 0;
  count_mc = 0;
  count_olc = 0;
#endif
}

void print_queries(unsigned n, std::ostream &fout) {
#ifdef INSTRUMENT_TASK
  if (n_queries>0) {
#ifdef DO_PRINT
    fout << n << " " << count_intersections/n_queries << "\n";
#endif
  }
  count_intersections = 0;
  n_queries = 0;
#endif
}

/** 
 * Wait-for-key prompt
 * 
 * @param time time of simulation
 * 
 * @return character received by keyboard
 */
char wait_prompt(const double time) {
#ifdef DO_PRINT
  if (interactive) {
    printf("# %g [enter to continue]\n",time);
    return getchar();
  }
#endif
  return 0;
}


/*
  100, 1604 cycles
  my ipaq: 
  test 1: 27.7 seconds, 277ms/cycle
  test 2: 117 seconds, 72ms/cycle
  test 3: 45 seconds, 28ms/cycle

  test 1  61209: 81 ms/c
  test 2 116266: 72 ms/c
  test 3  46122: 29 ms/c
  test 4 111742: 70 ms/c

*/


bool parse_args(int argc, char** argv) 
{
  while (1)    {
    static struct option long_options[] =
      {
	/* These options set a flag. */
	{"verbose", optional_argument,       0, 'v'},
	{"interactive", optional_argument,   0, 'i'},
	{"startalt", required_argument,   0, 'a'},
	{"bearingnoise", required_argument,   0, 'n'},
	{"outputskip", required_argument,       0, 's'},
	{"targetnoise", required_argument,       0, 't'},
	{"turnspeed", required_argument,       0, 'r'},
	{"igc", required_argument,       0, 'f'},
	{0, 0, 0, 0}
      };
    /* getopt_long stores the option index here. */
    int option_index = 0;
    
    int c = getopt_long (argc, argv, "s:v:i:n:t:r:a:f:",
                         long_options, &option_index);
    /* Detect the end of the options. */
    if (c == -1)
      break;
    
    switch (c) {
    case 0:
      /* If this option set a flag, do nothing else now. */
      if (long_options[option_index].flag != 0)
	break;
      printf ("option %s", long_options[option_index].name);
      if (optarg)
	printf (" with arg %s", optarg);
      printf ("\n");
      break;
    case 'f':
      replay_file = optarg;
      break;
    case 'a':
      start_alt = atof(optarg);
      break;
    case 's':
      output_skip = atoi(optarg);
      break;
    case 'v':
      if (optarg) {
        verbose = atoi(optarg);
      } else {
        verbose = 1;
      }
      break;
    case 'n':
      bearing_noise = atof(optarg);
      break;
    case 't':
      target_noise = atof(optarg);
      break;
    case 'r':
      turn_speed = atof(optarg);
      break;
    case 'i':
      if (optarg) {
        interactive = atoi(optarg);
      } else {
        interactive = 1;
      }
      break;
    case '?':
      /* getopt_long already printed an error message. */
      return false;
      break;      
    default:
      return false;
    }
  }

  if (interactive && !verbose) {
    verbose=1;
  }

  return true;
}

const char* test_name(const char* in, int task_num, int wind_num)
{
  static char buffer[80];
  sprintf(buffer,"%s (task %s, wind %s)", in, task_name(task_num), wind_name(wind_num));
  return buffer;
}
