import argparse
from . import common
from .__init__ import *

msg = """
The interface for parsing COSIM output logs. This parser comprises
of following five tools:

- blood_graph
- stats_parser
- pc_histogram
- trace_parser
- vcache_stall_graph

By default blood_graph, stats_parser and pc_histogram are executed,
given required files are generated by cosim. If required files for
a tool are missing it is skipped.
"""

parser = argparse.ArgumentParser(
    description=msg,
    formatter_class=argparse.RawDescriptionHelpFormatter,
    prog="vanilla_parser",
    conflict_handler='error')

# Add common arguments
common.add_args(parser)

# Add package specific options
parser.add_argument("--only", nargs='*', default=[], type=str,
                    choices=["blood_graph", "stats_parser", "pc_histogram",
                             "trace_parser", "vcache_stall_graph"],
                    metavar="SUBMODULE",
                    help="List of tools to run instead of the default set")
parser.add_argument("--also", nargs='*', default=[], type=str,
                    choices=["blood_graph", "stats_parser", "pc_histogram",
                             "trace_parser", "vcache_stall_graph"],
                    metavar="SUBMODULE",
                    help="List of tools to run in addition to the default set")

# Load command line options of all parser submodules
blood_graph.add_args(
    parser.add_argument_group('Blood graph specific options'))
stats_parser.add_args(
    parser.add_argument_group('Stats parser specific options'))
pc_histogram.add_args(
    parser.add_argument_group('PC histogram specific options'))
vcache_stall_graph.add_args(
    parser.add_argument_group('Vcache stall graph specific options'))

# Parse arguments
args = parser.parse_args()

# Default set
if (len(args.only) == 0) or ("blood_graph" in args.only) or ("blood_graph" in args.also):
    common.check_exists_and_run(
        [args.trace], blood_graph.main, args)
if (len(args.only) == 0) or ("pc_histogram" in args.only) or ("pc_histogram" in args.also):
    common.check_exists_and_run(
        [args.trace], pc_histogram.main, args)
if (len(args.only) == 0) or ("stats_parser" in args.only) or ("stats_parser" in args.also):
    common.check_exists_and_run(
        [args.stats], stats_parser.main, args)

# Run these tools only when mentioned explicitly
if ("vcache_stall_graph" in args.only) or ("vcache_stall_graph" in args.also):
    common.check_exists_and_run(
        [args.vcache_trace, args.vcache_stats], vcache_stall_graph.main, args)
if ("trace_parser" in args.only) or ("trace_parser" in args.also):
    common.check_exists_and_run(
        [args.log], trace_parser.main, args)
