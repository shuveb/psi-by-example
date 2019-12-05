/* This program uses the same features as example 3, but has more
   options, and somewhat more structure in the -help output.  It
   also shows how you can ‘steal’ the remainder of the input
   arguments past a certain point, for programs that accept a
   list of items.  It also shows the special argp KEY value
   ARGP_KEY_NO_ARGS, which is only given if no non-option
   arguments were supplied to the program.

   For structuring the help output, two features are used,
   *headers* which are entries in the options vector with the
   first four fields being zero, and a two part documentation
   string (in the variable DOC), which allows documentation both
   before and after the options; the two parts of DOC are
   separated by a vertical-tab character (’\v’, or ’\013’).  By
   convention, the documentation before the options is just a
   short string saying what the program does, and that afterwards
   is longer, describing the behavior in more detail.  All
   documentation strings are automatically filled for output,
   although newlines may be included to force a line break at a
   particular point.  All documentation strings are also passed to
   the ‘gettext’ function, for possible translation into the
   current locale. */

#include <stdlib.h>
#include <error.h>
#include <argp.h>

const char *argp_program_version =
  "argp-ex4 1.0";
const char *argp_program_bug_address =
  "<bug-gnu-utils@prep.ai.mit.edu>";

/* Program documentation. */
static char doc[] =
  "Argp example #4 -- a program with somewhat more complicated\
options\
\vThis part of the documentation comes *after* the options;\
 note that the text is automatically filled, but it's possible\
 to force a line-break, e.g.\n<-- here.";

/* A description of the arguments we accept. */
static char args_doc[] = "ARG1 [STRING...]";

/* Keys for options without short-options. */
#define OPT_ABORT  1            /* –abort */

/* The options we understand. */
static struct argp_option options[] = {
  {"cpu-win", 'c', "CPU_WIN", 0, "Set CPU window (500-10000ms) to CPU_WIN" },
  {"cpu-trig", 'C', "CPU_TRIG", 0, "Set CPU threshold (50-1000ms) to CPU_TRIG" },
  {"full", 'f', 0, 0, "Set CPU threshold for full pressure" },
  {"io-win", 'i', "IO_WIN", 0, "Set IO window (500-10000ms) to IO_WIN" },
  {"io-trig", 'I', "IO_TRIG", 0, "Set IO threshold (50-1000ms) to IO_TRIG" },
  {"mem-win", 'm', "MEM_WIN", 0, "Set MEMORY window (500-10000ms) to MEM_WIN" },
  {"mem-trig", 'M', "MEM_TRIG", 0, "Set MEMORY threshold (50-1000ms) to MEM_TRIG" },
  {"verbose",  'v', 0,       0, "Produce verbose output" },
  {"quiet",    'q', 0,       0, "Don't produce any output" },
  {"some", 's', 0, 0, "Set CPU threshold for some pressure" },
  {"trigger", 't', "TRIGGER", 0, "Set Global threshold to (500-10000ms) to TRIGGER" },
  {"threshold",   'T', 0,       OPTION_ALIAS },
  {"window", 'w', "WIN", 0, "Set Global window (500-10000ms) to WIN" },
  {"tracking",   'W', 0,       OPTION_ALIAS },
  {"output",   'o', "FILE",  0,
   "Output to FILE instead of standard output" },
  { 0 }
};

/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *arg1;                   /* arg1 */
  char **strings;               /* [string…] */
  int silent, verbose, abort;   /* ‘-s’, ‘-v’, ‘--abort’ */
  int full;                     /* ‘-f’ */
  int some;                     /* ‘-s’ */
  char *output_file;            /* file arg to ‘--output’ */
  char *cpu_window;             /* file arg to ‘--cpu-window’ */
  char *cpu_trigger;            /* file arg to ‘--cpu-thresh’ */
  char *io_window;              /* file arg to ‘--io-window’ */
  char *io_trigger;             /* file arg to ‘--io-thresh’ */
  char *memory_window;          /* file arg to ‘--memory-window’ */
  char *memory_trigger;         /* file arg to ‘--memory-thresh’ */
};

/* Parse a single option. */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'q':
      arguments->silent = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 'f':
      arguments->full = 1;
      break;
    case 's':
      arguments->some = 1;
      break;
    case 'c':
      arguments->cpu_window = arg;
      break;
    case 'C':
      arguments->cpu_trigger = arg;
      break;
    case 'i':
      arguments->io_window = arg;
      break;
    case 'I':
      arguments->io_trigger = arg;
      break;
    case 'm':
      arguments->memory_window = arg;
      break;
    case 'M':
      arguments->memory_trigger = arg;
      break;
    case 'o':
      arguments->output_file = arg;
      break;

    case ARGP_KEY_NO_ARGS:
      argp_usage (state);

    case ARGP_KEY_ARG:
      /* Here we know that state->arg_num == 0, since we
         force argument parsing to end before any more arguments can
         get here. */
      arguments->arg1 = arg;

      /* Now we consume all the rest of the arguments.
         state->next is the index in state->argv of the
         next argument to be parsed, which is the first string
         we’re interested in, so we can just use
         &state->argv[state->next] as the value for
         arguments->strings.

         In addition, by setting state->next to the end
         of the arguments, we can force argp to stop parsing here and
         return. */
      arguments->strings = &state->argv[state->next];
      state->next = state->argc;

      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

int
main (int argc, char **argv)
{
  int i, j;
  struct arguments arguments;

  /* Default values. */
  arguments.silent = 0;
  arguments.verbose = 0;
  arguments.output_file = "-";
  arguments.abort = 0;
  int repeat_count = 1;
  /* Parse our arguments; every option seen by parse_opt will be
     reflected in arguments. */
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  for (i = 0; i < repeat_count; i++)
    {
      printf ("ARG1 = %s\n", arguments.arg1);
      printf ("STRINGS = ");
      for (j = 0; arguments.strings[j]; j++)
        printf (j == 0 ? "%s" : ", %s", arguments.strings[j]);
      printf ("\n");
      printf ("OUTPUT_FILE = %s\nVERBOSE = %s\nSILENT = %s\n",
              arguments.output_file,
              arguments.verbose ? "yes" : "no",
              arguments.silent ? "yes" : "no");
    }

  exit (0);
}
