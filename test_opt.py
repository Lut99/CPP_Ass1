# MAKE.py
#   Group 44
#
# DESCRIPTION: A file that runs the simulation multiple times and displays the
#              average speedups. Also varies the number of amplitude points and
#              number of threads.
#              The optimised version tries to maximize node use, by assigning
#              jobs to a node until that node's runtime has exceeded.

import argparse
import subprocess
import select

AMP_POINTS = [10**3, 10**4, 10**5, 10**6, 10**7]
NUM_THREADS = [1, 2, 3, 4, 5, 6, 7, 8]
NUM_NODES = 3
USE_DAS = False
TIMEOUT = 3


class Worker():
    def __init__(self, id_, time_to_run=850, default_runtime=10):
        self.id = id_
        self.max_rt = time_to_run
        self.def_rt = default_runtime
        # Create the argument list
        to_run = ["python3", "test_opt_worker.py", str(self.def_rt),
                  str(self.max_rt)]
        if USE_DAS:
            to_run = ["prun", "-v", "-np", "1"] + to_run
        self.process = subprocess.Popen(to_run, stdin=subprocess.PIPE,
                                        stderr=subprocess.PIPE)
        self.stdin = self.process.stdin
        self.stdout = self.process.stderr

        self.running_job = None

    def give_job(self, job):
        if type(job) != Job:
            raise TypeError("Can only pass Job objects")
        self.write(job.call)
        self.running_job = job

    def fileno(self):
        """ Wraps internal fileno() for select() compatibility """
        return self.stdout.fileno()

    def write(self, text, end="\n"):
        """ Writes to worker stdin """
        self.stdin.write(text + end)

    def read(self):
        """ Reads from worker stdout """
        return self.stdout.readline()


class Job():
    def __init__(self, id_, call):
        self.id = id_
        self.call = call
        self.time_took = -1
        self.time_took_normalized = -1

    def __str__(self):
        return "(Job {}: '{}')".format(self.id, self.call)


def main(path_to_sim, time_per_node):
    # JOB CREATION: Change this according to the desired job
    work_to_do = []
    n = 0
    for n_threads in NUM_THREADS:
        for i, point in enumerate(AMP_POINTS):
            time_steps = AMP_POINTS[(len(AMP_POINTS) - 1) - i]
            work_to_do.append(Job(n, "{} {} {} {}"
                              .format(path_to_sim, point, time_steps,
                                      n_threads)))
            n += 1

    # Run the tasks. Do up to NUM_NODES at a time.
    workers = []
    worker_id = 0
    work_done = {}
    while len(work_to_do) > 0 or len(workers) > 0:
        # Schedule up to NUM_NODES workers
        while len(workers) < NUM_NODES:
            workers.append(Worker(worker_id))
            worker_id += 1

        # For every worker that has nothing to do, give it a job
        for worker in workers:
            if worker.running_job is None:
                worker.give_job(work_to_do.pop(0))

        # Wait TIMEOUT to check if any worker says something
        (readable, writeable, error) = select.select(workers, [], [], TIMEOUT)
        for job in readable:
            line = job.process.stdout.readline()
            if line == ""
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
    p.add_argument("-t", "--time", type=int, help="The max time one node can" +
                   " run stuff on.", default=14)
    args = p.parse_args()

    # Run main
    main(args.assign, args.time)
