# MAKE.py
#   Group 44
#
# DESCRIPTION: A file that runs the simulation multiple times and displays the
#              average speedups. Also varies the number of amplitude points and
#              number of threads.

import argparse
import subprocess
import sys

AMP_POINTS = [10**3, 10**4, 10**5, 10**6, 10**7]
NUM_THREADS = [1, 2, 3, 4, 5, 6, 7, 8]


def main(path_to_sim):
    # Loop through the different combinations of variables
    for n_threads in NUM_THREADS:
        for i, point in enumerate(AMP_POINTS):
            time_steps = AMP_POINTS[(len(AMP_POINTS) - 1) - i]
            # Run 'assign1_1' in Ass1_1/ for this set of parameters
            p = subprocess.Popen([path_to_sim, str(point),
                                    str(time_steps), str(n_threads)],
                                    stdout=subprocess.PIPE)
            (result, _) = p.communicate()
            time_s = None
            norm_s = None
            for line in result.decode("utf-8").split("\n"):
                words = line.split(" ")
                if len(words) > 1:
                    if words[0] == "Took":
                        time_s = float(words[1])
                    if words[0] == "Normalized:":
                        norm_s = float(words[1])
            if time_s is None or norm_s is None:
                print("WARNING: Error while executing for " +
                      "{} points and {} threads".format(point, n_threads))
                continue
            # Write the results in a copy/pastable way
            print("{},{}".format(time_s, norm_s))
            sys.stdout.flush()



if __name__ == "__main__":
    # Parse arguments
    p = argparse.ArgumentParser()
    p.add_argument("-a", "--assign", help="The path to the assign file",
                   default="Ass1_1/assign1_1")
    args = p.parse_args()

    # Run main
    main(args.assign)
