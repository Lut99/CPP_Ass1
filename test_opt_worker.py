# MAKE.py
#   Group 44
#
# DESCRIPTION: This file is the worker process for the test_opt.py tester.
#              it is supposed to run on a node until the designated runtime
#              has passed. The command to be run is passed each time from the
#              farmer process.

import subprocess
import sys
import time


def write(text, end="\n"):
    sys.stderr.write(text + end)
    sys.stderr.flush()


def main(default_runtime, max_time):
    start = time.time()
    end = start + max_time
    avg_runtime = default_runtime
    runtimes = []
    while end - time.time() >= 0:
        # Wait for input from the farmer
        job = sys.stdin.readline()
        # Remove the newline
        job = job[:-1]
        print("Received job: '{}'".format(job))
        # Determine if it's a command
        if (job == "###STOP###"):
            break
        # Try to run this subprocess
        if end - time.time() <= avg_runtime:
            write("not-enough-time")
            continue
        # Create the subprocess
        start_job = time.time()
        p = subprocess.Popen(job.split(" "), stdout=subprocess.PIPE)
        (stdout, _) = p.communicate()
        if stdout is None:
            print("Job '{}' returned None".format(job))
            write("result-none")
        else:
            # Send the result back to the input
            stdout = stdout.decode("utf-8")
            write("result,{}:".format(stdout.count("\n") + 1))
            write(stdout)
            stop_job = time.time()
            # Compute the new average runtime
            runtimes.append(stop_job - start_job)
            avg_runtime = sum(runtimes) / len(runtimes)
            print("Handled job '{}' in {:.2f} seconds (new average: {})\n"
                  .format(job, stop_job - start_job, avg_runtime))

        write("time-left,{}".format(end - time.time()))
    # Done
    print("Bye")


if __name__ == "__main__":
    # Parse the mandatory time to run
    if len(sys.argv) < 3:
        write("invalid-call")
        print("Usage: python3 test_opt_worker.py <default-job-runtime> " +
              "<time-to-work-max>")
        exit()
    default_runtime = int(sys.argv[1])
    time_to_run = int(sys.argv[2])
    write("hello")
    main(default_runtime, time_to_run)
