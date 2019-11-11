# MAKE.py
#   Group 44
#
# DESCRIPTION: A file that runs the simulation multiple times and displays the
#              average speedups. Also varies the number of amplitude points and
#              number of threads.

import argparse
import subprocess
import select

AMP_POINTS = [10**3, 10**4, 10**5, 10**6, 10**7]
NUM_THREADS = [1, 2, 3, 4, 5, 6, 7, 8]
NUM_NODES = 3
USE_DAS = True
TIMEOUT = 3


class Job():
    def __init__(self, id_, point, time_steps, n_threads):
        self.id = id_
        self.point = str(point)
        self.time_steps = str(time_steps)
        self.n_threads = str(n_threads)
        self.process = None
        self.time_took = -1
        self.time_took_normalized = -1

    def fileno(self):
        return self.process.stdout.fileno()

    def __str__(self):
        return "({},{},{})".format(self.point, self.time_steps, self.n_threads)


def main(path_to_sim):
    # Create all combinations of possible amp_points and threads
    work_to_do = []
    n = 0
    for n_threads in NUM_THREADS:
        for i, point in enumerate(AMP_POINTS):
            time_steps = AMP_POINTS[(len(AMP_POINTS) - 1) - i]
            work_to_do.append(Job(n, point, time_steps, n_threads))
            n += 1

    # Run the tasks. Do up to NUM_NODES at a time.
    work_doing = []
    work_done = {}
    while len(work_doing) > 0 or len(work_to_do) > 0:
        while len(work_doing) < NUM_NODES and len(work_to_do) > 0:
            # Run a new job
            job = work_to_do.pop(0)
            to_run = [path_to_sim, job.point, job.time_steps, job.n_threads]
            if USE_DAS:
                to_run = ["prun", "-v", "-np", "1"] + to_run
            job.process = subprocess.Popen(to_run, stdout=subprocess.PIPE)
            work_doing.append(job)
        # Wait for any of the jobs to complete (1 second timeout to avoid load
        #   on DAS)
        (readable, writeable, error) = select.select(work_doing, [], [], TIMEOUT)
        for job in readable:
            line = job.process.stdout.readline()
            words = line.decode("utf-8").split(" ")
            if words[0] == "Took":
                job.time_took = float(words[1])
            elif words[0] == "Normalized:":
                job.time_took_normalized = float(words[1])
            # If they're both filled, the job is done
            if job.time_took != -1 and job.time_took_normalized != -1:
                work_done[job.id] = job
                work_doing.remove(job)
                continue

    # If everything is done, print the results
    for n in range(len(work_done)):
        job = work_done[n]
        print("{},{}".format(job.time_took, job.time_took_normalized))


if __name__ == "__main__":
    # Parse arguments
    p = argparse.ArgumentParser()
    p.add_argument("-a", "--assign", help="The path to the assign file",
                   default="Ass1_1/assign1_1")
    args = p.parse_args()

    # Run main
    main(args.assign)
